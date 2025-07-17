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

#include "parser_common.h"

parser_error_t schema_find_index(uint64_t index_leaf, merkle_leaves_indices_t *indices, uint64_t *index_vec,
                                 bool *found);
parser_error_t schema_move_leaf_offset(merkle_leaves_data_t *leaves, uint64_t index);
parser_error_t schema_reset_leaf_offset(merkle_leaves_data_t *leaves);
parser_error_t get_schema_type(parser_tx_t *txObj, uint32_t index, uint8_t *type);
parser_error_t schema_get_unsigned_transaction_index(parser_tx_t *txObj, uint64_t *root_index);
bool is_link_skip(link_t *link);
bool should_show_field(link_t *value, bool silent, bool field_is_expert, bool expert_mode);
parser_error_t find_bracket_content(const char *input, uint8_t index, char *content, uint16_t content_len);

#ifdef __cplusplus
}
#endif
