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

#include "common_txdef.h"

typedef struct {
    uint32_t entries;
    parser_context_t data;
} merkle_leaves_data_t;

typedef struct {
    uint32_t entries;
    parser_context_t indices;
} merkle_leaves_indices_t;

typedef struct {
    uint32_t entries;
    parser_context_t data;
} merkle_lemmas_t;

typedef struct {
    merkle_leaves_data_t leaves;      // The list of the original data items at the bottom level of the Merkle tree
    merkle_leaves_indices_t indices;  // The list of indices of the leaves in the Merkle tree
    merkle_lemmas_t lemmas;           // The list of hashes needed to reconstruct the Merkle root from the leaf
    uint64_t tree_size;               // The size of the Merkle tree
    bytes_t root_hash;                // The hash of the Merkle root
} merkle_proof_t;

#ifdef __cplusplus
}
#endif
