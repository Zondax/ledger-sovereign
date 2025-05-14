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

parser_error_t schema_display_integer(parser_context_t *ctx, parser_tx_t *txObj, primitive_integer_t *primitive) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(txObj);
    CHECK_INPUT(primitive);

    parser_context_t ctx_bytes = {0};
    uint16_t len = 0;

    ctx_bytes.buffer.ptr = ctx->buffer.ptr + ctx->offset;
    switch (primitive->type) {
        case INTEGER_I8:
        case INTEGER_U8: {
            len = OFFSET_U8;
            CTX_CHECK_AND_ADVANCE(ctx, len);
            break;
        }
        case INTEGER_I16:
        case INTEGER_U16: {
            len = OFFSET_U16;
            CTX_CHECK_AND_ADVANCE(ctx, len);
            break;
        }
        case INTEGER_I32:
        case INTEGER_U32: {
            len = OFFSET_U32;
            CTX_CHECK_AND_ADVANCE(ctx, len);
            break;
        }
        case INTEGER_I64:
        case INTEGER_U64: {
            len = OFFSET_U64;
            CTX_CHECK_AND_ADVANCE(ctx, len);
            break;
        }
        case INTEGER_I128:
        case INTEGER_U128: {
            len = OFFSET_U64 * 2;
            CTX_CHECK_AND_ADVANCE(ctx, len);
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
                    len += primitive->display.fixed_point.from_sibling_field.byte_offset + 1;
                    break;
                default:
                    return parser_unexpected_type;
            }
            break;
        default:
            return parser_unexpected_type;
    }
    ctx_bytes.buffer.len = len;

    set_data_context(&ctx_bytes);

    return parser_ok;
}

parser_error_t schema_display_byte_array(parser_context_t *ctx, parser_tx_t *txObj, primitive_byte_array_t *byte_array) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(txObj);
    CHECK_INPUT(byte_array);

    parser_context_t ctx_bytes = {0};
    ctx_bytes.buffer.len = byte_array->len;
    ctx_bytes.buffer.ptr = ctx->buffer.ptr + ctx->offset;
    CTX_CHECK_AND_ADVANCE(ctx, byte_array->len)

    if (byte_array->len > MAX_INPUT_CHUNK) {
        return parser_ui_buffer_too_small;
    }

    set_data_context(&ctx_bytes);

    return parser_ok;
}

parser_error_t schema_display_primitive(parser_context_t *ctx, parser_tx_t *txObj, primitive_t *primitive) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(txObj);

    uint16_t primitive_size = sizeof(primitive);
    print_u16("Primitive size: ", primitive_size);

    set_primitive(primitive);

    switch (primitive->type) {
        case PRIMITIVE_INTEGER:
            primitive_size = sizeof(primitive->integer);
            print_u16("Primitive size integer_0: ", primitive_size);
            CHECK_ERROR(schema_display_integer(ctx, txObj, &primitive->integer));
            break;
        case PRIMITIVE_BYTE_ARRAY:
            primitive_size = sizeof(primitive->byte_array);
            print_u16("Primitive size byte_array: ", primitive_size);
            CHECK_ERROR(schema_display_byte_array(ctx, txObj, &primitive->byte_array));
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
            CHECK_ERROR(append_item_title_new((char *)variant.name.ptr, variant.name.len));
            remove_variant = true;
        }
        CHECK_ERROR(schema_display_generic_by_index(ctx, txObj, variant.value.data.by_index));
        if (remove_variant) {
            CHECK_ERROR(remove_last_item_title());
            CHECK_ERROR(remove_last_item_title_new());
        }
    } else {
        parser_context_t ctx_bytes = {0};
        ctx_bytes.buffer = variant.name;
        primitive_t primitive = {0};
        primitive.type = PRIMITIVE_STRING;
        set_primitive(&primitive);
        set_data_context(&ctx_bytes);
    }

    return parser_ok;
}

parser_error_t schema_display_struct(parser_context_t *ctx, parser_tx_t *txObj) {
    CHECK_INPUT(txObj);

    schema_struct_t struct_type = {0};
    CHECK_ERROR(read_struct(&txObj->merkle_proofs.leaves.data, &struct_type));
    CHECK_ERROR(schema_reset_leaf_offset(&txObj->merkle_proofs.leaves));

    named_field_t named_field = {0};
    if (struct_type.has_show_as || struct_type.has_structured_show_as) {
        for (uint32_t i = 0; i < struct_type.fields_qty; i++) {
            MEMZERO(&named_field, sizeof(named_field_t));
            CHECK_ERROR(read_named_field(&struct_type.named_fields, &named_field));

            bool try_push = false;
            set_enable_push_item(true);
            if (!named_field.silent || !is_link_skip(&named_field.value)) {
                if (!named_field.is_expert || ui_expert_mode) {
                    char structured_show_as[100] = {0};
                    CHECK_ERROR(find_bracket_content((char *)struct_type.structured_show_as.ptr, i, structured_show_as,
                                                     sizeof(structured_show_as)));
                    uint16_t len = strlen(structured_show_as);
                    if (len > 0) {
                        CHECK_ERROR(append_item_title_new(structured_show_as, len));
                        try_push = true;
                    }
                } else {
                    set_enable_push_item(false);
                }
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

            if (try_push) {
                if (!is_data_context_empty()) {
                    CHECK_ERROR(push_item_new(txObj));
                    CHECK_ERROR(remove_last_item_title_new());
                }
            } else {
                // TODO: check if this is correct
                clear_item_data_buffer();
            }
        }
    } else {
        for (uint32_t i = 0; i < struct_type.fields_qty; i++) {
            if (i == 2) {
                print_string("schema_display_struct IMPLEMENT ME 2");
            }
            MEMZERO(&named_field, sizeof(named_field_t));
            CHECK_ERROR(read_named_field(&struct_type.named_fields, &named_field));
            bool remove_variant = false;
            if ((!named_field.silent && !is_link_skip(&named_field.value) && !named_field.is_expert) || ui_expert_mode) {
                CHECK_ERROR(append_item_title_new((char *)named_field.display_name.ptr, named_field.display_name.len));
                remove_variant = true;
            }
            CHECK_ERROR(schema_display_generic_by_index(ctx, txObj, named_field.value.data.by_index));
            if (remove_variant) {
                if (!is_data_context_empty()) {
                    CHECK_ERROR(push_item_new(txObj));
                }
                CHECK_ERROR(remove_last_item_title_new());
            }
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
    if (tuple_type.has_show_as || tuple_type.has_structured_show_as) {
        for (uint32_t i = 0; i < tuple_type.fields_qty; i++) {
            MEMZERO(&unnamed_field, sizeof(unnamed_field_t));
            CHECK_ERROR(read_unnamed_field(&tuple_type.unnamed_fields, &unnamed_field));
            switch (unnamed_field.value.tag) {
                case LINK_BY_INDEX:
                    CHECK_ERROR(schema_display_generic_by_index(ctx, txObj, unnamed_field.value.data.by_index));
                    break;
                default:
                    print_string("Tuple field is not a link by index\n");
                    return parser_unexpected_type;
            }
        }
    } else {
        for (uint32_t i = 0; i < tuple_type.fields_qty; i++) {
            MEMZERO(&unnamed_field, sizeof(unnamed_field_t));
            CHECK_ERROR(read_unnamed_field(&tuple_type.unnamed_fields, &unnamed_field));
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
            if (tuple_type.fields_qty == 1) {
                return parser_ok;
            }

            if (!unnamed_field.is_expert || ui_expert_mode) {
                char index_str[12] = {0};
                snprintf(index_str, sizeof(index_str), "%u", i);
                CHECK_ERROR(append_item_title_new(index_str, strlen(index_str)));
                if (!is_data_context_empty()) {
                    CHECK_ERROR(push_item_new(txObj));
                    CHECK_ERROR(remove_last_item_title_new());
                }
            }
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
        parser_context_t ctx_bytes = {0};
        ctx_bytes.buffer.ptr = (uint8_t *)NONE_STRING;
        ctx_bytes.buffer.len = strlen(NONE_STRING);
        primitive_t primitive = {0};
        primitive.type = PRIMITIVE_STRING;
        set_primitive(&primitive);
        set_data_context(&ctx_bytes);
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
        char index_str[12];
        snprintf(index_str, sizeof(index_str), "%d", i);
        CHECK_ERROR(append_item_title_new(index_str, strlen(index_str)));
        switch (array_type.value.tag) {
            case LINK_BY_INDEX:
                CHECK_ERROR(schema_display_generic_by_index(ctx, txObj, array_type.value.data.by_index));
                if (!is_data_context_empty()) {
                    CHECK_ERROR(push_item_new(txObj));
                    CHECK_ERROR(remove_last_item_title_new());
                }
                break;
            case LINK_IMMEDIATE: {
                CHECK_ERROR(schema_display_primitive(ctx, txObj, &array_type.value.data.immediate));
                if (!is_data_context_empty()) {
                    CHECK_ERROR(push_item_new(txObj));
                    CHECK_ERROR(remove_last_item_title_new());
                    if (i == array_type.len - 1) {
                        CHECK_ERROR(remove_last_item_title_new());
                    }
                }
                break;
            }
            default:
                print_string("Array field is not a link by index\n");
                return parser_unexpected_type;
        }
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

        switch (vec_type.value.tag) {
            case LINK_BY_INDEX:
                CHECK_ERROR(schema_display_generic_by_index(ctx, txObj, vec_type.value.data.by_index));
                break;
            case LINK_IMMEDIATE: {
                char index_str[12];
                snprintf(index_str, sizeof(index_str), "%d", i);
                CHECK_ERROR(append_item_title_new(index_str, strlen(index_str)));
                CHECK_ERROR(schema_display_primitive(ctx, txObj, &vec_type.value.data.immediate));
                if (!is_data_context_empty()) {
                    CHECK_ERROR(push_item_new(txObj));
                    CHECK_ERROR(remove_last_item_title_new());
                    if (i == vec_len - 1) {
                        CHECK_ERROR(remove_last_item_title_new());
                    }
                }
                break;
            }
            default:
                print_string("Vec field is not a link by index\n");
                return parser_unexpected_type;
        }
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
    init_item_title_buffer_new(NULL);
    init_item_data_buffer();
    set_enable_push_item(true);

    txObj->unsigned_transaction_raw.buffer.ptr = ctx->buffer.ptr + ctx->offset;
    uint16_t offset_mem_txn = ctx->offset;

    uint64_t root_index = 0;
    CHECK_ERROR(schema_get_unsigned_transaction_index(txObj, &root_index));

    CHECK_ERROR(schema_display_generic_by_index(ctx, txObj, root_index));

    txObj->unsigned_transaction_raw.buffer.len = ctx->offset - offset_mem_txn;

    // CHECK_ERROR(test_schema_create_device_items());

    return parser_ok;
}
