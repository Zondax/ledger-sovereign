/*******************************************************************************
 *   (c) 2018 - 2023 Zondax AG
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

#include "ui_utils.h"

#include <stdio.h>

#include "borsh.h"
#include "parser_common.h"
#include "zxerror.h"
#include "zxformat.h"
#include "zxmacros.h"

parser_error_t uint128_to_str(uint64_t high, uint64_t low, char *out, uint16_t out_len) {
    CHECK_INPUT(out);
    if (out_len < U128_STR_MAX_LEN) return parser_value_out_of_range;

    MEMZERO(out, out_len);
    char *p = out;

    if (high == 0 && low == 0) {
        *(p++) = '0';
        return parser_ok;
    }

    uint64_t temp_high = high;
    uint64_t temp_low = low;

    while (temp_high != 0 || temp_low != 0) {
        if (p - out >= (out_len - 1)) return parser_value_out_of_range;

        uint64_t quotient_high = 0;
        uint64_t quotient_low = 0;
        uint64_t remainder = 0;
        uint64_t current;

        current = temp_high;
        quotient_high = current / 10;
        remainder = current % 10;

        current = (remainder << 32) | (temp_low >> 32);
        uint64_t q = current / 10;
        remainder = current % 10;
        quotient_low = (q << 32);

        current = (remainder << 32) | (temp_low & 0xFFFFFFFF);
        q = current / 10;
        remainder = current % 10;
        quotient_low |= q;

        *(p++) = (char)('0' + remainder);
        temp_high = quotient_high;
        temp_low = quotient_low;
    }

    while (p > out) {
        p--;
        char z = *out;
        *out = *p;
        *p = z;
        out++;
    }

    return parser_ok;
}

parser_error_t render_number(uint64_t high, uint64_t low, uint8_t decimalPlaces, const char *postfix, const char *prefix,
                             char *outValue, uint16_t outValueLen) {
    CHECK_INPUT(outValue);
    if (outValueLen < U128_STR_MAX_LEN) return parser_value_out_of_range;

    CHECK_ERROR(uint128_to_str(high, low, outValue, outValueLen));

    if (intstr_to_fpstr_inplace(outValue, outValueLen, decimalPlaces) == 0) {
        return parser_unexpected_value;
    }

    if (z_str3join(outValue, outValueLen, prefix, postfix) != zxerr_ok) {
        return parser_unexpected_buffer_end;
    }

    number_inplace_trimming(outValue, 1);

    return parser_ok;
}

parser_error_t split_bracket_content(const char *input, char *first_content, uint16_t first_content_len, char *remaining,
                                     uint16_t remaining_len) {
    CHECK_INPUT(input);
    CHECK_INPUT(first_content);
    CHECK_INPUT(remaining);

    MEMZERO(first_content, first_content_len);
    MEMZERO(remaining, remaining_len);

    const char *first_open = strchr(input, '{');
    if (first_open == NULL) {
        return parser_ui_open_bracket_not_found;
    }

    const char *first_close = strchr(first_open, '}');
    if (first_close == NULL) {
        return parser_ui_close_bracket_not_found;
    }

    size_t content_len = first_close - first_open - 1;
    if (content_len >= first_content_len) {
        return parser_ui_find_bracket_buffer_too_small;
    }
    strncpy(first_content, first_open + 1, content_len);
    first_content[content_len] = '\0';

    size_t remaining_content_len = strlen(first_close + 1);
    if (remaining_content_len >= remaining_len) {
        return parser_ui_find_bracket_buffer_too_small;
    }
    strncpy(remaining, first_close + 1, remaining_content_len);
    remaining[remaining_content_len] = '\0';

    return parser_ok;
}

parser_error_t find_bracket_content(const char *input, uint8_t index, char *content, uint16_t content_len) {
    CHECK_INPUT(input);
    CHECK_INPUT(content);

    MEMZERO(content, content_len);

    const char *current = input;
    for (uint8_t i = 0; i <= index; i++) {
        current = strchr(current, '{');
        if (current == NULL) return parser_ui_open_bracket_not_found;

        if (i < index) {
            current++;
            continue;
        }

        const char *close = strchr(current, '}');
        if (close == NULL) return parser_ui_close_bracket_not_found;

        size_t len = close - current - 1;
        if (len >= content_len) return parser_ui_find_bracket_buffer_too_small;

        strncpy(content, current + 1, len);
        content[len] = '\0';
        return parser_ok;
    }

    return parser_unexpected_value;
}