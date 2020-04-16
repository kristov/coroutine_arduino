#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 16000000UL
#define BAUD 57600
#include <util/setbaud.h>

/** @brief Process stack
 *
 */
uint8_t ps[256];
uint8_t* psp;
uint8_t* ksp;

/** @brief Init the UART
 *
 */
uint8_t uart_init() {
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~(_BV(U2X0));
#endif
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
    return 0;
}

/** @brief Put a character
 *
 */
uint8_t uart_putc(uint8_t c) {
    if (!(UCSR0A & (1 << UDRE0))) {
        return 1;
    }
    UDR0 = c;
	return 0;
}

/** @brief Print a line of text
 *
 */
void uart_println(uint8_t* str) {
    while (*str) {
        while (uart_putc(*str));
        str++;
    }
    while (uart_putc('\n'));
    while (uart_putc('\r'));
}

/** @brief Get a character
 *
 */
uint8_t uart_getc(uint8_t* c) {
	if (!(UCSR0A & (1 << RXC0))) {
        return 1;
    }
    *c = UDR0;
	return 0;
}

/** @brief Start a coroutine
 * 
 */
void kstart(void* coroutine) {
    asm volatile(
        "; save the kernel stack pointer\n\t"
        "in __tmp_reg__, %[_SPL_]\n\t"
        "sts ksp, __tmp_reg__\n\t"
        "in __tmp_reg__, %[_SPH_]\n\t"
        "sts ksp+1, __tmp_reg__\n\t"
        "; enable the proc stack\n\t"
        "lds __tmp_reg__,psp\n\t"
        "out %[_SPL_],__tmp_reg__\n\t"
        "lds __tmp_reg__,psp+1\n\t"
        "out %[_SPH_],__tmp_reg__\n\t"
        "; push the coroutine address onto proc stack\n\t"
        "push %A0\n\t"
        "push %B0\n\t"
        "; jump to the coroutine on ret at end of function\n\t"
        : :
        "e" (coroutine),
        [_SPL_] "i" _SFR_IO_ADDR(SPL),
        [_SPH_] "i" _SFR_IO_ADDR(SPH)
    );
    return;
}

/** @brief Yield to the kernel
 * 
 */
void kyield() {
    asm volatile(
        "; save the proc stack pointer\n\t"
        "in __tmp_reg__, %[_SPL_]\n\t"
        "sts psp, __tmp_reg__\n\t"
        "in __tmp_reg__, %[_SPH_]\n\t"
        "sts psp+1, __tmp_reg__\n\t"
        "; restore the kernel stack\n\t"
        "lds __tmp_reg__,ksp\n\t"
        "out %[_SPL_],__tmp_reg__\n\t"
        "lds __tmp_reg__,ksp+1\n\t"
        "out %[_SPH_],__tmp_reg__\n\t"
        : :
        [_SPL_] "i" _SFR_IO_ADDR(SPL),
        [_SPH_] "i" _SFR_IO_ADDR(SPH)
    );
    return;
}

/** @brief Resume to a process with a stack pointer at psp
 * 
 */
void kresume() {
    asm volatile(
        "; save the kernel stack pointer\n\t"
        "in __tmp_reg__, %[_SPL_]\n\t"
        "sts ksp, __tmp_reg__\n\t"
        "in __tmp_reg__, %[_SPH_]\n\t"
        "sts ksp+1, __tmp_reg__\n\t"
        "; restore the proc stack\n\t"
        "lds __tmp_reg__,psp\n\t"
        "out %[_SPL_],__tmp_reg__\n\t"
        "lds __tmp_reg__,psp+1\n\t"
        "out %[_SPH_],__tmp_reg__\n\t"
        : :
        [_SPL_] "i" _SFR_IO_ADDR(SPL),
        [_SPH_] "i" _SFR_IO_ADDR(SPH)
    );
}

/** @brief End a coroutine
 * 
 */
void kend() {
    psp = 0;
    asm volatile(
        "; zero the proc stack pointer\n\t"
        "; restore the kernel stack\n\t"
        "lds __tmp_reg__,ksp\n\t"
        "out %[_SPL_],__tmp_reg__\n\t"
        "lds __tmp_reg__,ksp+1\n\t"
        "out %[_SPH_],__tmp_reg__\n\t"
        : :
        [_SPL_] "i" _SFR_IO_ADDR(SPL),
        [_SPH_] "i" _SFR_IO_ADDR(SPH)
    );
    return;
}

void the_coroutine() {
    char* msg = "Test message";
    uart_println((uint8_t*)"3. coroutine started, yielding");
    _delay_ms(500);
    kyield();
    uart_println((uint8_t*)"5. continued from yield");
    _delay_ms(500);
    while (*msg) {
        while (uart_putc(*msg));
        msg++;
        kyield();
    }
    while (uart_putc('\n'));
    while (uart_putc('\r'));
    kend();
}

int main() {
    // global var storing the current stack top of the proc
    psp = &ps[255];

    // initalize the UART
    uart_init();
    uart_println((uint8_t*)"1. main started");
    _delay_ms(500);

    uart_println((uint8_t*)"2. starting coroutine");
    _delay_ms(500);
    kstart(the_coroutine);

    uart_println((uint8_t*)"4. returned from kstart");
    _delay_ms(500);
    while (1) {
        _delay_ms(500);
        if (psp) {
            kresume();
        }
        else {
            uart_println((uint8_t*)"6. coroutine ended");
        }
    }
    return 0;
}
