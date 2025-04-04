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

#include "borsh.h"
#include "parser_common.h"

parser_error_t read_amount(parser_context_t *ctx, amount_t *amount);
parser_error_t read_token_id(parser_context_t *ctx, token_id_t *token_id);
parser_error_t read_address(parser_context_t *ctx, address_t *address);
parser_error_t read_coins(parser_context_t *ctx, coins_t *coins);
parser_error_t read_gas(parser_context_t *ctx, gas_t *gas);
parser_error_t read_gas_u128(parser_context_t *ctx, gas_u128_t *gas);

#ifdef __cplusplus
}
#endif
