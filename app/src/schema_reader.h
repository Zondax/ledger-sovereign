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

parser_error_t read_struct(parser_context_t *ctx, schema_struct_t *schema_struct);
parser_error_t read_enum(parser_context_t *ctx, schema_enum_t *schema_enum);
parser_error_t read_tuple(parser_context_t *ctx, schema_tuple_t *schema_tuple);
parser_error_t read_option(parser_context_t *ctx, schema_option_t *schema_option);
parser_error_t read_array(parser_context_t *ctx, schema_array_t *schema_array);
parser_error_t read_link(parser_context_t *ctx, link_t *link);

parser_error_t metadata_read(parser_context_t *ctx, parser_tx_t *txObj);
parser_error_t merkle_proofs_read(parser_context_t *ctx, parser_tx_t *txObj);
parser_error_t get_leave_index(merkle_leaves_data_t *leaves, merkle_leaves_indices_t *indices, uint32_t schema_index,
                               uint32_t field_index[], uint16_t *qty, uint16_t max_indexes);

#ifdef __cplusplus
}
#endif
