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
    BANK_CALL_MESSAGE_CREATE_TOKEN = 0,
    BANK_CALL_MESSAGE_TRANSFER,
    BANK_CALL_MESSAGE_BURN,
    BANK_CALL_MESSAGE_MINT,
    BANK_CALL_MESSAGE_FREEZE,
} bank_call_message_e;

typedef struct {
    address_t to;
    coins_t coins;
} bank_transfer_t;

typedef struct {
    coins_t coins;
} bank_burn_t;

typedef struct {
    coins_t coins;
    address_t mint_to_address;
} bank_mint_t;

typedef struct {
    token_id_t token_id;
} bank_freeze_t;

typedef struct {
    bank_call_message_e type;
    union {
        bank_transfer_t transfer;
        bank_burn_t burn;
        bank_mint_t mint;
        bank_freeze_t freeze;
    };
} bank_call_message_t;

#ifdef __cplusplus
}
#endif
