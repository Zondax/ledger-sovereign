/*******************************************************************************
 *   (c) 2018 - 2025 Zondax AG
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

#include "schema_display.h"

#include "app_mode.h"
#include "bech32.h"
#include "borsh.h"
#include "schema_helper.h"
#include "schema_reader.h"
#include "ui_item_manager.h"
#include "ui_utils.h"
#include "zxerror.h"
#include "zxformat.h"

bool ui_expert_mode = false;

#define NONE_STRING "None"

parser_error_t append_structured_show_as_title(const bytes_t *structured_show_as, uint32_t field_index, bool *remove_title) {
    CHECK_INPUT(structured_show_as);
    CHECK_INPUT(remove_title);

    *remove_title = false;
    char title[100] = {0};
    CHECK_ERROR(find_bracket_content((char *)structured_show_as->ptr, field_index, title, sizeof(title)));
    uint16_t len = strlen(title);
    if (strlen(title) > 0) {
        CHECK_ERROR(append_item_title(title, len));
        *remove_title = true;
    }
    return parser_ok;
}

parser_error_t schema_display_integer(parser_context_t *ctx, primitive_integer_t *primitive, parser_context_t *ctx_to_push) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(primitive);
    CHECK_INPUT(ctx_to_push);

    uint16_t len_to_push = 0;
    const uint8_t *ctx_mem = ctx->buffer.ptr + ctx->offset;
    switch (primitive->type) {
        case INTEGER_I8:
        case INTEGER_U8: {
            len_to_push = OFFSET_U8;
            CTX_CHECK_AND_ADVANCE(ctx, len_to_push);
            break;
        }
        case INTEGER_I16:
        case INTEGER_U16: {
            len_to_push = OFFSET_U16;
            CTX_CHECK_AND_ADVANCE(ctx, len_to_push);
            break;
        }
        case INTEGER_I32:
        case INTEGER_U32: {
            len_to_push = OFFSET_U32;
            CTX_CHECK_AND_ADVANCE(ctx, len_to_push);
            break;
        }
        case INTEGER_I64:
        case INTEGER_U64: {
            len_to_push = OFFSET_U64;
            CTX_CHECK_AND_ADVANCE(ctx, len_to_push);
            break;
        }
        case INTEGER_I128:
        case INTEGER_U128: {
            len_to_push = OFFSET_U64 * 2;
            CTX_CHECK_AND_ADVANCE(ctx, len_to_push);
            break;
        }
        default:
            return parser_unexpected_type;
    }

    switch (primitive->display.type) {
        case INTEGER_DISPLAY_HEX:
            // TODO: Implement me
            print_string("schema_display_integer IMPLEMENT ME 0");
            return parser_unexpected_type;
        case INTEGER_DISPLAY_DECIMAL:
            print_string("schema_display_integer len += 0");
            break;
        case INTEGER_DISPLAY_FIXED_POINT:
            switch (primitive->display.fixed_point.type) {
                case FIXED_POINT_DISPLAY_DECIMALS:
                    // TODO: Implement me
                    print_string("render_fixed_point IMPLEMENT ME 0");
                    return parser_unexpected_type;
                case FIXED_POINT_DISPLAY_FROM_SIBLING_FIELD:
                    // get offset
                    if (ctx->offset + primitive->display.fixed_point.from_sibling_field.byte_offset >= ctx->buffer.len) {
                        return parser_unexpected_buffer_end;
                    }
                    len_to_push += primitive->display.fixed_point.from_sibling_field.byte_offset + 1;
                    break;
                default:
                    return parser_unexpected_type;
            }
            break;
        default:
            return parser_unexpected_type;
    }
    ctx_to_push->buffer.ptr = ctx_mem;
    ctx_to_push->buffer.len = len_to_push;

    return parser_ok;
}

parser_error_t schema_display_byte_array(parser_context_t *ctx, primitive_byte_array_t *byte_array,
                                         parser_context_t *ctx_to_push) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(byte_array);
    CHECK_INPUT(ctx_to_push);

    const uint8_t *ctx_mem = ctx->buffer.ptr + ctx->offset;
    uint16_t len_to_push = byte_array->len;
    CTX_CHECK_AND_ADVANCE(ctx, len_to_push);

    if (byte_array->len > MAX_INPUT_CHUNK) {
        return parser_ui_buffer_too_small;
    }

    ctx_to_push->buffer.ptr = ctx_mem;
    ctx_to_push->buffer.len = len_to_push;

    return parser_ok;
}

parser_error_t schema_display_primitive(parser_context_t *ctx, parser_tx_t *txObj, primitive_t *primitive) {
    CHECK_INPUT(ctx);

    uint16_t primitive_size = sizeof(primitive);
    print_u16("Primitive size: ", primitive_size);

    parser_context_t ctx_to_push = {0};
    switch (primitive->type) {
        case PRIMITIVE_INTEGER:
            primitive_size = sizeof(primitive->integer);
            print_u16("Primitive size integer_0: ", primitive_size);
            CHECK_ERROR(schema_display_integer(ctx, &primitive->integer, &ctx_to_push));
            break;
        case PRIMITIVE_BYTE_ARRAY:
            primitive_size = sizeof(primitive->byte_array);
            print_u16("Primitive size byte_array: ", primitive_size);
            CHECK_ERROR(schema_display_byte_array(ctx, &primitive->byte_array, &ctx_to_push));
            break;
        case PRIMITIVE_BYTE_VEC:
            // TODO: Implement me
            print_string("schema_display_primitive IMPLEMENT ME 0");
            return parser_unexpected_type;
        case PRIMITIVE_FLOAT32:
            // TODO: Implement me
            print_string("schema_display_primitive IMPLEMENT ME 1");
            return parser_unexpected_type;
        case PRIMITIVE_FLOAT64:
            // TODO: Implement me
            print_string("schema_display_primitive IMPLEMENT ME 2");
            return parser_unexpected_type;
        case PRIMITIVE_STRING:
            // TODO: Implement me
            print_string("schema_display_primitive IMPLEMENT ME 3");
            return parser_unexpected_type;
        case PRIMITIVE_BOOLEAN:
            // TODO: Implement me
            print_string("schema_display_primitive IMPLEMENT ME 4");
            return parser_unexpected_type;
        default:
            print_u8("Unknown type: ", primitive->type);
            return parser_unexpected_type;
    }

    if (is_enable_push_item()) {
        CHECK_ERROR(push_item(txObj, primitive, &ctx_to_push));
    }
    return parser_ok;
}

parser_error_t schema_display_enum(parser_context_t *ctx, parser_tx_t *txObj) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(txObj);

    schema_enum_t enum_type = {0};
    CHECK_ERROR(read_enum(&txObj->merkle_proofs.leaves.data, &enum_type));
    CHECK_ERROR(schema_reset_leaf_offset(&txObj->merkle_proofs.leaves));

    uint8_t discriminant = 0;
    CHECK_ERROR(read_u8(ctx, &discriminant));

    if (discriminant >= enum_type.variants_qty) {
        return parser_scheme_discriminant_overflow;
    }

    enum_variant_t variant = {0};
    for (uint32_t i = 0; i <= discriminant; i++) {
        MEMZERO(&variant, sizeof(enum_variant_t));
        CHECK_ERROR(read_enum_variant(&enum_type.enum_variants, &variant));
    }

    if (variant.has_value && variant.value.tag == LINK_BY_INDEX) {
        bool remove_variant = false;
        if (!variant.hide_tag && !enum_type.hide_tag) {
            CHECK_ERROR(append_item_title((char *)variant.name.ptr, variant.name.len));
            remove_variant = true;
        }
        CHECK_ERROR(schema_display_generic_by_index(ctx, txObj, variant.value.data.by_index));
        if (remove_variant) {
            CHECK_ERROR(remove_last_item_title());
        }
    } else {
        parser_context_t ctx_bytes = {0};
        ctx_bytes.buffer = variant.name;
        primitive_t primitive = {0};
        primitive.type = PRIMITIVE_STRING;
        if (is_enable_push_item()) {
            CHECK_ERROR(push_item_string(txObj, variant.name.ptr, variant.name.len));
        }
    }

    return parser_ok;
}

parser_error_t schema_display_struct(parser_context_t *ctx, parser_tx_t *txObj) {
    CHECK_INPUT(txObj);

    schema_struct_t struct_type = {0};
    CHECK_ERROR(read_struct(&txObj->merkle_proofs.leaves.data, &struct_type));
    CHECK_ERROR(schema_reset_leaf_offset(&txObj->merkle_proofs.leaves));

    named_field_t named_field = {0};

    for (uint32_t i = 0; i < struct_type.fields_qty; i++) {
        MEMZERO(&named_field, sizeof(named_field_t));
        CHECK_ERROR(read_named_field(&struct_type.named_fields, &named_field));

        bool show_field = should_show_field(&named_field.value, named_field.silent, named_field.is_expert, ui_expert_mode);

        bool remove_title = false;
        if (show_field) {
            if (struct_type.has_show_as || struct_type.has_structured_show_as) {
                CHECK_ERROR(append_structured_show_as_title(&struct_type.structured_show_as, i, &remove_title));
            } else {
                CHECK_ERROR(append_item_title((char *)named_field.display_name.ptr, named_field.display_name.len));
                remove_title = true;
            }
        }

        set_enable_push_item(show_field);
        // in the case that father field is not shown, we don't need to show the child field, even if it's enabled
        if (show_field && is_item_title_empty()) {
            set_enable_push_item(false);
        }

        switch (named_field.value.tag) {
            case LINK_BY_INDEX:
                CHECK_ERROR(schema_display_generic_by_index(ctx, txObj, named_field.value.data.by_index));
                break;
            case LINK_IMMEDIATE:
                CHECK_ERROR(schema_display_primitive(ctx, txObj, &named_field.value.data.immediate));
                break;
            default:
                print_string("schema_display_struct IMPLEMENT ME 1");
                return parser_unexpected_type;
        }

        if (remove_title) {
            CHECK_ERROR(remove_last_item_title());
        }
    }

    return parser_ok;
}

parser_error_t schema_display_tuple(parser_context_t *ctx, parser_tx_t *txObj) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(txObj);

    schema_tuple_t tuple_type = {0};
    CHECK_ERROR(read_tuple(&txObj->merkle_proofs.leaves.data, &tuple_type));
    CHECK_ERROR(schema_reset_leaf_offset(&txObj->merkle_proofs.leaves));

    unnamed_field_t unnamed_field = {0};

    for (uint32_t i = 0; i < tuple_type.fields_qty; i++) {
        MEMZERO(&unnamed_field, sizeof(unnamed_field_t));
        CHECK_ERROR(read_unnamed_field(&tuple_type.unnamed_fields, &unnamed_field));

        bool show_field =
            should_show_field(&unnamed_field.value, unnamed_field.silent, unnamed_field.is_expert, ui_expert_mode);

        bool remove_title = false;
        if (show_field) {
            if (tuple_type.has_show_as || tuple_type.has_structured_show_as) {
                CHECK_ERROR(append_structured_show_as_title(&tuple_type.structured_show_as, i, &remove_title));
            }
        }

        set_enable_push_item(show_field);
        // in the case that father field is not shown, we don't need to show the child field, even if it's enabled
        if (show_field && is_item_title_empty()) {
            set_enable_push_item(false);
        }

        switch (unnamed_field.value.tag) {
            case LINK_BY_INDEX:
                CHECK_ERROR(schema_display_generic_by_index(ctx, txObj, unnamed_field.value.data.by_index));
                break;
            case LINK_IMMEDIATE:
                CHECK_ERROR(schema_display_primitive(ctx, txObj, &unnamed_field.value.data.immediate));
                break;
            default:
                print_string("Tuple field is not a link by index\n");
                return parser_unexpected_type;
        }

        if (remove_title) {
            CHECK_ERROR(remove_last_item_title());
        }
    }

    return parser_ok;
}

parser_error_t schema_display_option(parser_context_t *ctx, parser_tx_t *txObj) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(txObj);

    uint8_t discriminant = 0;
    CHECK_ERROR(read_u8(ctx, &discriminant));
    if (discriminant == 0) {
        if (is_enable_push_item()) {
            CHECK_ERROR(push_item_string(txObj, NONE_STRING, strlen(NONE_STRING)));
        }
        CHECK_ERROR(schema_reset_leaf_offset(&txObj->merkle_proofs.leaves));
        return parser_ok;
    }

    schema_option_t option_type = {0};
    CHECK_ERROR(read_option(&txObj->merkle_proofs.leaves.data, &option_type));
    CHECK_ERROR(schema_reset_leaf_offset(&txObj->merkle_proofs.leaves));

    switch (option_type.value.tag) {
        case LINK_BY_INDEX:
            CHECK_ERROR(schema_display_generic_by_index(ctx, txObj, option_type.value.data.by_index));
            break;
        case LINK_IMMEDIATE:
            CHECK_ERROR(schema_display_primitive(ctx, txObj, &option_type.value.data.immediate));
            break;
        default:
            print_string("Option field is not a link by index\n");
            return parser_unexpected_type;
    }

    return parser_ok;
}

parser_error_t schema_display_array(parser_context_t *ctx, parser_tx_t *txObj) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(txObj);

    schema_array_t array_type = {0};
    CHECK_ERROR(read_array(&txObj->merkle_proofs.leaves.data, &array_type));
    CHECK_ERROR(schema_reset_leaf_offset(&txObj->merkle_proofs.leaves));

    for (uint32_t i = 0; i < array_type.len; i++) {
        CHECK_ERROR(append_item_title_index(i));
        switch (array_type.value.tag) {
            case LINK_BY_INDEX:
                CHECK_ERROR(schema_display_generic_by_index(ctx, txObj, array_type.value.data.by_index));
                break;
            case LINK_IMMEDIATE: {
                CHECK_ERROR(schema_display_primitive(ctx, txObj, &array_type.value.data.immediate));
                break;
            }
            default:
                print_string("Array field is not a link by index\n");
                return parser_unexpected_type;
        }

        CHECK_ERROR(remove_last_item_title())
    }

    return parser_ok;
}

parser_error_t schema_display_vec(parser_context_t *ctx, parser_tx_t *txObj) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(txObj);

    uint32_t vec_len = 0;
    CHECK_ERROR(read_u32(ctx, &vec_len));

    schema_vec_t vec_type = {0};
    vec_type.len = vec_len;

    for (uint32_t i = 0; i < vec_len; i++) {
        CHECK_ERROR(read_link(&txObj->merkle_proofs.leaves.data, &vec_type.value));
        CHECK_ERROR(schema_reset_leaf_offset(&txObj->merkle_proofs.leaves));
        CHECK_ERROR(append_item_title_index(i));
        switch (vec_type.value.tag) {
            case LINK_BY_INDEX:
                CHECK_ERROR(schema_display_generic_by_index(ctx, txObj, vec_type.value.data.by_index));
                break;
            case LINK_IMMEDIATE: {
                CHECK_ERROR(schema_display_primitive(ctx, txObj, &vec_type.value.data.immediate));
                break;
            }
            default:
                print_string("Vec field is not a link by index\n");
                return parser_unexpected_type;
        }
        CHECK_ERROR(remove_last_item_title())
    }

    return parser_ok;
}

parser_error_t schema_display_generic_by_index(parser_context_t *ctx, parser_tx_t *txObj, uint32_t index) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(txObj);

    uint64_t index_vec = 0;
    if (!schema_find_index(index, &txObj->merkle_proofs.indices, &index_vec)) {
        return parser_schema_index_not_found;
    }

    CHECK_ERROR(schema_move_leaf_offset(&txObj->merkle_proofs.leaves, index_vec));

    uint32_t len = 0;
    CHECK_ERROR(read_u32(&txObj->merkle_proofs.leaves.data, &len));

    uint8_t type = 0;
    CHECK_ERROR(read_u8(&txObj->merkle_proofs.leaves.data, &type));

    switch (type) {
        case LINKING_SCHEME_ENUM:
            CHECK_ERROR(schema_display_enum(ctx, txObj));
            break;
        case LINKING_SCHEME_STRUCT:
            CHECK_ERROR(schema_display_struct(ctx, txObj));
            break;
        case LINKING_SCHEME_TUPLE:
            CHECK_ERROR(schema_display_tuple(ctx, txObj));
            break;
        case LINKING_SCHEME_OPTION:
            CHECK_ERROR(schema_display_option(ctx, txObj));
            break;
        case LINKING_SCHEME_ARRAY:
            CHECK_ERROR(schema_display_array(ctx, txObj));
            break;
        case LINKING_SCHEME_VEC:
            CHECK_ERROR(schema_display_vec(ctx, txObj));
            break;
        default:
            print_u8("Unknown type: ", type);
            CHECK_ERROR(schema_reset_leaf_offset(&txObj->merkle_proofs.leaves));
            return parser_unexpected_type;
    }

    return parser_ok;
}

parser_error_t schema_parser_transaction(parser_context_t *ctx, parser_tx_t *txObj) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(txObj);
    ui_expert_mode = app_mode_expert();

    init_item_title_buffer(NULL);
    set_enable_push_item(true);

    txObj->unsigned_transaction_raw.buffer.ptr = ctx->buffer.ptr + ctx->offset;
    uint16_t offset_mem_txn = ctx->offset;

    uint64_t root_index = 0;
    CHECK_ERROR(schema_get_unsigned_transaction_index(txObj, &root_index));

    CHECK_ERROR(schema_display_generic_by_index(ctx, txObj, root_index));

    txObj->unsigned_transaction_raw.buffer.len = ctx->offset - offset_mem_txn;

    return parser_ok;
}
