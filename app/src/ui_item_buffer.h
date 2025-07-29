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
parser_error_t init_item_buffer(item_buffer_t *buffer, const char *initial_data, const char *separator_open,
                                const char *separator_close);
parser_error_t get_item_buffer_qty(const item_buffer_t *buffer, uint8_t *qty);
parser_error_t append_item_buffer(item_buffer_t *buffer, const char *input, uint16_t input_len);
parser_error_t clear_item_buffer(item_buffer_t *buffer);
parser_error_t is_item_buffer_empty(item_buffer_t *buffer, bool *is_empty);
parser_error_t remove_last_item_buffer(item_buffer_t *buffer);
parser_error_t get_item_buffer_content(item_buffer_t *buffer, uint8_t index, char *content, uint16_t content_len);
parser_error_t get_item_buffer_content_length(item_buffer_t *buffer, uint8_t index, size_t *length);
parser_error_t get_item_buffer_range_length(item_buffer_t *buffer, uint8_t index_start, uint8_t index_end,
                                            size_t *total_length);
parser_error_t get_item_buffer_range(item_buffer_t *buffer, uint8_t index_start, uint8_t index_end,
                                     item_buffer_t *output);
parser_error_t remove_first_separator(item_buffer_t *buffer, char *output, uint16_t output_len);
parser_error_t remove_last_separator(item_buffer_t *buffer, char *output, uint16_t output_len);