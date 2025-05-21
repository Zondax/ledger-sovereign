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

#include "parser_impl.h"

#include "borsh.h"
#include "schema_reader.h"
#include "schema_txn_parser.h"
#include "stack_manager.h"
#include "zxerror.h"

parser_error_t _read(parser_context_t *c, parser_tx_t *v) {
    checkStack();

    CHECK_ERROR(schema_merkle_proofs_read(c, v));
    CHECK_ERROR(schema_extra_data_read(c, v));
    CHECK_ERROR(schema_parser_transaction(c, v));
    CHECK_ERROR(schema_chain_hash_read(c, v));

    if (c->offset != c->buffer.len) {
        return parser_unexpected_error;
    }

    return parser_ok;
}

const char *parser_getErrorDescription(parser_error_t err) {
    switch (err) {
        case parser_ok:
            return "No error";
        case parser_no_data:
            return "No more data";
        case parser_init_context_empty:
            return "Initialized empty context";
        case parser_unexpected_buffer_end:
            return "Unexpected buffer end";
        case parser_encoding_failed:
            return "Encoding failed";
        case parser_invalid_crypto_settings:
            return "Invalid crypto settings";
        case parser_ledger_api_error:
            return "Ledger API error";
        case parser_unexpected_type:
            return "Unexpected type";
        case parser_unexpected_method:
            return "Unexpected method";
        case parser_unexpected_version:
            return "Unexpected version";
        case parser_unexpected_characters:
            return "Unexpected characters";
        case parser_unexpected_field:
            return "Unexpected field";
        case parser_duplicated_field:
            return "Unexpected duplicated field";
        case parser_value_out_of_range:
            return "Value out of range";
        case parser_unexpected_chain:
            return "Unexpected chain";
        case parser_missing_field:
            return "missing field";
        case parser_unknown_transaction:
            return "unknown transaction";
        case parser_running_out_of_stack:
            return "running out of stack";

        case parser_display_idx_out_of_range:
            return "display index out of range";
        case parser_display_page_out_of_range:
            return "display page out of range";
        case parser_root_type_indices_overflow:
            return "root type indices overflow";
        case parser_unexpected_root_hash:
            return "unexpected root hash";
        case parser_unexpected_chain_hash:
            return "unexpected chain hash";
        case parser_schema_index_not_found:
            return "schema index not found";
        case parser_scheme_discriminant_overflow:
            return "scheme discriminant overflow";
        case parser_name_registry_not_found:
            return "name registry not found";
        case parser_too_many_items:
            return "too many items";
        case parser_push_item_too_long:
            return "push item too long";

        // parser specific
        case parser_schema_fixed_point_unknown_type:
            return "fixed point unknown type";
        case parser_schema_integer_display_unknown_type:
            return "integer display unknown type";
        case parser_schema_byte_display_unknown_type:
            return "byte display unknown type";
        case parser_schema_primitive_unknown_type:
            return "primitive unknown type";
        case parser_schema_linking_scheme_unknown_type:
            return "linking scheme unknown type";
        case parser_schema_link_unknown_type:
            return "link unknown type";
        case parser_schema_parser_txn_failed:
            return "parser txn failed";

        // ui specific
        case parser_ui_item_title_empty:
            return "item title empty";
        case parser_ui_open_bracket_not_found:
            return "open bracket not found";
        case parser_ui_close_bracket_not_found:
            return "close bracket not found";
        case parser_ui_find_bracket_buffer_too_small:
            return "find bracket buffer too small";
        case parser_ui_buffer_not_initialized:
            return "buffer not initialized";
        case parser_ui_buffer_init_failed:
            return "buffer init failed";
        case parser_ui_buffer_too_small:
            return "buffer too small";

        default:
            return "Unrecognized error code";
    }
}

const char *parser_getZxErrorDescription(zxerr_t err) {
    switch (err) {
        case zxerr_unknown:
            return "error unknown";
        case zxerr_ok:
            return "No error";
        case zxerr_no_data:
            return "No more data";
        case zxerr_buffer_too_small:
            return "Buffer too small";
        case zxerr_out_of_bounds:
            return "Out of bound";
        case zxerr_encoding_failed:
            return "Encoding failed";
        case zxerr_invalid_crypto_settings:
            return "Invalid crypto settings";
        case zxerr_ledger_api_error:
            return "Api error";

        default:
            return "Unrecognized error code";
    }
}
