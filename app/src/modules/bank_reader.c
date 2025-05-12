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

#include "bank_reader.h"

parser_error_t read_bank_transfer(parser_context_t *ctx, bank_transfer_t *transfer) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(transfer);

    CHECK_ERROR(read_address(ctx, &transfer->to));
    CHECK_ERROR(read_coins(ctx, &transfer->coins));

    return parser_ok;
}

parser_error_t read_bank_mint(parser_context_t *ctx, bank_mint_t *mint) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(mint);

    CHECK_ERROR(read_coins(ctx, &mint->coins));
    CHECK_ERROR(read_address(ctx, &mint->mint_to_address));

    return parser_ok;
}

parser_error_t read_bank_burn(parser_context_t *ctx, bank_burn_t *burn) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(burn);

    CHECK_ERROR(read_coins(ctx, &burn->coins));
    return parser_ok;
}

parser_error_t read_bank_freeze(parser_context_t *ctx, bank_freeze_t *freeze) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(freeze);

    CHECK_ERROR(read_token_id(ctx, &freeze->token_id));
    return parser_ok;
}

parser_error_t read_bank_call_message(parser_context_t *ctx, bank_call_message_t *bank) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(bank);

    // read call message type
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&bank->type));
    switch (bank->type) {
        case BANK_CALL_MESSAGE_CREATE_TOKEN:
            print_string("BANK_CALL_MESSAGE_CREATE_TOKEN NOT IMPLEMENTED");
            return parser_unexpected_error;
        case BANK_CALL_MESSAGE_TRANSFER:
            CHECK_ERROR(read_bank_transfer(ctx, &bank->transfer));
            break;
        case BANK_CALL_MESSAGE_BURN:
            CHECK_ERROR(read_bank_burn(ctx, &bank->burn));
            break;
        case BANK_CALL_MESSAGE_MINT:
            CHECK_ERROR(read_bank_mint(ctx, &bank->mint));
            break;
        case BANK_CALL_MESSAGE_FREEZE:
            CHECK_ERROR(read_bank_freeze(ctx, &bank->freeze));
            break;
        default:
            print_u8("Unexpected bank call message type:", bank->type);
            return parser_unexpected_error;
    }
    return parser_ok;
}
