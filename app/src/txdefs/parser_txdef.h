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

#include "bank_txdef.h"
#include "paymaster_txdef.h"
#include "schema_txdef.h"
#include "ui_txdef.h"
typedef struct {
    uint8_t type;
    union {
        bank_call_message_t bank;
        paymaster_call_message_t paymaster;
    };
} runtime_t;

typedef struct {
    uint64_t max_priority_fee_bips;
    amount_t max_fee;
    bool has_gas_limit;
    gas_t gas_limit;
    uint64_t chain_id;
} tx_details_t;

typedef struct {
    runtime_t runtime_call;
    uint64_t generation;
    tx_details_t tx_details;
} unsigned_transaction_t;

typedef struct {
    schema_t schema;
    merkle_proof_t merkle_proofs;
    parser_context_t unsigned_transaction_raw;
    ui_items_t ui_items;
} parser_tx_t;

#ifdef __cplusplus
}
#endif
