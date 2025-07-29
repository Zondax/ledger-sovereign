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

#include "ui_item_buffer.h"

#include <stdio.h>

#include "borsh.h"
#include "parser_common.h"
#include "zxerror.h"
#include "zxformat.h"
#include "zxmacros.h"

parser_error_t init_item_buffer(item_buffer_t *buffer, const char *initial_data, const char *separator_open,
                                const char *separator_close) {
    CHECK_INPUT(buffer);
    CHECK_INPUT(separator_open);
    CHECK_INPUT(separator_close);

    MEMZERO(buffer->data, sizeof(buffer->data));
    MEMZERO(buffer->separator_open, sizeof(buffer->separator_open));
    MEMZERO(buffer->separator_close, sizeof(buffer->separator_close));
    buffer->separator_open_len = strlen(separator_open);
    buffer->separator_close_len = strlen(separator_close);
    MEMCPY(buffer->separator_open, separator_open, buffer->separator_open_len);
    MEMCPY(buffer->separator_close, separator_close, buffer->separator_close_len);
    buffer->qty = 0;
    buffer->initialized = true;
    if (initial_data != NULL) {
        strncpy(buffer->data, initial_data, sizeof(buffer->data) - 1);
        buffer->data[sizeof(buffer->data) - 1] = '\0';
        if (get_item_buffer_qty(buffer, &buffer->qty) != parser_ok) {
            buffer->initialized = false;
            return parser_ui_buffer_init_failed;
        }
    }
    return parser_ok;
}

parser_error_t get_item_buffer_qty(const item_buffer_t *buffer, uint8_t *qty) {
    CHECK_INPUT(buffer);
    CHECK_INPUT(qty);

    if (!buffer->initialized) {
        return parser_ui_buffer_not_initialized;
    }

    uint8_t count = 0;
    const char *p = buffer->data;
    const char *open = buffer->separator_open;
    const char *close = buffer->separator_close;
    size_t open_len = buffer->separator_open_len;
    size_t close_len = buffer->separator_close_len;

    // Skip leading open separators
    while (strncmp(p, open, open_len) == 0) {
        p += open_len;
    }

    // Count items between separators
    while (*p) {
        if (strncmp(p, close, close_len) == 0) {
            count++;
            // Skip to next open separator
            while (*p && strncmp(p, open, open_len) != 0) {
                p++;
            }
            if (strncmp(p, open, open_len) == 0) {
                p += open_len;
            }
        } else {
            p++;
        }
    }

    // If the string isn't empty and doesn't end with a close separator, count the last item
    if (p > buffer->data && strncmp(p - close_len, close, close_len) != 0) {
        count++;
    }

    *qty = count;
    return parser_ok;
}

parser_error_t append_item_buffer(item_buffer_t *buffer, const char *input, uint16_t input_len) {
    CHECK_INPUT(buffer);
    CHECK_INPUT(input);

    if (strlen(buffer->data) + input_len >= sizeof(buffer->data)) {
        return parser_unexpected_error;
    }
    if (strlen(buffer->data) < buffer->separator_open_len ||
        MEMCMP(buffer->data + strlen(buffer->data) - buffer->separator_open_len, buffer->separator_open,
               buffer->separator_open_len) != 0) {
        strncat(buffer->data, buffer->separator_open, buffer->separator_open_len);
    }

    strncat(buffer->data, input, input_len);

    if (strlen(buffer->data) < buffer->separator_close_len ||
        MEMCMP(buffer->data + strlen(buffer->data) - buffer->separator_close_len, buffer->separator_close,
               buffer->separator_close_len) != 0) {
        strncat(buffer->data, buffer->separator_close, buffer->separator_close_len);
    }

    return parser_ok;
}

parser_error_t clear_item_buffer(item_buffer_t *buffer) {
    CHECK_INPUT(buffer);
    if (!buffer->initialized) {
        return parser_ui_buffer_not_initialized;
    }
    MEMZERO(buffer->data, sizeof(buffer->data));
    buffer->qty = 0;
    return parser_ok;
}

parser_error_t is_item_buffer_empty(item_buffer_t *buffer, bool *is_empty) {
    CHECK_INPUT(buffer);
    CHECK_INPUT(is_empty);

    if (!buffer->initialized) {
        return parser_ui_buffer_not_initialized;
    }
    *is_empty = buffer->data[0] == '\0';

    return parser_ok;
}

parser_error_t remove_last_item_buffer(item_buffer_t *buffer) {
    CHECK_INPUT(buffer);
    if (!buffer->initialized) {
        return parser_ui_buffer_not_initialized;
    }

    // Find last separator_close
    char *last_close = NULL;
    for (char *p = buffer->data; *p; p++) {
        if (MEMCMP(p, buffer->separator_close, buffer->separator_close_len) == 0) {
            last_close = p;
        }
    }

    if (last_close == NULL || last_close == buffer->data) {
        buffer->data[0] = '\0';
        return parser_ok;
    }

    // Null-terminate at the last separator_close
    *last_close = '\0';

    // Find second last separator_close
    char *second_last_close = NULL;
    for (char *p = buffer->data; p < last_close; p++) {
        if (MEMCMP(p, buffer->separator_close, buffer->separator_close_len) == 0) {
            second_last_close = p;
        }
    }

    if (second_last_close != NULL) {
        // Restore the last separator_close
        MEMCPY(last_close, buffer->separator_close, buffer->separator_close_len);
        // Null-terminate after the second last separator_close
        *(second_last_close + buffer->separator_close_len) = '\0';
        // clear the buffer if it's empty
        if (strlen(buffer->data) == buffer->separator_open_len &&
            MEMCMP(buffer->data, buffer->separator_open, buffer->separator_open_len) == 0) {
            buffer->data[0] = '\0';
        }

    } else {
        // If no second last separator, clear the buffer
        buffer->data[0] = '\0';
    }

    return parser_ok;
}

parser_error_t get_item_buffer_content(item_buffer_t *buffer, uint8_t index, char *content, uint16_t content_len) {
    CHECK_INPUT(buffer);
    CHECK_INPUT(content);

    MEMZERO(content, content_len);

    const char *current = buffer->data;
    for (uint8_t i = 0; i <= index; i++) {
        while (*current && MEMCMP(current, buffer->separator_open, buffer->separator_open_len) != 0) {
            current++;
        }
        if (*current == '\0') return parser_ui_separator_not_found;

        if (i < index) {
            current++;
            continue;
        }

        const char *close = current + 1;
        while (*close && MEMCMP(close, buffer->separator_close, buffer->separator_close_len) != 0) {
            close++;
        }
        if (*close == '\0') return parser_ui_separator_not_found;

        size_t len = close - current - buffer->separator_open_len;
        if (len >= content_len) return parser_ui_buffer_too_small;

        strncpy(content, current + buffer->separator_open_len, len);
        content[len] = '\0';
        return parser_ok;
    }

    return parser_unexpected_value;
}

parser_error_t get_item_buffer_content_length(item_buffer_t *buffer, uint8_t index, size_t *length) {
    CHECK_INPUT(buffer);
    CHECK_INPUT(length);

    *length = 0;

    const char *current = buffer->data;
    for (uint8_t i = 0; i <= index; i++) {
        while (*current && MEMCMP(current, buffer->separator_open, buffer->separator_open_len) != 0) {
            current++;
        }
        if (*current == '\0') return parser_ui_separator_not_found;

        if (i < index) {
            current++;
            continue;
        }

        const char *close = current + 1;
        while (*close && MEMCMP(close, buffer->separator_close, buffer->separator_close_len) != 0) {
            close++;
        }
        if (*close == '\0') return parser_ui_separator_not_found;

        *length = close - current - buffer->separator_open_len;
        return parser_ok;
    }

    return parser_unexpected_value;
}

parser_error_t get_item_buffer_range_length(item_buffer_t *buffer, uint8_t index_start, uint8_t index_end,
                                            size_t *total_length) {
    CHECK_INPUT(buffer);
    CHECK_INPUT(total_length);

    *total_length = 0;
    size_t content_length = 0;

    if (index_start >= index_end) {
        return parser_ui_item_title_empty;
    }

    for (uint8_t i = index_start; i < index_end; i++) {
        CHECK_ERROR(get_item_buffer_content_length(buffer, i, &content_length));
        *total_length += content_length;

        // Add separator length for all items except the last one
        if (i < index_end - 1) {
            *total_length += strlen(SEPARATOR_TITLE_DISPLAY);
        }
    }

    return parser_ok;
}

parser_error_t get_item_buffer_range(item_buffer_t *buffer, uint8_t index_start, uint8_t index_end,
                                     item_buffer_t *output) {
    CHECK_INPUT(buffer);
    CHECK_INPUT(output);

    MEMZERO(output->data, sizeof(output->data));
    char content[100] = {0};

    for (uint8_t i = index_start; i <= index_end; i++) {
        CHECK_ERROR(get_item_buffer_content(buffer, i, content, sizeof(content)));
        CHECK_ERROR(append_item_buffer(output, content, strlen(content)));
    }

    return parser_ok;
}

parser_error_t remove_first_separator(item_buffer_t *buffer, char *output, uint16_t output_len) {
    CHECK_INPUT(buffer);
    CHECK_INPUT(output);

    if (!buffer->initialized) {
        return parser_ui_buffer_not_initialized;
    }

    // Find first separator_open
    char *first_open = NULL;
    for (char *p = buffer->data; *p; p++) {
        if (MEMCMP(p, buffer->separator_open, buffer->separator_open_len) == 0) {
            first_open = p;
            break;
        }
    }

    if (first_open == NULL) {
        return parser_ok;
    }

    strncpy(output, buffer->data + buffer->separator_open_len, output_len);
    uint16_t len = strlen(output);
    output[len] = '\0';

    return parser_ok;
}

parser_error_t remove_last_separator(item_buffer_t *buffer, char *output, uint16_t output_len) {
    CHECK_INPUT(buffer);
    CHECK_INPUT(output);

    if (!buffer->initialized) {
        return parser_ui_buffer_not_initialized;
    }

    // Find last separator_close
    char *last_close = NULL;
    for (char *p = buffer->data; *p; p++) {
        if (MEMCMP(p, buffer->separator_close, buffer->separator_close_len) == 0) {
            last_close = p;
        }
    }

    if (last_close == NULL) {
        return parser_ok;
    }

    // Null-terminate at the last separator_close
    *last_close = '\0';

    strncpy(output, buffer->data, output_len);
    output[output_len] = '\0';

    return parser_ok;
}
