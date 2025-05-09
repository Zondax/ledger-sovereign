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
char item_data[MAX_STRING_LENGTH] = {0};

#define NONE_STRING "None"

parser_error_t find_name_registry(parser_tx_t *txObj, bytes_t *name, bytes_t *input_token) {
    CHECK_INPUT(txObj);
    CHECK_INPUT(name);
    CHECK_INPUT(input_token);

    name_registries_t registries = txObj->schema.chain_data.name_registries;
    for (uint32_t i = 0; i < registries.qty; i++) {
        name_registry_t name_registry = {0};
        CHECK_ERROR(read_name_registry(&registries.vec_registries, &name_registry));
        if (name_registry.name.len == name->len && MEMCMP(name_registry.name.ptr, name->ptr, name->len) == 0) {
            for (uint32_t j = 0; j < name_registry.qty; j++) {
                registry_t registry = {0};
                CHECK_ERROR(read_registry(&name_registry.registry, &registry));
                if (registry.data.len == input_token->len &&
                    MEMCMP(registry.data.ptr, input_token->ptr, input_token->len) == 0) {
                    MEMZERO(item_data, sizeof(item_data));
                    MEMCPY(item_data, registry.name.ptr, registry.name.len);
                    append_item_data(item_data, registry.name.len);
                    return parser_ok;
                }
            }
        }
    }

    return parser_name_registry_not_found;
}

parser_error_t render_fixed_point(parser_tx_t *txObj, fixed_point_display_t display, uint128_t value) {
    CHECK_INPUT(txObj);
    print_string("render_fixed_point");

    switch (display.type) {
        case FIXED_POINT_DISPLAY_DECIMALS:
            // TODO: Implement me
            print_string("render_fixed_point IMPLEMENT ME 0");
            return parser_unexpected_type;
        case FIXED_POINT_DISPLAY_FROM_SIBLING_FIELD:
            print_u64("Value: ", display.from_sibling_field.field_index);
            print_u64("Byte offset: ", display.from_sibling_field.byte_offset);
            // get offset
            if (txObj->unsigned_transaction_raw.offset + display.from_sibling_field.byte_offset >=
                txObj->unsigned_transaction_raw.buffer.len) {
                return parser_unexpected_buffer_end;
            }
            uint8_t offset = txObj->unsigned_transaction_raw.buffer
                                 .ptr[txObj->unsigned_transaction_raw.offset + display.from_sibling_field.byte_offset];
            print_u8("Offset: ", offset);

            MEMZERO(item_data, sizeof(item_data));
            CHECK_ERROR(render_number(value.hi, value.lo, offset, "", "", item_data, sizeof(item_data)));
            append_item_data(item_data, strlen(item_data));

            return parser_ok;
        default:
            return parser_unexpected_type;
    }
}

parser_error_t render_primitive_integer(parser_tx_t *txObj, integer_display_t display, uint128_t value) {
    CHECK_INPUT(txObj);
    print_string("render_primitive_integer");

    switch (display.type) {
        case INTEGER_DISPLAY_HEX:
            // TODO: Implement me
            print_string("render_primitive_integer IMPLEMENT ME 0");
            return parser_unexpected_type;
        case INTEGER_DISPLAY_DECIMAL:
            MEMZERO(item_data, sizeof(item_data));
            CHECK_ERROR(render_number(value.hi, value.lo, 0, "", "", item_data, sizeof(item_data)));
            append_item_data(item_data, strlen(item_data));
            break;
        case INTEGER_DISPLAY_FIXED_POINT:
            CHECK_ERROR(render_fixed_point(txObj, display.fixed_point, value));
            break;
        default:
            return parser_unexpected_type;
    }

    return parser_ok;
}

parser_error_t schema_display_integer(parser_tx_t *txObj, primitive_integer_t *primitive) {
    CHECK_INPUT(txObj);
    CHECK_INPUT(primitive);
    print_string("Displaying integer");

    uint128_t value = {0};
    switch (primitive->type) {
        case INTEGER_I8:
        case INTEGER_U8: {
            uint8_t *ptr = (uint8_t *)&value;
            CHECK_ERROR(read_u8(&txObj->unsigned_transaction_raw, ptr));
            print_u8("Value: ", *ptr);
            break;
        }
        case INTEGER_I16:
        case INTEGER_U16: {
            uint16_t *ptr = (uint16_t *)&value;
            CHECK_ERROR(read_u16(&txObj->unsigned_transaction_raw, ptr));
            print_u16("Value: ", *ptr);
            break;
        }
        case INTEGER_I32:
        case INTEGER_U32: {
            uint32_t *ptr = (uint32_t *)&value;
            CHECK_ERROR(read_u32(&txObj->unsigned_transaction_raw, ptr));
            print_u32("Value: ", *ptr);
            break;
        }
        case INTEGER_I64:
        case INTEGER_U64: {
            CHECK_ERROR(read_u64(&txObj->unsigned_transaction_raw, &value.lo));
            print_u64("Value: ", value.lo);
            break;
        }
        case INTEGER_I128:
        case INTEGER_U128: {
            CHECK_ERROR(read_u64(&txObj->unsigned_transaction_raw, &value.lo));
            print_u64("Value: ", value.lo);
            CHECK_ERROR(read_u64(&txObj->unsigned_transaction_raw, &value.hi));
            print_u64("Value: ", value.hi);
            break;
        }
        default:
            return parser_unexpected_type;
    }

    CHECK_ERROR(render_primitive_integer(txObj, primitive->display, value));

    return parser_ok;
}

parser_error_t schema_display_byte_array(parser_tx_t *txObj, primitive_byte_array_t *byte_array) {
    CHECK_INPUT(txObj);
    CHECK_INPUT(byte_array);
    print_string("Displaying byte array\n");

    bytes_t array = {0};
    array.len = byte_array->len;
    array.ptr = txObj->unsigned_transaction_raw.buffer.ptr + txObj->unsigned_transaction_raw.offset;
    CTX_CHECK_AND_ADVANCE(&txObj->unsigned_transaction_raw, byte_array->len)
    print_buffer_str(&array, "Array");
    print_buffer(&array, "Array");

    if (byte_array->has_name_registry) {
        print_buffer_str(&byte_array->name_registry, "Name registry");
        // check if name registry exist
        CHECK_ERROR(find_name_registry(txObj, &byte_array->name_registry, &array));
        return parser_ok;
    }

    if (byte_array->len > MAX_INPUT_CHUNK) {
        print_string("Byte array length is greater than max input chunk\n");
        return parser_unexpected_type;
    }

    switch (byte_array->display.type) {
        case BYTE_DISPLAY_HEX:
            // TODO: Implement me
            print_string("render_primitive_byte_array IMPLEMENT ME 0");
            return parser_unexpected_type;
        case BYTE_DISPLAY_DECIMAL:
            // TODO: Implement me
            print_string("render_primitive_byte_array IMPLEMENT ME 1");
            return parser_unexpected_type;
        case BYTE_DISPLAY_BECH32:
            // TODO: Implement me
            print_string("render_primitive_byte_array IMPLEMENT ME 2");
            return parser_unexpected_type;
        case BYTE_DISPLAY_BECH32M: {
            const char *hrp = (char *)byte_array->display.bech32m.prefix.prefix.ptr;
            MEMZERO(item_data, sizeof(item_data));
            MAP_ZXERR_TO_PARSER_ERR(
                bech32EncodeFromBytes(item_data, sizeof(item_data), hrp, array.ptr, array.len, 1, BECH32_ENCODING_BECH32M));
            append_item_data(item_data, strlen(item_data));
            break;
        }
        case BYTE_DISPLAY_BASE58:
            // TODO: Implement me
            print_string("render_primitive_byte_array IMPLEMENT ME 3");
            return parser_unexpected_type;
        default:
            return parser_unexpected_type;
    }

    return parser_ok;
}

parser_error_t schema_display_enum(parser_tx_t *txObj) {
    CHECK_INPUT(txObj);
    print_string("Displaying enum\n");

    schema_enum_t enum_type = {0};
    CHECK_ERROR(read_enum(&txObj->merkle_proofs.leaves.data, &enum_type));
    CHECK_ERROR(schema_reset_leaf_offset(&txObj->merkle_proofs.leaves));

    uint8_t discriminant = 0;
    CHECK_ERROR(read_u8(&txObj->unsigned_transaction_raw, &discriminant));

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
        if (variant.hide_tag || enum_type.hide_tag) {
            print_string("Variant hide tag TRUE\n");
            print_u64("Variant index: ", variant.value.data.by_index);
        } else {
            print_string("Variant hide tag FALSE\n");
            print_buffer_str(&variant.name, "Variant name");
            CHECK_ERROR(append_item_title((char *)variant.name.ptr, variant.name.len));
            remove_variant = true;
        }
        CHECK_ERROR(schema_display_generic_by_index(txObj, variant.value.data.by_index));
        if (remove_variant) {
            CHECK_ERROR(remove_last_item_title());
        }
    } else {
        CHECK_ERROR(append_item_data((char *)variant.name.ptr, variant.name.len));
    }

    return parser_ok;
}

parser_error_t schema_display_struct(parser_tx_t *txObj) {
    CHECK_INPUT(txObj);
    print_string("Displaying struct\n");

    schema_struct_t struct_type = {0};
    CHECK_ERROR(read_struct(&txObj->merkle_proofs.leaves.data, &struct_type));
    CHECK_ERROR(schema_reset_leaf_offset(&txObj->merkle_proofs.leaves));

    // for (uint32_t i = 0; i < index_qty; i++) {
    //     uint8_t type = 0;
    //     CHECK_ERROR(get_schema_type(txObj, field_index[i], &type));
    //     print_u8("Type: ", type);
    //     if (type == LINKING_SCHEME_INTEGER) {
    //         // TODO: Implement me
    //         print_string("schema_display_struct IMPLEMENT ME 0");
    //         return parser_unexpected_type;
    //     }
    // }

    named_field_t named_field = {0};
    if (struct_type.has_show_as || struct_type.has_structured_show_as) {
        print_buffer_str(&struct_type.show_as, "Show as");
        print_buffer_str(&struct_type.structured_show_as, "Structured show as");
        for (uint32_t i = 0; i < struct_type.fields_qty; i++) {
            MEMZERO(&named_field, sizeof(named_field_t));
            CHECK_ERROR(read_named_field(&struct_type.named_fields, &named_field));
            switch (named_field.value.tag) {
                case LINK_BY_INDEX:
                    CHECK_ERROR(schema_display_generic_by_index(txObj, named_field.value.data.by_index));
                    break;
                case LINK_IMMEDIATE:
                    CHECK_ERROR(schema_display_by_primitive(txObj, &named_field.value.data.immediate));
                    break;
                default:
                    print_string("schema_display_struct IMPLEMENT ME 1");
                    return parser_unexpected_type;
            }

            if (!named_field.silent || !is_link_skip(&named_field.value)) {
                char structured_show_as[100] = {0};
                CHECK_ERROR(find_bracket_content((char *)struct_type.structured_show_as.ptr, i, structured_show_as,
                                                 sizeof(structured_show_as)));

                if (!named_field.is_expert || ui_expert_mode) {
                    uint16_t len = strlen(structured_show_as);
                    if (len > 0) {
                        CHECK_ERROR(append_item_title(structured_show_as, len));
                        if (!is_item_data_empty()) {
                            CHECK_ERROR(push_item(txObj));
                            CHECK_ERROR(remove_last_item_title());
                        }
                    }
                } else {
                    clear_item_data_buffer();
                }
            }
        }
    } else {
        print_string("No show as or structured show as\n");
        for (uint32_t i = 0; i < struct_type.fields_qty; i++) {
            MEMZERO(&named_field, sizeof(named_field_t));
            CHECK_ERROR(read_named_field(&struct_type.named_fields, &named_field));
            bool remove_variant = false;
            if ((!named_field.silent && !is_link_skip(&named_field.value) && !named_field.is_expert) || ui_expert_mode) {
                CHECK_ERROR(append_item_title((char *)named_field.display_name.ptr, named_field.display_name.len));
                remove_variant = true;
            }
            CHECK_ERROR(schema_display_generic_by_index(txObj, named_field.value.data.by_index));
            if (remove_variant) {
                if (!is_item_data_empty()) {
                    CHECK_ERROR(push_item(txObj));
                }
                CHECK_ERROR(remove_last_item_title());
            }
        }
    }

    return parser_ok;
}

parser_error_t schema_display_tuple(parser_tx_t *txObj) {
    CHECK_INPUT(txObj);
    print_string("Displaying tuple\n");

    schema_tuple_t tuple_type = {0};
    CHECK_ERROR(read_tuple(&txObj->merkle_proofs.leaves.data, &tuple_type));
    CHECK_ERROR(schema_reset_leaf_offset(&txObj->merkle_proofs.leaves));

    // for (uint32_t i = 0; i < index_qty; i++) {
    //     uint8_t type = 0;
    //     CHECK_ERROR(get_schema_type(txObj, field_index[i], &type));
    //     print_u8("Type: ", type);
    //     if (type == LINKING_SCHEME_INTEGER) {
    //         // TODO: Implement me
    //         print_string("schema_display_tuple IMPLEMENT ME 0");
    //         return parser_unexpected_type;
    //     }
    // }

    unnamed_field_t unnamed_field = {0};
    if (tuple_type.has_show_as || tuple_type.has_structured_show_as) {
        for (uint32_t i = 0; i < tuple_type.fields_qty; i++) {
            MEMZERO(&unnamed_field, sizeof(unnamed_field_t));
            CHECK_ERROR(read_unnamed_field(&tuple_type.unnamed_fields, &unnamed_field));
            switch (unnamed_field.value.tag) {
                case LINK_BY_INDEX:
                    CHECK_ERROR(schema_display_generic_by_index(txObj, unnamed_field.value.data.by_index));
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
                    CHECK_ERROR(schema_display_generic_by_index(txObj, unnamed_field.value.data.by_index));
                    break;
                case LINK_IMMEDIATE:
                    CHECK_ERROR(schema_display_by_primitive(txObj, &unnamed_field.value.data.immediate));
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
                CHECK_ERROR(append_item_title(index_str, strlen(index_str)));
                if (!is_item_data_empty()) {
                    CHECK_ERROR(push_item(txObj));
                    CHECK_ERROR(remove_last_item_title());
                }
            }
        }
    }

    return parser_ok;
}

parser_error_t schema_display_option(parser_tx_t *txObj) {
    CHECK_INPUT(txObj);
    print_string("Displaying option\n");

    uint8_t discriminant = 0;
    CHECK_ERROR(read_u8(&txObj->unsigned_transaction_raw, &discriminant));
    if (discriminant == 0) {
        print_string("Option is empty\n");
        CHECK_ERROR(append_item_data(NONE_STRING, strlen(NONE_STRING)));
        CHECK_ERROR(schema_reset_leaf_offset(&txObj->merkle_proofs.leaves));
        return parser_ok;
    }

    schema_option_t option_type = {0};
    CHECK_ERROR(read_option(&txObj->merkle_proofs.leaves.data, &option_type));
    CHECK_ERROR(schema_reset_leaf_offset(&txObj->merkle_proofs.leaves));

    switch (option_type.value.tag) {
        case LINK_BY_INDEX:
            CHECK_ERROR(schema_display_generic_by_index(txObj, option_type.value.data.by_index));
            break;
        case LINK_IMMEDIATE:
            CHECK_ERROR(schema_display_by_primitive(txObj, &option_type.value.data.immediate));
            break;
        default:
            print_string("Option field is not a link by index\n");
    }

    return parser_ok;
}

parser_error_t schema_display_array(parser_tx_t *txObj) {
    CHECK_INPUT(txObj);
    print_string("Displaying array\n");

    schema_array_t array_type = {0};
    CHECK_ERROR(read_array(&txObj->merkle_proofs.leaves.data, &array_type));
    CHECK_ERROR(schema_reset_leaf_offset(&txObj->merkle_proofs.leaves));

    for (uint32_t i = 0; i < array_type.len; i++) {
        switch (array_type.value.tag) {
            case LINK_BY_INDEX:
                CHECK_ERROR(schema_display_generic_by_index(txObj, array_type.value.data.by_index));
                break;
            case LINK_IMMEDIATE:
                CHECK_ERROR(schema_display_by_primitive(txObj, &array_type.value.data.immediate));
                break;
            default:
                print_string("Array field is not a link by index\n");
        }
    }

    return parser_ok;
}

parser_error_t schema_display_vec(parser_tx_t *txObj) {
    CHECK_INPUT(txObj);
    print_string("Displaying vec\n");

    uint32_t vec_len = 0;
    CHECK_ERROR(read_u32(&txObj->unsigned_transaction_raw, &vec_len));
    print_u32("Vec length: ", vec_len);

    schema_vec_t vec_type = {0};
    vec_type.len = vec_len;

    for (uint32_t i = 0; i < vec_len; i++) {
        CHECK_ERROR(read_link(&txObj->merkle_proofs.leaves.data, &vec_type.value));
        CHECK_ERROR(schema_reset_leaf_offset(&txObj->merkle_proofs.leaves));
        switch (vec_type.value.tag) {
            case LINK_BY_INDEX:
                CHECK_ERROR(schema_display_generic_by_index(txObj, vec_type.value.data.by_index));
                break;
            case LINK_IMMEDIATE:
                CHECK_ERROR(schema_display_by_primitive(txObj, &vec_type.value.data.immediate));
                break;
            default:
                print_string("Vec field is not a link by index\n");
        }
    }

    return parser_ok;
}

parser_error_t schema_display_by_primitive(parser_tx_t *txObj, primitive_t *primitive) {
    CHECK_INPUT(txObj);

    print_string("Displaying generic by primitive\n");

    switch (primitive->type) {
        case PRIMITIVE_INTEGER:
            CHECK_ERROR(schema_display_integer(txObj, &primitive->integer));
            break;
        case PRIMITIVE_BYTE_ARRAY:
            CHECK_ERROR(schema_display_byte_array(txObj, &primitive->byte_array));
            break;
        case PRIMITIVE_BYTE_VEC:
            // TODO: Implement me
            print_string("schema_display_by_primitive IMPLEMENT ME 0");
            return parser_unexpected_type;
        case PRIMITIVE_FLOAT32:
            // TODO: Implement me
            print_string("schema_display_by_primitive IMPLEMENT ME 1");
            return parser_unexpected_type;
        case PRIMITIVE_FLOAT64:
            // TODO: Implement me
            print_string("schema_display_by_primitive IMPLEMENT ME 2");
            return parser_unexpected_type;
        case PRIMITIVE_STRING:
            // TODO: Implement me
            print_string("schema_display_by_primitive IMPLEMENT ME 3");
            return parser_unexpected_type;
        case PRIMITIVE_BOOLEAN:
            // TODO: Implement me
            print_string("schema_display_by_primitive IMPLEMENT ME 4");
            return parser_unexpected_type;
        default:
            print_u8("Unknown type: ", primitive->type);
            CHECK_ERROR(schema_reset_leaf_offset(&txObj->merkle_proofs.leaves));
            return parser_unexpected_type;
    }

    return parser_ok;
}

parser_error_t schema_display_generic_by_index(parser_tx_t *txObj, uint32_t index) {
    CHECK_INPUT(txObj);
    print_string("Displaying generic by link\n");

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
            CHECK_ERROR(schema_display_enum(txObj));
            break;
        case LINKING_SCHEME_STRUCT:
            CHECK_ERROR(schema_display_struct(txObj));
            break;
        case LINKING_SCHEME_TUPLE:
            CHECK_ERROR(schema_display_tuple(txObj));
            break;
        case LINKING_SCHEME_OPTION:
            CHECK_ERROR(schema_display_option(txObj));
            break;
        case LINKING_SCHEME_ARRAY:
            CHECK_ERROR(schema_display_array(txObj));
            break;
        case LINKING_SCHEME_VEC:
            CHECK_ERROR(schema_display_vec(txObj));
            break;
        default:
            print_u8("Unknown type: ", type);
            CHECK_ERROR(schema_reset_leaf_offset(&txObj->merkle_proofs.leaves));
            return parser_unexpected_type;
    }

    return parser_ok;
}

parser_error_t add_navigate_up(parser_tx_t *txObj, uint8_t navigate_up_qty, uint16_t greater_than_indexes[],
                               uint8_t greater_than_qty) {
    CHECK_INPUT(txObj);

    for (uint8_t k = 0; k < navigate_up_qty; k++) {
        char local_title[100] = {0};
        uint16_t index_title_greater_than = greater_than_indexes[greater_than_qty - k - 1];
        MEMCPY(local_title, txObj->device_items.items[index_title_greater_than].title,
               strlen(txObj->device_items.items[index_title_greater_than].title));
        MEMCPY(txObj->device_items.items[txObj->device_items.qty].title, local_title, strlen(local_title));
        MEMCPY(txObj->device_items.items[txObj->device_items.qty].data, NAVIGATE_UP, strlen(NAVIGATE_UP));
        txObj->device_items.qty++;
    }
    return parser_ok;
}

parser_error_t schema_create_device_items(parser_tx_t *txObj) {
    CHECK_INPUT(txObj);

    uint8_t title_qty = 1;

    print_string("schema_create_device_items: ");
    print_u32("schema_create_device_items: ", txObj->ui_items.qty);
    for (uint32_t i = 0; i < txObj->ui_items.qty; i++) {
        print_string(txObj->ui_items.items[i].title);
        print_string(txObj->ui_items.items[i].data);
    }

    uint8_t greater_than_qty = 0;
    uint16_t device_item_base = 0;
    bool use_navigate_up = false;
    uint16_t greater_than_indexes[MAX_ITEMS] = {0};
    for (uint32_t i = 0; i < txObj->ui_items.qty; i++) {
        if (txObj->device_items.qty > MAX_ITEMS - 1) {
            return parser_too_many_items;
        }

        clear_item_title_buffer();
        init_item_title_buffer(txObj->ui_items.items[i].title);

        uint8_t items_qty = 0;
        CHECK_ERROR(get_title_item_qty(&items_qty));
        if (items_qty == 0) {
            return parser_ui_item_title_empty;
        }
        print_u8("items_qty: ", items_qty);
        char content_title[100] = {0};
        if (items_qty >= title_qty) {
            uint8_t title_deep = items_qty - title_qty + 1;
            for (uint8_t j = 0; j < title_deep; j++) {
                MEMZERO(content_title, sizeof(content_title));
                CHECK_ERROR(create_item_title(j, j + title_qty, content_title, sizeof(content_title)));
                bool exists = false;
                for (uint16_t k = device_item_base; k < txObj->device_items.qty; k++) {
                    if (MEMCMP(txObj->device_items.items[k].title, content_title, strlen(content_title)) == 0) {
                        exists = true;
                        break;
                    }
                }
                if (!exists && (j + title_qty < items_qty)) {
                    if (greater_than_qty >= items_qty) {
                        uint8_t navigate_up_qty = greater_than_qty - j;
                        CHECK_ERROR(add_navigate_up(txObj, navigate_up_qty, greater_than_indexes, greater_than_qty));
                        greater_than_qty -= navigate_up_qty;
                    }
                    MEMCPY(txObj->device_items.items[txObj->device_items.qty].title, content_title, strlen(content_title));
                    MEMCPY(txObj->device_items.items[txObj->device_items.qty].data, PAGE_BREAK, strlen(PAGE_BREAK));
                    use_navigate_up = true;
                    greater_than_indexes[greater_than_qty] = txObj->device_items.qty;
                    greater_than_qty++;
                    txObj->device_items.qty++;
                }
                if (j + title_qty == items_qty) {
                    if (greater_than_qty >= items_qty) {
                        uint8_t navigate_up_qty = greater_than_qty - items_qty + 1;
                        CHECK_ERROR(add_navigate_up(txObj, navigate_up_qty, greater_than_indexes, greater_than_qty));
                        greater_than_qty -= navigate_up_qty;
                    }
                    MEMZERO(content_title, sizeof(content_title));
                    CHECK_ERROR(create_item_title(j, items_qty, content_title, sizeof(content_title)));
                }
                if (greater_than_qty == 0 && use_navigate_up) {
                    device_item_base = txObj->device_items.qty;
                    use_navigate_up = false;
                }
            }
        } else {
            if (greater_than_qty > 0) {
                uint8_t navigate_up_qty = greater_than_qty - items_qty + 1;
                CHECK_ERROR(add_navigate_up(txObj, navigate_up_qty, greater_than_indexes, greater_than_qty));
                greater_than_qty -= navigate_up_qty;
            }
            if (greater_than_qty == 0 && use_navigate_up) {
                device_item_base = txObj->device_items.qty;
                use_navigate_up = false;
            }
            CHECK_ERROR(create_item_title(0, title_qty - 1, content_title, sizeof(content_title)));
        }

        MEMCPY(txObj->device_items.items[txObj->device_items.qty].title, content_title, strlen(content_title));
        MEMCPY(txObj->device_items.items[txObj->device_items.qty].data, txObj->ui_items.items[i].data,
               strlen(txObj->ui_items.items[i].data));
        txObj->device_items.qty++;
    }

    print_string("\nSchema_create_device_items!!: ");
    for (uint8_t i = 0; i < txObj->device_items.qty; i++) {
        print_string(txObj->device_items.items[i].title);
        print_string(txObj->device_items.items[i].data);
    }

    return parser_ok;
}

parser_error_t schema_create_device_items_2(parser_tx_t *txObj) {
    CHECK_INPUT(txObj);

    uint8_t title_qty = 1;

    print_string("schema_create_device_items: ");
    print_u32("schema_create_device_items: ", txObj->ui_items.qty);
    for (uint32_t i = 0; i < txObj->ui_items.qty; i++) {
        print_string(txObj->ui_items.items[i].title);
        print_string(txObj->ui_items.items[i].data);
    }

    uint8_t greater_than_qty = 0;
    uint16_t device_item_base = 0;
    bool use_navigate_up = false;
    uint16_t greater_than_indexes[MAX_ITEMS] = {0};
    for (uint32_t i = 0; i < txObj->ui_items.qty; i++) {
        if (txObj->device_items.qty > MAX_ITEMS - 1) {
            return parser_too_many_items;
        }

        clear_item_title_buffer();
        init_item_title_buffer(txObj->ui_items.items[i].title);

        uint8_t items_qty = 0;
        CHECK_ERROR(get_title_item_qty(&items_qty));
        if (items_qty == 0) {
            return parser_ui_item_title_empty;
        }
        print_u8("items_qty: ", items_qty);
        char content_title[100] = {0};
        char content_title_2[100] = {0};
        char *path_str = "\n[";
        if (items_qty > 1) {
            MEMZERO(content_title_2, sizeof(content_title_2));
            strncat(content_title_2, path_str, strlen(path_str));
            CHECK_ERROR(create_item_title(items_qty - 1, items_qty, content_title, sizeof(content_title)));
            CHECK_ERROR(create_item_title(0, items_qty - 1, content_title_2 + strlen(path_str),
                                          sizeof(content_title_2) - strlen(path_str)));
            if (strlen(content_title) <= 1) {
                MEMZERO(content_title, sizeof(content_title));
                CHECK_ERROR(create_item_title(items_qty - 2, items_qty, content_title, sizeof(content_title)));
                strncat(content_title_2, path_str, strlen(path_str));
                CHECK_ERROR(create_item_title(0, items_qty - 2, content_title_2 + strlen(path_str),
                                              sizeof(content_title_2) - strlen(path_str)));
            }
            strncat(content_title_2, "]", 1);
        } else {
            CHECK_ERROR(create_item_title(items_qty - 1, items_qty, content_title, sizeof(content_title)));
            MEMZERO(content_title_2, sizeof(content_title_2));
        }

        char content_title_3[100] = {0};
        strncat(content_title_3, txObj->ui_items.items[i].data, strlen(txObj->ui_items.items[i].data));
        strncat(content_title_3, content_title_2, strlen(content_title_2));

        MEMCPY(txObj->device_items.items[txObj->device_items.qty].title, content_title, strlen(content_title));
        MEMCPY(txObj->device_items.items[txObj->device_items.qty].data, content_title_3, strlen(content_title_3));
        txObj->device_items.qty++;
    }

    print_string("\nSchema_create_device_items!!: ");
    for (uint8_t i = 0; i < txObj->device_items.qty; i++) {
        print_string(txObj->device_items.items[i].title);
        print_string(txObj->device_items.items[i].data);
    }

    return parser_ok;
}

parser_error_t test_schema_create_device_items() {
    parser_tx_t txObj = {0};

    // uint8_t qty = 0;
    // char *title1 = "|deep1|deep2|value3|";
    // char *data1 = "10000";
    // MEMCPY(txObj.ui_items.items[qty].title, title1, strlen(title1));
    // MEMCPY(txObj.ui_items.items[qty].data, data1, strlen(data1));
    // qty++;

    // char *title2 = "|deep1|deep2|deep3|deep4|deep5|value6|";
    // char *data2 = "20000";
    // MEMCPY(txObj.ui_items.items[qty].title, title2, strlen(title2));
    // MEMCPY(txObj.ui_items.items[qty].data, data2, strlen(data2));
    // qty++;

    // char *title3 = "|deep1|deep2|deep3|deep4|value5|";
    // char *data3 = "30000";
    // MEMCPY(txObj.ui_items.items[qty].title, title3, strlen(title3));
    // MEMCPY(txObj.ui_items.items[qty].data, data3, strlen(data3));
    // qty++;

    // char *title4 = "|value1|";
    // char *data4 = "40000";
    // MEMCPY(txObj.ui_items.items[qty].title, title4, strlen(title4));
    // MEMCPY(txObj.ui_items.items[qty].data, data4, strlen(data4));
    // qty++;

    // char *title5 = "|deep1|deep2|deep3|deep4|deep5|value6|";
    // char *data5 = "20000";
    // MEMCPY(txObj.ui_items.items[qty].title, title5, strlen(title5));
    // MEMCPY(txObj.ui_items.items[qty].data, data5, strlen(data5));
    // qty++;

    // char *title6 = "|value1|";
    // char *data6 = "60000";
    // MEMCPY(txObj.ui_items.items[qty].title, title6, strlen(title6));
    // MEMCPY(txObj.ui_items.items[qty].data, data6, strlen(data6));
    // qty++;

    ////////////////////////////////////////////////////////////////

    // char *title1 = "|RegisterPaymaster|policy|default_payee_policy|Allow|max_fee|";
    // char *data1 = "10000";
    // MEMCPY(txObj.ui_items.items[qty].title, title1, strlen(title1));
    // MEMCPY(txObj.ui_items.items[qty].data, data1, strlen(data1));
    // qty++;

    // char *title2 = "|RegisterPaymaster|policy|default_payee_policy|Allow|gas_limit|";
    // char *data2 = "None";
    // MEMCPY(txObj.ui_items.items[qty].title, title2, strlen(title2));
    // MEMCPY(txObj.ui_items.items[qty].data, data2, strlen(data2));
    // qty++;

    // char *title3 = "|RegisterPaymaster|policy|default_payee_policy|Allow|max_gas_price|value|0|";
    // char *data3 = "2000";
    // MEMCPY(txObj.ui_items.items[qty].title, title3, strlen(title3));
    // MEMCPY(txObj.ui_items.items[qty].data, data3, strlen(data3));
    // qty++;

    // char *title3_1 = "|RegisterPaymaster|policy|default_payee_policy|Allow|max_gas_price|value|1|";
    // char *data3_1 = "2000";
    // MEMCPY(txObj.ui_items.items[qty].title, title3_1, strlen(title3_1));
    // MEMCPY(txObj.ui_items.items[qty].data, data3_1, strlen(data3_1));
    // qty++;

    // char *title4 = "|RegisterPaymaster|policy|default_payee_policy|Allow|transaction_limit|";
    // char *data4 = "50";
    // MEMCPY(txObj.ui_items.items[qty].title, title4, strlen(title4));
    // MEMCPY(txObj.ui_items.items[qty].data, data4, strlen(data4));
    // qty++;

    // char *title5 = "|RegisterPaymaster|policy|payees|0|";
    // char *data5 = "sov1pv9skzctpv9skzctpv9skzctpv9skzctpv9skzctpv9skqm7ehv";
    // MEMCPY(txObj.ui_items.items[qty].title, title5, strlen(title5));
    // MEMCPY(txObj.ui_items.items[qty].data, data5, strlen(data5));
    // qty++;

    // char *title6 = "|RegisterPaymaster|policy|payees|1|";
    // char *data6 = "Deny";
    // MEMCPY(txObj.ui_items.items[qty].title, title6, strlen(title6));
    // MEMCPY(txObj.ui_items.items[qty].data, data6, strlen(data6));
    // qty++;

    // char *title8 = "|RegisterPaymaster|policy|authorized_sequencers|";
    // char *data8 = "All";
    // MEMCPY(txObj.ui_items.items[qty].title, title8, strlen(title8));
    // MEMCPY(txObj.ui_items.items[qty].data, data8, strlen(data8));
    // qty++;

    // char *title9 = "|Max Fee|";
    // char *data9 = "100000000000";
    // MEMCPY(txObj.ui_items.items[qty].title, title9, strlen(title9));
    // MEMCPY(txObj.ui_items.items[qty].data, data9, strlen(data9));
    // qty++;

    // char *title10 = "|Chain ID|";
    // char *data10 = "4321";
    // MEMCPY(txObj.ui_items.items[qty].title, title10, strlen(title10));
    // MEMCPY(txObj.ui_items.items[qty].data, data10, strlen(data10));
    // qty++;

    ////////////////////////////////////////////////////////////////

    uint8_t qty = 0;
    char *title1 = "|deep1|deep2|deep3|deep4|deep5|deep6|value7|";
    char *data1 = "10000";
    MEMCPY(txObj.ui_items.items[qty].title, title1, strlen(title1));
    MEMCPY(txObj.ui_items.items[qty].data, data1, strlen(data1));
    qty++;

    char *title2 = "|deep1|deep2|deep3|deep4|value5|";
    char *data2 = "20000";
    MEMCPY(txObj.ui_items.items[qty].title, title2, strlen(title2));
    MEMCPY(txObj.ui_items.items[qty].data, data2, strlen(data2));
    qty++;

    char *title4 = "|deep1|deep2|new_deep3|value4|";
    char *data4 = "40000";
    MEMCPY(txObj.ui_items.items[qty].title, title4, strlen(title4));
    MEMCPY(txObj.ui_items.items[qty].data, data4, strlen(data4));
    qty++;

    char *title5 = "|deep1|deep2|new_deep3|value4_new|";
    char *data5 = "50000";
    MEMCPY(txObj.ui_items.items[qty].title, title5, strlen(title5));
    MEMCPY(txObj.ui_items.items[qty].data, data5, strlen(data5));
    qty++;

    char *title6 = "|deep1|deep2|value3|";
    char *data6 = "60000";
    MEMCPY(txObj.ui_items.items[qty].title, title6, strlen(title6));
    MEMCPY(txObj.ui_items.items[qty].data, data6, strlen(data6));
    qty++;

    txObj.ui_items.qty = qty;
    CHECK_ERROR(schema_create_device_items_2(&txObj));
    return parser_ok;
}

parser_error_t schema_display(parser_tx_t *txObj, uint32_t start_index) {
    CHECK_INPUT(txObj);
    ui_expert_mode = app_mode_expert();

    init_item_title_buffer(NULL);
    init_item_data_buffer();

    CHECK_ERROR(schema_display_generic_by_index(txObj, start_index));

    // check input offset
    if (txObj->unsigned_transaction_raw.offset != txObj->unsigned_transaction_raw.buffer.len) {
        print_string("Input offset is not the same as buffer length\n");
        return parser_unexpected_type;
    }

    CHECK_ERROR(schema_create_device_items(txObj));
    // CHECK_ERROR(test_schema_create_device_items());

    return parser_ok;
}
