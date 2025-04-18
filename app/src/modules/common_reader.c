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

#include "common_reader.h"

parser_error_t read_amount(parser_context_t *ctx, amount_t *amount) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(amount);

    CHECK_ERROR(read_u64(ctx, &amount->lo));
    CHECK_ERROR(read_u64(ctx, &amount->hi));
    print_u64_hex("amount:", amount->lo);
    print_u64_hex("amount:", amount->hi);

    print_u64("amount:", amount->lo);
    print_u64("amount:", amount->hi);

    return parser_ok;
}

parser_error_t read_token_id(parser_context_t *ctx, token_id_t *token_id) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(token_id);

    token_id->token.len = TOKEN_ID_SIZE;
    token_id->token.ptr = ctx->buffer.ptr + ctx->offset;
    CTX_CHECK_AND_ADVANCE(ctx, TOKEN_ID_SIZE)

    print_buffer(&token_id->token, "token_id");

    return parser_ok;
}

parser_error_t read_address(parser_context_t *ctx, address_t *address) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(address);

    CHECK_ERROR(read_u8(ctx, (uint8_t *)&address->type));
    print_u8("address type", address->type);

    address->address.len = ADDRESS_SIZE;
    address->address.ptr = ctx->buffer.ptr + ctx->offset;
    CTX_CHECK_AND_ADVANCE(ctx, ADDRESS_SIZE);

    print_buffer(&address->address, "address");

    return parser_ok;
}

parser_error_t read_coins(parser_context_t *ctx, coins_t *coins) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(coins);

    CHECK_ERROR(read_amount(ctx, &coins->amount));
    CHECK_ERROR(read_token_id(ctx, &coins->token_id));

    return parser_ok;
}

parser_error_t read_gas(parser_context_t *ctx, gas_t *gas) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(gas);

    for (int i = 0; i < GAS_DIMENSIONS; i++) {
        CHECK_ERROR(read_u64(ctx, &gas->gas[i]));
        print_u64_hex("gas[%d]:", gas->gas[i]);
        print_u64("gas[%d]:", gas->gas[i]);
    }

    return parser_ok;
}

parser_error_t read_gas_u128(parser_context_t *ctx, gas_u128_t *gas) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(gas);

    for (int i = 0; i < GAS_DIMENSIONS; i++) {
        CHECK_ERROR(read_amount(ctx, &gas->gas[i]));
    }

    return parser_ok;
}
