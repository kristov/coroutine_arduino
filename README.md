# Coroutine example for Arduino

This is an example of a coroutine running on the Arduino Duemilanove (although it should run on any Arduino). To test this out plug in your Arduino and run:

    make
    make install
    screen /dev/ttyUSB0 57600

Note: this uses "screen" but you can use any serial connection. You should see these messages printed once per second:

    1. main started
    2. starting coroutine
    3. coroutine started, yielding
    4. returned from costart
    5. continued from yield

Then it should print "Test message" one character at a time per second. The "magic" of coroutines is that this string is printed from the following function (with debug messages removed):

    void the_coroutine() {
        char* msg = "Test message";
        coyield();
        while (*msg) {
            while (uart_putc(*msg));
            msg++;
            coyield();
        }
        coend();
    }

Each time a character is printed using `uart_putc()` the coroutine "yields" back to the `main()` routine using `coyield()`. This is kinda similar to making a blocking write call in a preemptive multitasking OS. This is intended to be a core component of a cooperative multitasking OS I am writing for the Arduino.

These coroutines must be structured as follows:

## Starting a coroutine

First a stack must be allocated for the coroutine somewhere in memory. In this example the stack is called `ps`:

    uint8_t ps[256];

The kernel stack is left as-is at the top of RAM. There are then two variables in SRAM in `coroutine.c`:

    uint8_t* co_psp;
    uint8_t* co_ksp;

These are used to store the stack pointers for the coroutine and the kernel respectively. In future I would like to store the coroutine stack pointer on the coroutine stack and return it to the caller via the return of `coresume()`, which would elminate the need to create a separate structure to store all the running coroutines.

The coroutine is started using `costart()` which accepts a stack pointer and a function pointer as an argument. The first `coyield()` will return to the instruction after the call to `costart()`.

## Function entry

Any initialization code is run at the start of the coroutine before the first `coyield()`. This is to allow the routine to initialize itself, populate data structures etc. The first `coyield()` returns to the `costart()`.

## Continuing the coroutine

Subsequent calls to the coroutine are made through `coresume()` which will resume whatever is in `psp`. In future I want to pass the stack pointer as an argument to `coresume()` to specify which coroutine to resume, and additionally return the modified stack pointer from `coresume()` to store somewhere.

## Ending the coroutine

The coroutine must be finished via `coend()` which simply zeros the `co_psp` variable. The kernel then needs to call `cogetstack()` to check if this `coresume()` is returning from a `coyield()` or a `coend()`. It's clunky but I could not figure out a way to return a value in this setup. However in an OS environment where there are multiple stacks the kernel loop will want to save the stack pointer before calling `coswitch()` to enable a different coroutine.
