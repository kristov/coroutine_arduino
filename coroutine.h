#ifndef COROUTINE_H
#define COROUTINE_H

#include <avr/io.h>

/** @brief Switch to another stack
 * 
 */
void coswitch(uint8_t* sktop);

/** @brief Start a coroutine
 * 
 */
void costart(uint8_t* sktop, void* coroutine);

/** @brief Yield to the kernel
 * 
 */
void coyield();

/** @brief Resume to a process with a stack pointer at co_psp
 * 
 */
void coresume();

/** @brief End a coroutine
 * 
 */
void coend();

/** @brief Get current stack pointer
 * 
 */
uint8_t* cogetstack();

#endif
