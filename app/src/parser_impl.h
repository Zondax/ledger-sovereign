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

#include <zxmacros.h>

#include "parser_common.h"
#include "parser_txdef.h"
#include "zxtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CHECK_INPUT(val)       \
    if (val == NULL) {         \
        return parser_no_data; \
    }

// Checks that there are at least SIZE bytes available in the buffer
#define CTX_CHECK(CTX, SIZE)                                             \
    if ((CTX) == NULL || ((CTX)->offset + (SIZE)) > (CTX)->buffer.len) { \
        return parser_unexpected_buffer_end;                             \
    }

#define CTX_CHECK_AND_ADVANCE(CTX, SIZE) \
    CTX_CHECK((CTX), (SIZE))             \
    (CTX)->offset += (SIZE);

#define MAP_ZXERR_TO_PARSER_ERR(zxerr)                                           \
    ((zxerr) == zxerr_ok                        ? parser_ok                      \
     : (zxerr) == zxerr_no_data                 ? parser_no_data                 \
     : (zxerr) == zxerr_buffer_too_small        ? parser_unexpected_buffer_end   \
     : (zxerr) == zxerr_out_of_bounds           ? parser_unexpected_buffer_end   \
     : (zxerr) == zxerr_encoding_failed         ? parser_encoding_failed         \
     : (zxerr) == zxerr_invalid_crypto_settings ? parser_invalid_crypto_settings \
     : (zxerr) == zxerr_ledger_api_error        ? parser_ledger_api_error        \
                                                : parser_unexpected_error)

// #{TODO} --> functions to parse, get, process transaction fields
parser_error_t _read(parser_context_t *c, parser_tx_t *v);

#ifdef __cplusplus
}
#endif
