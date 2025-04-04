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
#include "crypto_helper.h"

#include "bech32.h"
#include "zxformat.h"
#include "parser_impl.h"

#define OFFSET_U32 4

parser_error_t read_u32(parser_context_t* ctx, uint32_t* val) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(val);

    *val = *(uint32_t*)(ctx->buffer + ctx->offset);
    CTX_CHECK_AND_ADVANCE(ctx, OFFSET_U32);
    return parser_ok;
}
