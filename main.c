#include <avr/io.h>
#include <util/delay.h>
#include <coroutine.h>

#define F_CPU 16000000UL
#define BAUD 57600
#include <util/setbaud.h>

/** @brief Process stack
 *
 */
uint8_t ps[256];

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

void the_coroutine() {
    char* msg = "Test message";
    uart_println((uint8_t*)"3. coroutine started, yielding");
    _delay_ms(500);
    coyield();
    uart_println((uint8_t*)"5. continued from yield");
    _delay_ms(500);
    while (*msg) {
        while (uart_putc(*msg));
        msg++;
        coyield();
    }
    while (uart_putc('\n'));
    while (uart_putc('\r'));
    coend();
}

int main() {
    // initalize the UART
    uart_init();
    uart_println((uint8_t*)"1. main started");
    _delay_ms(500);

    uart_println((uint8_t*)"2. starting coroutine");
    _delay_ms(500);
    costart(&ps[255], the_coroutine);

    uart_println((uint8_t*)"4. returned from costart");
    _delay_ms(500);
    while (1) {
        _delay_ms(500);
        if (cogetstack()) {
            coresume();
        }
        else {
            uart_println((uint8_t*)"6. coroutine ended");
        }
    }
    return 0;
}
