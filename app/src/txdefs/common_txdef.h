/*******************************************************************************
 *  (c) 2018 - 2025 Zondax AG
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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define GAS_DIMENSIONS 2
#define TOKEN_ID_SIZE 32
#define ADDRESS_SIZE 28
#define DEFAULT_SAFE_VEC_LEN 20
#define MAX_INPUT_CHUNK 256

typedef enum {
    RUNTIME_CALL_BANK = 0,
    RUNTIME_CALL_SEQUENCER_REGISTRY,
    RUNTIME_CALL_VALUE_SETTER,
    RUNTIME_CALL_ATTESTER_INCENTIVES,
    RUNTIME_CALL_PROVER_INCENTIVES,
    RUNTIME_CALL_ACCOUNTS,
    RUNTIME_CALL_UNIQUENESS,
    RUNTIME_CALL_CHAIN_STATE,
    RUNTIME_CALL_BLOB_STORAGE,
    RUNTIME_CALL_PAYMASTER,
    RUNTIME_CALL_EVM,
    RUNTIME_CALL_ACCESS_PATTERN,
} runtime_call_e;

typedef enum {
    ADDRESS_TYPE_STANDARD = 0,
    ADDRESS_TYPE_VM,
} address_e;

typedef struct {
    uint64_t hi;
    uint64_t lo;
} uint128_t;

typedef struct {
    const uint8_t *ptr;
    uint16_t len;
} bytes_t;

typedef struct {
    bytes_t buffer;
    uint16_t offset;
} parser_context_t;

typedef struct {
    uint64_t hi;
    uint64_t lo;
} amount_t;

typedef struct {
    bytes_t token;
} token_id_t;

typedef struct {
    amount_t amount;
    token_id_t token_id;
} coins_t;

typedef struct {
    uint64_t gas[GAS_DIMENSIONS];
} gas_t;

typedef struct {
    amount_t gas[GAS_DIMENSIONS];
} gas_u128_t;

typedef struct {
    address_e type;
    bytes_t address;
} address_t;

typedef struct {
    uint32_t length;
    address_t address[DEFAULT_SAFE_VEC_LEN];
} address_list_t;

#ifdef __cplusplus
}
#endif
