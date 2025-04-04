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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <sigutils.h>
#include <stdbool.h>

#include "parser_impl.h"

#define OFFSET_U8 1
#define OFFSET_U16 2
#define OFFSET_U32 4
#define OFFSET_U64 8

#define DEFINE_READ_UINT(bits)                                                              \
    static inline parser_error_t read_u##bits(parser_context_t *ctx, uint##bits##_t *val) { \
        CHECK_INPUT(ctx);                                                                   \
        CHECK_INPUT(val);                                                                   \
        *val = *(uint##bits##_t *)(ctx->buffer.ptr + ctx->offset);                          \
        CTX_CHECK_AND_ADVANCE(ctx, OFFSET_U##bits);                                         \
        return parser_ok;                                                                   \
    }

DEFINE_READ_UINT(8)
DEFINE_READ_UINT(16)
DEFINE_READ_UINT(32)
DEFINE_READ_UINT(64)

#undef DEFINE_READ_UINT

// TODO: Remove these functions
void print_buffer(bytes_t *buffer, const char *title);
void print_buffer_str(bytes_t *buffer, const char *title);
void print_string(const char *str);
void print_u8(const char *str, uint8_t val);
void print_u16(const char *str, uint16_t val);
void print_u32(const char *str, uint32_t val);
void print_u64(const char *str, uint64_t val);
void print_u64_hex(const char *str, uint64_t val);

#ifdef __cplusplus
}
#endif
