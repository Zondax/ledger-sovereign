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
#include "parser_common.h"

/**
 * Converts a 128-bit unsigned integer to its decimal string representation.
 * Output buffer will be null terminated and cleared using MEMZERO.
 *
 * @param[out] data    Output buffer for the resulting string
 * @param[in]  dataLen Size of the output buffer
 * @param[in]  high    Upper 64 bits of the 128-bit number
 * @param[in]  low     Lower 64 bits of the 128-bit number
 *
 * @return parser_error_t:
 *         - parser_ok on success
 *         - parser_no_data if data is NULL
 *         - parser_value_out_of_range if buffer too small (< U128_STR_MAX_LEN)
 */
parser_error_t uint128_to_str(uint64_t high, uint64_t low, char *out, uint16_t out_len);

parser_error_t render_number(uint64_t high, uint64_t low, uint8_t decimalPlaces, const char *postfix, const char *prefix,
                             char *outValue, uint16_t outValueLen);

parser_error_t split_bracket_content(const char *input, char *first_content, uint16_t first_content_len, char *remaining,
                                     uint16_t remaining_len);

parser_error_t find_bracket_content(const char *input, uint8_t index, char *content, uint16_t content_len);
