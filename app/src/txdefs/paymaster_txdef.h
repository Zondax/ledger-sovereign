/*******************************************************************************
 *  (c) 2018 - 2023 Zondax AG
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

#include "common_txdef.h"

typedef enum {
    PAYMASTER_CALL_REGISTER_PAYMASTER = 0,
    PAYMASTER_CALL_SET_PAYER_FOR_SEQUENCER,
    PAYMASTER_CALL_UPDATE_POLICY,
} paymaster_call_message_e;

typedef enum {
    PAYEE_POLICY_ALLOW = 0,
    PAYEE_POLICY_DENY,
} payee_policy_e;

typedef enum {
    AUTHORIZED_SEQUENCERS_ALL = 0,
    AUTHORIZED_SEQUENCERS_SOME,
} authorized_sequencers_e;

typedef struct {
    bool has_max_fee;
    amount_t max_fee;
    bool has_gas_limit;
    gas_t gas_limit;
    bool has_max_gas_price;
    gas_u128_t max_gas_price;
    bool has_transaction_limit;
    uint64_t transaction_limit;
} payee_policy_allow_t;

typedef struct {
    payee_policy_e type;
    union {
        payee_policy_allow_t allow;
    };
} payee_policy_t;

typedef struct {
    uint32_t length;
    struct {
        address_t address;
        payee_policy_t policy;
    } pairs[DEFAULT_SAFE_VEC_LEN];
} payee_policy_list_t;

typedef struct {
    authorized_sequencers_e type;
    union {
        address_list_t updaters;
    };
} authorized_sequencers_t;

typedef struct {
    payee_policy_t default_payee_policy;
    payee_policy_list_t payees;
    address_list_t authorized_updaters;
    authorized_sequencers_t authorized_sequencers;
} paymaster_policy_initializer_t;

typedef struct {
    paymaster_policy_initializer_t policy;
} paymaster_call_register_paymaster_t;

typedef struct {
    address_t payer;
} paymaster_call_set_payer_for_sequencer_t;

typedef struct {
    address_t payer;
    // TODO: implement this
    // policy_update_t update;
} paymaster_call_update_policy_t;

typedef struct {
    paymaster_call_message_e type;
    union {
        paymaster_call_register_paymaster_t register_paymaster;
        paymaster_call_set_payer_for_sequencer_t set_payer_for_sequencer;
        paymaster_call_update_policy_t update_policy;
    };
} paymaster_call_message_t;

#ifdef __cplusplus
}
#endif
