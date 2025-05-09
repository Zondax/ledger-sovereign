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
item_buffer_t item_data_buffer = {0};

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

parser_error_t remove_last_item_title() {
    CHECK_ERROR(remove_last_item_buffer(&item_title_buffer));

    print_string("Removed last variant\n");
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

    print_u32("Item data buffer qty: ", item_data_buffer.qty);

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

parser_error_t push_item(parser_tx_t *txObj) {
    CHECK_INPUT(txObj);

    if (txObj->ui_items.qty >= MAX_ITEMS) {
        return parser_too_many_items;
    }

    uint16_t title_len = strlen(item_title_buffer.data);
    if (title_len > MAX_STRING_LENGTH) {
        return parser_push_item_too_long;
    }

    uint16_t data_len = strlen(item_data_buffer.data);
    if (data_len > MAX_STRING_LENGTH) {
        return parser_push_item_too_long;
    }
    if (data_len == 0) {
        return parser_ui_item_data_empty;
    }

    char content[100] = {0};
    for (uint16_t i = 0; i < item_data_buffer.qty; i++) {
        MEMZERO(content, sizeof(content));
        CHECK_ERROR(get_item_data(content, sizeof(content)));
        if (item_data_buffer.qty == 1) {
            MEMCPY(txObj->ui_items.items[txObj->ui_items.qty].title, item_title_buffer.data, strlen(item_title_buffer.data));
            MEMCPY(txObj->ui_items.items[txObj->ui_items.qty].data, content, strlen(content));
        } else {
            char index_str[12];
            snprintf(index_str, sizeof(index_str), "%d", i);
            CHECK_ERROR(append_item_title(index_str, strlen(index_str)));
            MEMCPY(txObj->ui_items.items[txObj->ui_items.qty].title, item_title_buffer.data, strlen(item_title_buffer.data));
            MEMCPY(txObj->ui_items.items[txObj->ui_items.qty].data, content, strlen(content));
            CHECK_ERROR(remove_last_item_title());
        }
        txObj->ui_items.qty++;
    }

    // after pushing all items, clear the item_data_buffer
    clear_item_data_buffer();

    for (uint16_t i = 0; i < txObj->ui_items.qty; i++) {
        print_string("Push item: ");
        print_string(txObj->ui_items.items[i].title);
        print_string(txObj->ui_items.items[i].data);
    }

    return parser_ok;
}

parser_error_t test_remove_last_item_buffer() {
    item_buffer_t buffer = {0};
    item_buffer_t buffer2 = {0};
    init_item_buffer(&buffer2, NULL, "/", "/");

    char content[100] = {0};
    init_item_buffer(&buffer, "|hello|world|test|test2|", "|", "|");
    print_string("Initial buffer");
    print_string(buffer.data);
    CHECK_ERROR(get_item_buffer_content(&buffer, 0, content, sizeof(content)));
    print_string("Content: ");
    print_string(content);
    CHECK_ERROR(get_item_buffer_content(&buffer, 1, content, sizeof(content)));
    print_string("Content: ");
    print_string(content);
    CHECK_ERROR(get_item_buffer_content(&buffer, 2, content, sizeof(content)));
    print_string("Content: ");
    print_string(content);
    if (get_item_buffer_content(&buffer, 4, content, sizeof(content)) != parser_ok) {
        print_string("Error: ");
        print_string(content);
    }

    CHECK_ERROR(get_item_buffer_range(&buffer, 0, 1, &buffer2));
    print_string("Range buffer 0 1");
    print_string(buffer2.data);

    CHECK_ERROR(get_item_buffer_range(&buffer, 1, 3, &buffer2));
    print_string("Range buffer 1 3");
    print_string(buffer2.data);

    if (get_item_buffer_range(&buffer, 0, 0, &buffer2) != parser_ok) {
        print_string("Error get_item_buffer_range 0 0: ");
    }
    print_string("Range buffer 0 0");
    print_string(buffer2.data);

    CHECK_ERROR(remove_last_item_buffer(&buffer));
    print_string("Removed last variant");
    print_string(buffer.data);
    CHECK_ERROR(remove_last_item_buffer(&buffer));
    print_string("Removed last variant");
    print_string(buffer.data);
    CHECK_ERROR(remove_last_item_buffer(&buffer));
    print_string("Removed last variant");
    print_string(buffer.data);
    CHECK_ERROR(remove_last_item_buffer(&buffer));
    print_string("Removed last variant");
    print_string(buffer.data);

    CHECK_ERROR(append_item_buffer(&buffer, "1", 1));
    CHECK_ERROR(append_item_buffer(&buffer, "2", 1));
    CHECK_ERROR(append_item_buffer(&buffer, "3", 1));
    CHECK_ERROR(append_item_buffer(&buffer, "4", 1));
    CHECK_ERROR(append_item_buffer(&buffer, "5", 1));
    print_string("Appended 5 items");
    print_string(buffer.data);
    CHECK_ERROR(remove_last_item_buffer(&buffer));
    print_string("Removed last variant");
    print_string(buffer.data);

    char output[100] = {0};
    CHECK_ERROR(remove_last_separator(&buffer, output, sizeof(output)));
    print_string("Removed last separator");
    print_string(output);

    CHECK_ERROR(remove_first_separator(&buffer, output, sizeof(output)));
    print_string("Removed first separator");
    print_string(output);

    return parser_ok;
}
