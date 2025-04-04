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

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
typedef struct {
    const uint8_t *ptr;
    uint16_t len;
} bytes_t;

typedef struct {
    uint32_t qty;
} types_t;

typedef struct {
    uint8_t* display_name;
    bool silent;
    uint8_t* value;
    uint8_t* doc;
} named_field_t;

typedef struct {
    uint8_t* type_name;
    bool has_show_as;
    uint8_t* show_as;
    bool has_structured_show_as;
    uint8_t* structured_show_as;
    bool peekable;
    named_field_t* fields;
} schema_struct_t;

// Schema structure
typedef struct {
    types_t types;
    bytes_t root_type_indices;
    bytes_t chain_data;
    bytes_t extra_metadata_hash;
    bytes_t chain_hash;
} schema_t;

typedef struct {
    schema_t schema;
} parser_tx_t;

#ifdef __cplusplus
}
#endif
