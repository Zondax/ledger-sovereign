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

#include <stddef.h>
#include <stdint.h>

#include "parser_txdef.h"

#define CHECK_ERROR(__CALL)                   \
    {                                         \
        parser_error_t __err = __CALL;        \
        CHECK_APP_CANARY()                    \
        if (__err != parser_ok) return __err; \
    }

typedef enum {
    // Generic errors
    parser_ok = 0,
    parser_no_data,
    parser_init_context_empty,
    parser_display_idx_out_of_range,
    parser_display_page_out_of_range,
    parser_unexpected_error,

    // Coin generic
    parser_unexpected_type,
    parser_unexpected_method,
    parser_unexpected_buffer_end,
    parser_encoding_failed,
    parser_invalid_crypto_settings,
    parser_ledger_api_error,
    parser_unexpected_value,
    parser_unexpected_number_items,
    parser_unexpected_version,
    parser_unexpected_characters,
    parser_unexpected_field,
    parser_duplicated_field,
    parser_value_out_of_range,
    parser_invalid_address,
    parser_unexpected_chain,
    parser_missing_field,
    parser_unknown_transaction,
    parser_running_out_of_stack,
    parser_root_type_indices_overflow,
    parser_unexpected_root_hash,
    parser_unexpected_chain_hash,
    parser_schema_index_not_found,
    parser_scheme_discriminant_overflow,
    parser_name_registry_not_found,
    parser_too_many_items,
    parser_push_item_too_long,

    // parser specific
    parser_schema_fixed_point_unknown_type,
    parser_schema_integer_display_unknown_type,
    parser_schema_byte_display_unknown_type,
    parser_schema_primitive_unknown_type,
    parser_schema_linking_scheme_unknown_type,
    parser_schema_link_unknown_type,
    parser_schema_parser_txn_failed,

    // ui specific
    parser_ui_item_title_empty,
    parser_ui_open_bracket_not_found,
    parser_ui_close_bracket_not_found,
    parser_ui_find_bracket_buffer_too_small,
    parser_ui_buffer_not_initialized,
    parser_ui_buffer_init_failed,
    parser_ui_buffer_too_small,
} parser_error_t;

#ifdef __cplusplus
}
#endif
