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

#define MAX_FIELDS_QTY 20
#define MAX_VARIANTS_QTY 20
#define MAX_HRP_LEN 83
#define MAX_SCHEMES_QTY 200

typedef enum {
    ROLLUP_ROOTS_TRANSACTION = 0,
    ROLLUP_ROOTS_UNSIGNED_TRANSACTION,
    ROLLUP_ROOTS_RUNTIME_CALL,
    ROLLUP_ROOTS_ADDRESS,
} rollup_roots_e;

typedef enum {
    LINKING_SCHEME_ENUM = 0,
    LINKING_SCHEME_STRUCT,
    LINKING_SCHEME_TUPLE,
    LINKING_SCHEME_OPTION,
    LINKING_SCHEME_INTEGER,
    LINKING_SCHEME_BYTE_ARRAY,
    LINKING_SCHEME_FLOAT32,
    LINKING_SCHEME_FLOAT64,
    LINKING_SCHEME_STRING,
    LINKING_SCHEME_BOOLEAN,
    LINKING_SCHEME_SKIP,
    LINKING_SCHEME_BYTE_VEC,
    LINKING_SCHEME_ARRAY,
    LINKING_SCHEME_VEC,
    LINKING_SCHEME_MAP,
} linking_scheme_e;

typedef enum {
    PRIMITIVE_INTEGER = 0,
    PRIMITIVE_BYTE_ARRAY,
    PRIMITIVE_BYTE_VEC,
    PRIMITIVE_FLOAT32,
    PRIMITIVE_FLOAT64,
    PRIMITIVE_STRING,
    PRIMITIVE_BOOLEAN,
    PRIMITIVE_SKIP
} primitive_e;

typedef enum {
    BYTE_DISPLAY_HEX = 0,
    BYTE_DISPLAY_DECIMAL,
    BYTE_DISPLAY_BECH32,
    BYTE_DISPLAY_BECH32M,
    BYTE_DISPLAY_BASE58
} byte_display_e;
typedef enum {
    INTEGER_DISPLAY_HEX = 0,
    INTEGER_DISPLAY_DECIMAL,
    INTEGER_DISPLAY_FIXED_POINT,
} integer_display_e;

typedef enum {
    INTEGER_I8 = 0,
    INTEGER_I16,
    INTEGER_I32,
    INTEGER_I64,
    INTEGER_I128,
    INTEGER_U8,
    INTEGER_U16,
    INTEGER_U32,
    INTEGER_U64,
    INTEGER_U128,
} integer_e;

typedef enum { FIXED_POINT_DISPLAY_DECIMALS = 0, FIXED_POINT_DISPLAY_FROM_SIBLING_FIELD } fixed_point_display_e;

typedef enum { LINK_BY_INDEX = 0, LINK_IMMEDIATE, LINK_PLACEHOLDER, LINK_INDEXED_PLACEHOLDER } link_e;

typedef struct {
    uint64_t field_index;
    uint64_t byte_offset;
} fixed_point_display_from_sibling_field_t;

typedef struct {
    fixed_point_display_e type;
    union {
        uint8_t decimals;
        fixed_point_display_from_sibling_field_t from_sibling_field;
    };
} fixed_point_display_t;

typedef struct {
    integer_display_e type;
    union {
        fixed_point_display_t fixed_point;
    };
} integer_display_t;

typedef struct {
    bytes_t prefix;
} hrp_t;

typedef struct {
    hrp_t prefix;
} byte_display_bech32_t;

typedef struct {
    hrp_t prefix;
} byte_display_bech32m_t;

typedef struct {
    bool has_title_elements;
    uint64_t title_elements;
    bool has_title_lines;
    uint64_t title_lines;
} structured_display_overrides_t;

typedef struct {
    byte_display_e type;
    union {
        byte_display_bech32_t bech32;
        byte_display_bech32m_t bech32m;
    };
} byte_display_t;

typedef struct {
    integer_e type;
    integer_display_t display;
} primitive_integer_t;

typedef struct {
    uint64_t len;
    byte_display_t display;
} primitive_byte_array_t;

typedef struct {
    byte_display_t display;
} primitive_byte_vec_t;

typedef struct {
    uint64_t len;
} primitive_skip_t;

typedef struct {
    primitive_e type;
    union {
        primitive_integer_t integer;
        primitive_byte_array_t byte_array;
        primitive_byte_vec_t byte_vec;
        primitive_skip_t skip;
    };
} primitive_t;

typedef struct {
    link_e tag;
    union {
        uint64_t by_index;
        primitive_t immediate;
        size_t indexed_placeholder;
    } data;
} link_t;

typedef struct {
    bytes_t display_name;
    bool silent;
    bool is_expert;
    link_t value;
    bytes_t doc;
} named_field_t;

typedef struct {
    link_t value;
    bool silent;
    bool is_expert;
    bytes_t doc;
} unnamed_field_t;

typedef struct {
    bytes_t name;
    uint8_t discriminant;
    bool hide_tag;
    bool has_value;
    link_t value;
} enum_variant_t;

typedef struct {
    bytes_t type_name;
    uint32_t variants_qty;
    enum_variant_t variants[MAX_VARIANTS_QTY];
    bool hide_tag;
    structured_display_overrides_t structured_display_overrides;
} schema_enum_t;

typedef struct {
    bytes_t type_name;
    bool has_show_as;
    bytes_t show_as;
    bool has_structured_show_as;
    bytes_t structured_show_as;
    bool peekable;
    uint32_t fields_qty;
    named_field_t fields[MAX_FIELDS_QTY];
    structured_display_overrides_t structured_display_overrides;
} schema_struct_t;

typedef struct {
    bool has_show_as;
    bytes_t show_as;
    bool has_structured_show_as;
    bytes_t structured_show_as;
    bool peekable;
    uint32_t fields_qty;
    unnamed_field_t fields[MAX_FIELDS_QTY];
    structured_display_overrides_t structured_display_overrides;
} schema_tuple_t;

typedef struct {
    bool has_show_as;
    bytes_t show_as;
    bool has_structured_show_as;
    bytes_t structured_show_as;
    bool peekable;
    uint32_t fields_qty;
    unnamed_field_t fields[MAX_FIELDS_QTY];
} schema_option_t;

typedef struct {
    uint64_t len;
    link_t value;
} schema_array_t;

typedef struct {
    link_t key;
    link_t value;
} schema_map_t;

// this struct is used to store where the schema starts in the input buffer
typedef struct {
    linking_scheme_e type;
    parser_context_t data;
} linking_scheme_t;

typedef struct {
    uint32_t qty;
    linking_scheme_t schemes[MAX_SCHEMES_QTY];
} types_t;

typedef struct {
    uint32_t qty;
    uint64_t indices[MAX_SCHEMES_QTY];
} root_type_indices_t;

typedef struct {
    uint64_t chain_id;
    bytes_t chain_name;
    uint8_t gas_token_decimals;
} chain_data_t;

typedef struct {
    types_t types;
    root_type_indices_t root_type_indices;
    chain_data_t chain_data;
    bytes_t extra_metadata_hash;
    bytes_t chain_hash;
    uint64_t call_message_index[MAX_RUNTIME_CALL_TYPE];
} schema_t;

#ifdef __cplusplus
}
#endif
