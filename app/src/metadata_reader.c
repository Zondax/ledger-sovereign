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

#include "metadata_reader.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "parser_common.h"
#include "parser_impl.h"
#include "parser_txdef.h"
#include "zxmacros.h"
#include "borsh.h"

#if defined(TARGET_NANOS) || defined(TARGET_NANOX) || defined(TARGET_NANOS2) || defined(TARGET_STAX) || defined(TARGET_FLEX)
#define STACK_SHIFT   20
#define MINIMUM_STACK 400
#else
static uint16_t recursionDepthCounter = 0;
#define MAX_RECURSION_DEPTH 50
#endif

/**
 * @brief Checks the available stack space to prevent stack overflow.
 *
 * @return parser_error_t Returns parser_running_out_of_stack if stack space is insufficient, otherwise parser_ok.
 */
parser_error_t checkStack() {
#if defined(TARGET_NANOS) || defined(TARGET_NANOX) || defined(TARGET_NANOS2) || defined(TARGET_STAX) || defined(TARGET_FLEX)
    // NOLINTNEXTLINE(readability-identifier-length): here `p` is fine
    void *p = NULL;
    const uint32_t availableStack = (uint32_t)((void *)&p) + STACK_SHIFT - (uint32_t)&app_stack_canary;
    ZEMU_LOGF(50, "Available stack: %d\n", availableStack)
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
 *
 * @return parser_error_t Always returns parser_ok.
 */
parser_error_t freeStack() {
#if !defined(TARGET_NANOS) && !defined(TARGET_NANOX) && !defined(TARGET_NANOS2) && !defined(TARGET_STAX) && \
    !defined(TARGET_FLEX)
    if (recursionDepthCounter > 0) {
        recursionDepthCounter--;
    }
#endif
    return parser_ok;
}

parser_error_t metadata_read(parser_context_t *ctx, parser_tx_t *txObj) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(txObj);
    
    CHECK_ERROR(read_u32(ctx, &txObj->schema.types.qty));

    printf("types.qty: %d\n", txObj->schema.types.qty);

    return parser_ok;
}
