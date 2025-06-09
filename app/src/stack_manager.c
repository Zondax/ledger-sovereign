/*******************************************************************************
 *   (c) 2018 - 2025 Zondax AG
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ********************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "parser_common.h"
#include "zxmacros.h"

#if defined(LEDGER_SPECIFIC)
#define STACK_SHIFT 0x20
#define MINIMUM_STACK 0x400
#else
static int16_t recursionDepthCounter = 0;
#define MAX_RECURSION_DEPTH 50
#endif

/**
 * @brief Checks the available stack space to prevent stack overflow.
 *
 * @return parser_error_t Returns parser_running_out_of_stack if stack space is insufficient, otherwise parser_ok.
 */
parser_error_t checkStack() {
#if defined(LEDGER_SPECIFIC)
    // NOLINTNEXTLINE(readability-identifier-length): here `p` is fine
    void *p = NULL;
    const uint32_t availableStack = (uint32_t)((void *)&p) + STACK_SHIFT - (uint32_t)&app_stack_canary;
    if (availableStack <= MINIMUM_STACK) {
        return parser_running_out_of_stack;
    }
#else
    if (recursionDepthCounter >= MAX_RECURSION_DEPTH) {
        return parser_running_out_of_stack;
    }
    recursionDepthCounter++;
#endif
    return parser_ok;
}

/**
 * @brief Frees the stack space by decrementing the recursion depth counter.
 * @param depth The depth of the stack to free.
 *
 * @return parser_error_t Always returns parser_ok.
 */
parser_error_t freeStack(uint8_t depth) {
#if !defined(LEDGER_SPECIFIC)
    if (recursionDepthCounter > 0) {
        recursionDepthCounter -= depth;
    }
#else
    (void)depth;
    void *p = NULL;
    const uint32_t availableStack = (uint32_t)((void *)&p) - (uint32_t)&app_stack_canary;
    if (availableStack <= MINIMUM_STACK) {
        return parser_running_out_of_stack;
    }
#endif
    return parser_ok;
}
