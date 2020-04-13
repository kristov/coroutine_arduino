# Coroutine example for Arduino

This is an example of a coroutine running on the Arduino Duemilanove (although it should run on any Arduino). To test this out plug in your Arduino and run:

    make
    make install
    screen /dev/ttyUSB0 57600

Note: this uses "screen" but you can use any serial connection. You should see these messages printed once per second:

    1. main started
    2. starting coroutine
    3. coroutine started, yielding
    4. returned from kstart
    5. continued from yield

Then it should print "Test message" one character at a time per second. The "magic" of coroutines is that this string is printed from the following function (with debug messages removed):

    void the_coroutine() {
        char* msg = "Test message";
        kyield();
        while (*msg) {
            while (uart_putc(*msg));
            msg++;
            kyield();
        }
        kend();
    }

Each time a character is printed using `uart_putc()` the coroutine "yields" back to the `main()` routine using `kyield()`. This is kinda similar to making a blocking write call in a preemptive multitasking OS. This is intended to be a core component of a cooperative multitasking OS I am writing for the Arduino.

These coroutines must be structured as follows:

## Starting a coroutine

First a stack must be allocated for the coroutine somewhere in memory. In this example the stack is called `ps`:

    uint8_t ps[256];

The kernel stack is left as-is at the top of RAM. There are then two global variables:

    uint8_t* psp;
    uint8_t* ksp;

These are used to store the stack pointers for the coroutine and the kernel respectively. In future I would like to store the coroutine stack pointer on the coroutine stack and return it to the caller via the return of `kresume()`, which would elminate the need to create a separate structure to store all the running coroutines.

The coroutine is started using `kstart()` which accepts a function pointer as an argument. The first `yield()` will return to the instruction after the call to `kstart()`.

## Function entry

Any initialization code is run at the start of the coroutine before the first `yield()`. This is to allow the routine to initialize itself, populate data structures etc. The first `yield()` returns to the `kstart()`.

## Continuing the coroutine

Subsequent calls to the coroutine are made through `kresume()` which will resume whatever is in `psp`. In future I want to pass the stack pointer as an argument to `kresume()` to specify which coroutine to resume, and additionally return the modified stack pointer from `kresume()` to store somewhere.

## Ending the coroutine

The coroutine must be finished via `kend()` however this is broken.
