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

#include "zxerror.h"

parser_error_t _read(parser_context_t *c, parser_tx_t *v) {
    UNUSED(c);
    UNUSED(v);
    // #{TODO} --> parse parameters: read from c->buffer and store in v
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

        case parser_display_idx_out_of_range:
            return "display index out of range";
        case parser_display_page_out_of_range:
            return "display page out of range";

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
