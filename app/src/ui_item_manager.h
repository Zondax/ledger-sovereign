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

// Item title
void init_item_title_buffer(const char *initial_data);
parser_error_t append_item_title(const char *input, uint16_t input_len);
parser_error_t remove_last_item_title();
parser_error_t get_title_item_qty(uint8_t *qty);
void clear_item_title_buffer();
bool is_item_title_empty();
parser_error_t get_item_title(char *item_title, uint16_t item_title_len);
parser_error_t create_item_title(uint16_t index_start, uint16_t index_end, char *output, uint16_t output_len);
parser_error_t get_item_title_range_length(uint16_t index_start, uint16_t index_end, size_t *total_length);

// Item title new
void init_item_title_buffer_new(const char *initial_data);
parser_error_t append_item_title_new(const char *input, uint16_t input_len);
parser_error_t remove_last_item_title_new();
parser_error_t get_title_item_qty_new(uint8_t *qty);
bool is_item_title_empty_new();
parser_error_t get_item_title_new(char *item_title, uint16_t item_title_len);
parser_error_t create_item_title_new(uint16_t index_start, uint16_t index_end, char *output, uint16_t output_len);
parser_error_t get_item_title_range_length_new(uint16_t index_start, uint16_t index_end, size_t *total_length);

// Item data
void init_item_data_buffer();
parser_error_t append_item_data(const char *input, uint16_t input_len);
void clear_item_data_buffer();
bool is_item_data_empty();
parser_error_t get_item_data(char *item_data, uint16_t item_data_len);
bool is_data_context_empty();

parser_error_t set_primitive(primitive_t *value);
parser_error_t set_data_context(parser_context_t *context);

void set_enable_push_item(bool value);
parser_error_t push_item_new(parser_tx_t *txObj);
