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
#include "ui_utils.h"
#include "zxerror.h"
#include "zxformat.h"
#include "zxmacros.h"

item_buffer_t item_title_buffer = {0};
item_buffer_t item_title_buffer_new = {0};
item_buffer_t item_data_buffer = {0};
primitive_t primitive;
parser_context_t data_context;
static bool data_context_full = false;
static bool enable_push_item = false;

// Item title buffer
void init_item_title_buffer(const char *initial_data) {
    init_item_buffer(&item_title_buffer, initial_data, SEPARATOR_TITLE_OPEN, SEPARATOR_TITLE_CLOSE);
}

void init_item_title_buffer_new(const char *initial_data) {
    init_item_buffer(&item_title_buffer_new, initial_data, SEPARATOR_TITLE_OPEN, SEPARATOR_TITLE_CLOSE);
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

parser_error_t append_item_title_new(const char *buffer, uint16_t input_len) {
    if (!item_title_buffer_new.initialized) {
        init_item_title_buffer_new(NULL);
    }
    CHECK_ERROR(append_item_buffer(&item_title_buffer_new, buffer, input_len));

    print_string("Appended variant name\n");
    print_string(item_title_buffer_new.data);

    return parser_ok;
}

parser_error_t remove_last_item_title() {
    CHECK_ERROR(remove_last_item_buffer(&item_title_buffer));

    print_string("Removed last variant\n");
    print_string(item_title_buffer.data);

    return parser_ok;
}

parser_error_t remove_last_item_title_new() {
    CHECK_ERROR(remove_last_item_buffer(&item_title_buffer_new));

    print_string("Removed last variant\n");
    print_string(item_title_buffer_new.data);

    return parser_ok;
}

parser_error_t get_title_item_qty(uint8_t *qty) {
    CHECK_INPUT(qty);
    return get_item_buffer_qty(&item_title_buffer, qty);
}

parser_error_t get_title_item_qty_new(uint8_t *qty) {
    CHECK_INPUT(qty);
    return get_item_buffer_qty(&item_title_buffer_new, qty);
}

void clear_item_title_buffer() { clear_item_buffer(&item_title_buffer); }

void clear_item_title_buffer_new() { clear_item_buffer(&item_title_buffer_new); }

bool is_item_title_empty() {
    bool is_empty = false;
    is_item_buffer_empty(&item_title_buffer, &is_empty);
    return is_empty;
}

bool is_item_title_empty_new() {
    bool is_empty = false;
    is_item_buffer_empty(&item_title_buffer_new, &is_empty);
    return is_empty;
}

parser_error_t get_item_title(char *item_title, uint16_t item_title_len) {
    CHECK_INPUT(item_title);
    CHECK_ERROR(get_item_buffer_content(&item_title_buffer, 0, item_title, item_title_len));
    return parser_ok;
}

parser_error_t get_item_title_new(char *item_title, uint16_t item_title_len) {
    CHECK_INPUT(item_title);
    CHECK_ERROR(get_item_buffer_content(&item_title_buffer_new, 0, item_title, item_title_len));
    return parser_ok;
}

parser_error_t create_item_title(uint16_t index_start, uint16_t index_end, char *output, uint16_t output_len) {
    CHECK_INPUT(output);

    MEMZERO(output, output_len);

    // TODO: remove this
    char content[100] = {0};

    if (index_start >= index_end) {
        return parser_ui_item_title_empty;
    }

    for (uint8_t i = index_start; i < index_end; i++) {
        CHECK_ERROR(get_item_buffer_content(&item_title_buffer, i, content, sizeof(content)));
        strncat(output, content, strlen(content));
        if (i < index_end - 1) {
            strncat(output, SEPARATOR_TITLE_DISPLAY, strlen(SEPARATOR_TITLE_DISPLAY));
        }
    }

    print_string("Create item title: ");
    print_string(output);

    return parser_ok;
}

parser_error_t create_item_title_new(uint16_t index_start, uint16_t index_end, char *output, uint16_t output_len) {
    CHECK_INPUT(output);

    MEMZERO(output, output_len);

    // TODO: remove this
    char content[100] = {0};

    if (index_start >= index_end) {
        return parser_ui_item_title_empty;
    }

    for (uint8_t i = index_start; i < index_end; i++) {
        CHECK_ERROR(get_item_buffer_content(&item_title_buffer_new, i, content, sizeof(content)));
        strncat(output, content, strlen(content));
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

parser_error_t get_item_title_range_length_new(uint16_t index_start, uint16_t index_end, size_t *total_length) {
    CHECK_ERROR(get_item_buffer_range_length(&item_title_buffer_new, index_start, index_end, total_length));
    return parser_ok;
}

// Item data buffer
void init_item_data_buffer() { init_item_buffer(&item_data_buffer, NULL, SEPARATOR_DATA_OPEN, SEPARATOR_DATA_CLOSE); }

parser_error_t append_item_data(const char *input, uint16_t input_len) {
    if (!item_data_buffer.initialized) {
        init_item_data_buffer();
    }
    CHECK_ERROR(append_item_buffer(&item_data_buffer, input, input_len));

    item_data_buffer.qty++;

    print_string("Appended variant data\n");
    print_string(item_data_buffer.data);

    return parser_ok;
}

void clear_item_data_buffer() { clear_item_buffer(&item_data_buffer); }

bool is_item_data_empty() {
    bool is_empty = false;
    is_item_buffer_empty(&item_data_buffer, &is_empty);
    return is_empty;
}

parser_error_t get_item_data(char *item_data, uint16_t item_data_len) {
    CHECK_INPUT(item_data);
    CHECK_ERROR(get_item_buffer_content(&item_data_buffer, 0, item_data, item_data_len));
    return parser_ok;
}

parser_error_t set_primitive(primitive_t *value) {
    CHECK_INPUT(value);
    MEMZERO(&primitive, sizeof(primitive));
    primitive = *value;
    return parser_ok;
}

parser_error_t set_data_context(parser_context_t *context) {
    CHECK_INPUT(context);
    MEMZERO(&data_context, sizeof(data_context));
    MEMCPY(&data_context, context, sizeof(data_context));
    data_context_full = true;
    return parser_ok;
}

bool is_data_context_empty() { return !data_context_full; }

void set_enable_push_item(bool value) { enable_push_item = value; }

parser_error_t push_item_new(parser_tx_t *txObj) {
    CHECK_INPUT(txObj);

    if (!enable_push_item) {
        data_context_full = false;
        return parser_ok;
    }

    if (txObj->ui_items_new.qty >= MAX_ITEMS) {
        return parser_too_many_items;
    }

    uint16_t title_len = strlen(item_title_buffer_new.data);
    if (title_len > MAX_STRING_LENGTH) {
        return parser_push_item_too_long;
    }

    MEMCPY(txObj->ui_items_new.items[txObj->ui_items_new.qty].title, item_title_buffer_new.data,
           strlen(item_title_buffer_new.data));
    txObj->ui_items_new.items[txObj->ui_items_new.qty].primitive = primitive;
    txObj->ui_items_new.items[txObj->ui_items_new.qty].data_context = data_context;
    txObj->ui_items_new.qty++;

    for (uint16_t i = 0; i < txObj->ui_items_new.qty; i++) {
        print_string("Push item NEW: ");
        print_string(txObj->ui_items_new.items[i].title);
        print_buffer(&txObj->ui_items_new.items[i].data_context.buffer, "data context");
    }

    data_context_full = false;

    return parser_ok;
}
