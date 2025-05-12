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
#include "common_reader.h"
#include "paymaster_reader.h"
#include "schema_display.h"
#include "schema_helper.h"
#include "schema_reader.h"

parser_error_t read_runtime(parser_context_t *ctx, runtime_t *runtime) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(runtime);

    // read runtime type
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&runtime->type));
    print_u8("runtime type:", runtime->type);
    switch (runtime->type) {
        case RUNTIME_CALL_BANK:
            print_string("RUNTIME_CALL_BANK");
            CHECK_ERROR(read_bank_call_message(ctx, &runtime->bank));
            break;
        case RUNTIME_CALL_PAYMASTER:
            print_string("RUNTIME_CALL_PAYMASTER");
            CHECK_ERROR(read_paymaster_call_message(ctx, &runtime->paymaster));
            break;
        default:
            print_u8("Unexpected runtime type:", runtime->type);
            return parser_unexpected_error;
    }

    return parser_ok;
}

parser_error_t read_tx_details(parser_context_t *ctx, tx_details_t *tx_details) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(tx_details);
    // read max priority fee bips
    CHECK_ERROR(read_u64(ctx, &tx_details->max_priority_fee_bips));
    print_u64_hex("max priority fee bips:", tx_details->max_priority_fee_bips);

    // read max fee
    CHECK_ERROR(read_amount(ctx, &tx_details->max_fee));
    print_u64("max fee:", tx_details->max_fee.lo);
    print_u64("max fee:", tx_details->max_fee.hi);

    // read gas limit
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&tx_details->has_gas_limit));
    print_u8("has gas limit:", tx_details->has_gas_limit);
    if (tx_details->has_gas_limit) {
        for (int i = 0; i < GAS_DIMENSIONS; i++) {
            CHECK_ERROR(read_u64(ctx, &tx_details->gas_limit.gas[i]));
            print_u64("gas[%d]:", tx_details->gas_limit.gas[i]);
        }
    }

    // read chain id
    CHECK_ERROR(read_u64(ctx, &tx_details->chain_id));
    print_u64_hex("chain id:", tx_details->chain_id);

    return parser_ok;
}

parser_error_t unsigned_transaction_read(parser_context_t *ctx, parser_tx_t *txObj) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(txObj);

    // // read runtime
    // CHECK_ERROR(read_runtime(ctx, &txObj->unsigned_transaction.runtime_call));
    // // read generation
    // CHECK_ERROR(read_u64(ctx, &txObj->unsigned_transaction.generation));
    // print_u64_hex("generation:", txObj->unsigned_transaction.generation);

    // // read tx details
    // CHECK_ERROR(read_tx_details(ctx, &txObj->unsigned_transaction.tx_details));

    return parser_ok;
}
