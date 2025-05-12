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

parser_error_t schema_display_generic_by_index(parser_context_t *ctx, parser_tx_t *txObj, uint32_t start_index);
parser_error_t schema_display_by_primitive(parser_context_t *ctx, parser_tx_t *txObj, primitive_t *primitive);
parser_error_t schema_parser_transaction(parser_context_t *ctx, parser_tx_t *txObj);

#ifdef __cplusplus
}
#endif
