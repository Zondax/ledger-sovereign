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

#include <stdbool.h>
#include <stdint.h>

#include "crypto_helper.h"
#include "merkle_txdef.h"
#include "parser_common.h"
#include "parser_txdef.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    merkle_leaves_data_t leaves;
    merkle_leaves_indices_t indices;
    merkle_lemmas_t lemmas;
    int64_t lemma_index;
    uint64_t tree_size;
} proof_t;

parser_error_t verify_merkle_proofs(const merkle_proof_t *metadata);

#ifdef __cplusplus
}
#endif
