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

#ifdef __cplusplus
extern "C" {
#endif

#include "common_txdef.h"

#define MAX_ITEMS 30
#define MAX_STRING_LENGTH 100
#define U128_STR_MAX_LEN 40
#define MAX_SEPARATOR_LENGTH 100
#define SEPARATOR_TITLE_OPEN "|"
#define SEPARATOR_TITLE_CLOSE "|"
#define SEPARATOR_DATA_OPEN "{"
#define SEPARATOR_DATA_CLOSE "}"
#define SEPARATOR_TITLE_DISPLAY "/"
#define PAGE_BREAK ">"

// Buffer to store item data for array or vector display
typedef struct {
    bool initialized;
    uint8_t qty;
    char separator_open[MAX_SEPARATOR_LENGTH];
    uint8_t separator_open_len;
    char separator_close[MAX_SEPARATOR_LENGTH];
    uint8_t separator_close_len;
    char data[MAX_STRING_LENGTH];
} item_buffer_t;

typedef struct {
    char title[MAX_STRING_LENGTH];
    char data[MAX_STRING_LENGTH];
} ui_item_t;
typedef struct {
    uint16_t qty;
    ui_item_t items[MAX_ITEMS];
} ui_items_t;

#ifdef __cplusplus
}
#endif
