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

#include "ui_item_manager.h"

#include <stdio.h>

#include "borsh.h"
#include "parser_common.h"
#include "ui_item_buffer.h"
#include "zxerror.h"
#include "zxformat.h"
#include "zxmacros.h"

#define MAX_U16_STR_LEN 6

item_buffer_t item_title_buffer = {0};
static bool enable_push_item = false;

// Item title buffer
void init_item_title_buffer(const char *initial_data) {
    init_item_buffer(&item_title_buffer, initial_data, SEPARATOR_TITLE_OPEN, SEPARATOR_TITLE_CLOSE);
}

parser_error_t append_item_title(const char *buffer, uint16_t input_len) {
    if (!item_title_buffer.initialized) {
        init_item_title_buffer(NULL);
    }
    CHECK_ERROR(append_item_buffer(&item_title_buffer, buffer, input_len));

    print_string("Appended variant name\n");
    print_string(item_title_buffer.data);

    return parser_ok;
}

parser_error_t append_item_title_index(uint16_t index) {
    char index_str[MAX_U16_STR_LEN] = {0};
    snprintf(index_str, sizeof(index_str), "%u", index);
    return append_item_title(index_str, strlen(index_str));
}

parser_error_t remove_last_item_title() {
    CHECK_ERROR(remove_last_item_buffer(&item_title_buffer));

    print_string("Removed last variant");
    print_string(item_title_buffer.data);

    return parser_ok;
}

parser_error_t get_title_item_qty(uint8_t *qty) {
    CHECK_INPUT(qty);
    return get_item_buffer_qty(&item_title_buffer, qty);
}

void clear_item_title_buffer() { clear_item_buffer(&item_title_buffer); }

bool is_item_title_empty() {
    bool is_empty = false;
    is_item_buffer_empty(&item_title_buffer, &is_empty);

    print_string("is_item_title_empty");
    print_string(item_title_buffer.data);

    return is_empty;
}

parser_error_t get_item_title(char *item_title, uint16_t item_title_len) {
    CHECK_INPUT(item_title);
    CHECK_ERROR(get_item_buffer_content(&item_title_buffer, 0, item_title, item_title_len));
    return parser_ok;
}

parser_error_t create_item_title(uint16_t index_start, uint16_t index_end, char *output, uint16_t output_len) {
    CHECK_INPUT(output);

    MEMZERO(output, output_len);

    if (index_start >= index_end) {
        return parser_ui_item_title_empty;
    }

    for (uint8_t i = index_start; i < index_end; i++) {
        uint16_t output_str_len = strlen(output);
        CHECK_ERROR(get_item_buffer_content(&item_title_buffer, i, output + output_str_len, output_len - output_str_len));
        if (i < index_end - 1) {
            strncat(output, SEPARATOR_TITLE_DISPLAY, strlen(SEPARATOR_TITLE_DISPLAY));
        }
    }

    print_string("Create item title: ");
    print_string(output);

    return parser_ok;
}

parser_error_t get_item_title_range_length(uint16_t index_start, uint16_t index_end, size_t *total_length) {
    CHECK_ERROR(get_item_buffer_range_length(&item_title_buffer, index_start, index_end, total_length));
    return parser_ok;
}

void set_enable_push_item(bool value) { enable_push_item = value; }

bool is_enable_push_item() { return enable_push_item; }

parser_error_t push_item_string(parser_tx_t *txObj, const char *input, uint16_t input_len) {
    parser_context_t ctx_bytes = {0};
    ctx_bytes.buffer.ptr = (uint8_t *)input;
    ctx_bytes.buffer.len = input_len;
    primitive_t primitive = {0};
    primitive.type = PRIMITIVE_STRING;
    return push_item(txObj, &primitive, &ctx_bytes);
}

parser_error_t push_item(parser_tx_t *txObj, primitive_t *primitive, parser_context_t *data_context) {
    CHECK_INPUT(txObj);
    CHECK_INPUT(primitive);
    CHECK_INPUT(data_context);

    if (!enable_push_item) {
        return parser_ok;
    }

    if (txObj->ui_items_new.qty >= MAX_ITEMS) {
        return parser_too_many_items;
    }

    uint16_t title_len = strlen(item_title_buffer.data);
    if (title_len > MAX_STRING_LENGTH) {
        return parser_push_item_too_long;
    }

    MEMCPY(txObj->ui_items_new.items[txObj->ui_items_new.qty].title, item_title_buffer.data, strlen(item_title_buffer.data));
    txObj->ui_items_new.items[txObj->ui_items_new.qty].primitive = *primitive;
    txObj->ui_items_new.items[txObj->ui_items_new.qty].data_context = *data_context;
    txObj->ui_items_new.qty++;

    for (uint16_t i = 0; i < txObj->ui_items_new.qty; i++) {
        print_string("Push item NEW: ");
        print_string(txObj->ui_items_new.items[i].title);
        print_buffer(&txObj->ui_items_new.items[i].data_context.buffer, "data context");
    }

    return parser_ok;
}
