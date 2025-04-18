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

#include "paymaster_reader.h"

parser_error_t read_payee_policy_allow(parser_context_t *ctx, payee_policy_allow_t *allow) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(allow);

    CHECK_ERROR(read_u8(ctx, (uint8_t *)&allow->has_max_fee));
    print_u8("has max fee:", allow->has_max_fee);
    if (allow->has_max_fee) {
        CHECK_ERROR(read_amount(ctx, &allow->max_fee));
    }

    CHECK_ERROR(read_u8(ctx, (uint8_t *)&allow->has_gas_limit));
    print_u8("has gas limit:", allow->has_gas_limit);
    if (allow->has_gas_limit) {
        CHECK_ERROR(read_gas(ctx, &allow->gas_limit));
    }

    CHECK_ERROR(read_u8(ctx, (uint8_t *)&allow->has_max_gas_price));
    print_u8("has max gas price:", allow->has_max_gas_price);
    if (allow->has_max_gas_price) {
        CHECK_ERROR(read_gas_u128(ctx, &allow->max_gas_price));
    }

    CHECK_ERROR(read_u8(ctx, (uint8_t *)&allow->has_transaction_limit));
    print_u8("has transaction limit:", allow->has_transaction_limit);
    if (allow->has_transaction_limit) {
        CHECK_ERROR(read_u64(ctx, &allow->transaction_limit));
        print_u64("transaction limit:", allow->transaction_limit);
    }

    return parser_ok;
}

parser_error_t read_payee_policy(parser_context_t *ctx, payee_policy_t *policy) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(policy);

    CHECK_ERROR(read_u8(ctx, (uint8_t *)&policy->type));
    switch (policy->type) {
        case PAYEE_POLICY_ALLOW:
            print_string("PAYEE_POLICY_ALLOW");
            CHECK_ERROR(read_payee_policy_allow(ctx, &policy->allow));
            break;
        case PAYEE_POLICY_DENY:
            print_string("PAYEE_POLICY_DENY");
            break;
        default:
            print_u8("Unexpected payee policy type:", policy->type);
            return parser_unexpected_error;
    }
    return parser_ok;
}

parser_error_t read_payee_policy_list(parser_context_t *ctx, payee_policy_list_t *payees) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(payees);

    CHECK_ERROR(read_u32(ctx, &payees->length));
    print_u32("payees list length:", payees->length);
    for (int i = 0; i < (int)payees->length; i++) {
        CHECK_ERROR(read_address(ctx, &payees->pairs[i].address));
        CHECK_ERROR(read_payee_policy(ctx, &payees->pairs[i].policy));
    }
    return parser_ok;
}

parser_error_t read_authorized_updaters(parser_context_t *ctx, address_list_t *updaters) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(updaters);

    CHECK_ERROR(read_u32(ctx, &updaters->length));
    print_u32("authorized updaters length:", updaters->length);
    for (int i = 0; i < (int)updaters->length; i++) {
        CHECK_ERROR(read_address(ctx, &updaters->address[i]));
    }
    return parser_ok;
}

parser_error_t read_authorized_sequencers(parser_context_t *ctx, authorized_sequencers_t *sequencers) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(sequencers);

    CHECK_ERROR(read_u8(ctx, (uint8_t *)&sequencers->type));
    switch (sequencers->type) {
        case AUTHORIZED_SEQUENCERS_ALL:
            print_string("AUTHORIZED_SEQUENCERS_ALL");
            break;
        case AUTHORIZED_SEQUENCERS_SOME:
            print_string("AUTHORIZED_SEQUENCERS_SOME");
            CHECK_ERROR(read_authorized_updaters(ctx, &sequencers->updaters));
            break;
    }

    return parser_ok;
}

parser_error_t read_paymaster_register_paymaster_policy(parser_context_t *ctx, paymaster_policy_initializer_t *policy) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(policy);

    CHECK_ERROR(read_payee_policy(ctx, &policy->default_payee_policy));
    CHECK_ERROR(read_payee_policy_list(ctx, &policy->payees));
    CHECK_ERROR(read_authorized_updaters(ctx, &policy->authorized_updaters));
    CHECK_ERROR(read_authorized_sequencers(ctx, &policy->authorized_sequencers));

    return parser_ok;
}

parser_error_t read_paymaster_register_paymaster(parser_context_t *ctx, paymaster_call_register_paymaster_t *paymaster) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(paymaster);
    CHECK_ERROR(read_paymaster_register_paymaster_policy(ctx, &paymaster->policy));

    return parser_ok;
}

parser_error_t read_paymaster_call_message(parser_context_t *ctx, paymaster_call_message_t *paymaster) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(paymaster);

    // read call message type
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&paymaster->type));
    switch (paymaster->type) {
        case PAYMASTER_CALL_REGISTER_PAYMASTER:
            print_string("PAYMASTER_CALL_REGISTER_PAYMASTER");
            CHECK_ERROR(read_paymaster_register_paymaster(ctx, &paymaster->register_paymaster));
            break;
        case PAYMASTER_CALL_SET_PAYER_FOR_SEQUENCER:
            print_string("PAYMASTER_CALL_SET_PAYER_FOR_SEQUENCER_NOT_IMPLEMENTED");
            return parser_unexpected_error;
        case PAYMASTER_CALL_UPDATE_POLICY:
            print_string("PAYMASTER_CALL_UPDATE_POLICY_NOT_IMPLEMENTED");
            return parser_unexpected_error;
        default:
            print_u8("Unexpected paymaster call message type:", paymaster->type);
            return parser_unexpected_error;
    }
    return parser_ok;
}
