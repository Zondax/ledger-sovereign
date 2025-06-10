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
#include "borsh.h"

#include "bech32.h"
#include "crypto_helper.h"
#include "parser_impl.h"
#include "zxformat.h"

parser_error_t read_u8(parser_context_t *ctx, uint8_t *val) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(val);
    *val = *(uint8_t *)(ctx->buffer.ptr + ctx->offset);
    CTX_CHECK_AND_ADVANCE(ctx, OFFSET_U8);
    return parser_ok;
}

parser_error_t read_u16(parser_context_t *ctx, uint16_t *val) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(val);
    *val = *(uint16_t *)(ctx->buffer.ptr + ctx->offset);
    CTX_CHECK_AND_ADVANCE(ctx, OFFSET_U16);
    return parser_ok;
}

parser_error_t read_u32(parser_context_t *ctx, uint32_t *val) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(val);
    *val = *(uint32_t *)(ctx->buffer.ptr + ctx->offset);
    CTX_CHECK_AND_ADVANCE(ctx, OFFSET_U32);
    return parser_ok;
}

parser_error_t read_u64(parser_context_t *ctx, uint64_t *val) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(val);
    *val = *(uint64_t *)(ctx->buffer.ptr + ctx->offset);
    CTX_CHECK_AND_ADVANCE(ctx, OFFSET_U64);
    return parser_ok;
}

// void print_buffer(bytes_t *buffer, const char *title) {
// #if defined(LEDGER_SPECIFIC)
//     ZEMU_LOGF(50, "%s\n", title);
//     char print[1000] = {0};
//     array_to_hexstr(print, sizeof(print), buffer->ptr, buffer->len);
//     ZEMU_LOGF(1000, "%s\n", print);
// #else
//     printf("%s %d: ", title, buffer->len);
//     for (uint16_t i = 0; i < buffer->len; i++) {
//         printf("%02x", buffer->ptr[i]);
//     }
//     printf("\n");
// #endif
// }

// void print_buffer_u8(bytes_t *buffer, const char *title) {
// #if defined(LEDGER_SPECIFIC)
//     ZEMU_LOGF(50, "%s\n", title);
//     char print[1000] = {0};
//     array_to_hexstr(print, sizeof(print), buffer->ptr, buffer->len);
//     ZEMU_LOGF(1000, "%s\n", print);
// #else
//     printf("%s %d: [", title, buffer->len);
//     for (uint16_t i = 0; i < buffer->len; i++) {
//         printf("%d, ", buffer->ptr[i]);
//     }
//     printf("]\n");
// #endif
// }

// void print_buffer_str(bytes_t *buffer, const char *title) {
// #if defined(LEDGER_SPECIFIC)
//     ZEMU_LOGF(50, "%s\n", title);
//     char print[1000] = {0};
//     array_to_hexstr(print, sizeof(print), buffer->ptr, buffer->len);
//     ZEMU_LOGF(1000, "%s\n", print);
// #else
//     uint8_t buff[1000] = {0};
//     memcpy(buff, buffer->ptr, buffer->len);
//     printf("%s %s\n", title, buff);
// #endif
// }

void print_string(const char *str) {
#if defined(LEDGER_SPECIFIC)
    char print[1000] = {0};
    MEMCPY(print, str, strlen(str));
    ZEMU_LOGF(100, "%s\n", str);
#else
    printf("%s\n", str);
#endif
}

// void print_string_title(const char *str, const char *title) {
// #if defined(LEDGER_SPECIFIC)
//     ZEMU_LOGF(100, "%s: %s\n", title, str);
// #else
//     printf("%s: %s\n", title, str);
// #endif
// }

// void print_u8(const char *str, uint8_t val) {
// #if defined(LEDGER_SPECIFIC)
//     ZEMU_LOGF(100, "%s: %d\n", str, val);
// #else
//     printf("%s: %d\n", str, val);
// #endif
// }

// void print_u16(const char *str, uint16_t val) {
// #if defined(LEDGER_SPECIFIC)
//     ZEMU_LOGF(100, "%s: %d\n", str, val);
// #else
//     printf("%s: %d\n", str, val);
// #endif
// }

// void print_u32(const char *str, uint32_t val) {
// #if defined(LEDGER_SPECIFIC)
//     ZEMU_LOGF(100, "%s: %d\n", str, val);
// #else
//     printf("%s: %u\n", str, val);
// #endif
// }

// void print_u64(const char *str, uint64_t val) {
// #if defined(LEDGER_SPECIFIC)
//     ZEMU_LOGF(100, "%s: %lu\n", str, val);
// #else
//     printf("%s: %llu\n", str, val);
// #endif
// }

// void print_u64_hex(const char *str, uint64_t val) {
// #if defined(LEDGER_SPECIFIC)
//     ZEMU_LOGF(100, "%s: %lu\n", str, val);
// #else
//     printf("%s: 0x%llx\n", str, val);
// #endif
// }
