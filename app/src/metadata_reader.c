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

#include "metadata_reader.h"

#include "borsh.h"

#if defined(TARGET_NANOS) || defined(TARGET_NANOX) || defined(TARGET_NANOS2) || defined(TARGET_STAX) || defined(TARGET_FLEX)
#define STACK_SHIFT 20
#define MINIMUM_STACK 400
#else
static uint16_t recursionDepthCounter = 0;
#define MAX_RECURSION_DEPTH 50
#endif

/**
 * @brief Checks the available stack space to prevent stack overflow.
 *
 * @return parser_error_t Returns parser_running_out_of_stack if stack space is insufficient, otherwise parser_ok.
 */
parser_error_t checkStack() {
#if defined(TARGET_NANOS) || defined(TARGET_NANOX) || defined(TARGET_NANOS2) || defined(TARGET_STAX) || defined(TARGET_FLEX)
    // NOLINTNEXTLINE(readability-identifier-length): here `p` is fine
    void *p = NULL;
    const uint32_t availableStack = (uint32_t)((void *)&p) + STACK_SHIFT - (uint32_t)&app_stack_canary;
    ZEMU_LOGF(50, "Available stack: %d\n", availableStack)
    if (availableStack <= MINIMUM_STACK) {
        return parser_running_out_of_stack;
    }
#else
    if (recursionDepthCounter >= MAX_RECURSION_DEPTH) {
        return parser_running_out_of_stack;
    }
    recursionDepthCounter++;
#endif
    return parser_ok;
}

/**
 * @brief Frees the stack space by decrementing the recursion depth counter.
 *
 * @return parser_error_t Always returns parser_ok.
 */
parser_error_t freeStack() {
#if !defined(TARGET_NANOS) && !defined(TARGET_NANOX) && !defined(TARGET_NANOS2) && !defined(TARGET_STAX) && \
    !defined(TARGET_FLEX)
    if (recursionDepthCounter > 0) {
        recursionDepthCounter--;
    }
#endif
    return parser_ok;
}

parser_error_t read_fixed_point_display(parser_context_t *ctx, fixed_point_display_t *display) {
    CHECK_INPUT(display);
    CHECK_INPUT(ctx);

    // read type
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&display->type));
    print_u8("type fixed point display:", display->type);

    switch (display->type) {
        case FIXED_POINT_DISPLAY_DECIMALS:
            CHECK_ERROR(read_u8(ctx, (uint8_t *)&display->decimals));
            print_u8("fixed point display decimals:", display->decimals);
            break;
        case FIXED_POINT_DISPLAY_FROM_SIBLING_FIELD:
            CHECK_ERROR(read_u64(ctx, (uint64_t *)&display->from_sibling_field.field_index));
            CHECK_ERROR(read_u64(ctx, (uint64_t *)&display->from_sibling_field.byte_offset));
            print_u64("fixed point display from sibling field:", display->from_sibling_field.field_index);
            print_u64("fixed point display from sibling field:", display->from_sibling_field.byte_offset);
            break;
        default:
            print_string("fixed point display unknown!!!!");
            return parser_no_data;
    }

    return parser_ok;
}

parser_error_t read_integer_display(parser_context_t *ctx, integer_display_t *display) {
    CHECK_INPUT(display);
    CHECK_INPUT(ctx);

    // read type
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&display->type));
    print_u8("type integer display:", display->type);

    switch (display->type) {
        case INTEGER_DISPLAY_HEX:
            print_string("integer display hex");
            break;
        case INTEGER_DISPLAY_DECIMAL:
            print_string("integer display decimal");
            break;
        case INTEGER_DISPLAY_FIXED_POINT:
            print_string("integer display fixed point");
            CHECK_ERROR(read_fixed_point_display(ctx, &display->fixed_point));
            break;
        default:
            print_string("integer display unknown!!!!");
            return parser_no_data;
    }
    return parser_ok;
}

parser_error_t read_hrp(parser_context_t *ctx, hrp_t *hrp) {
    CHECK_INPUT(hrp);
    CHECK_INPUT(ctx);

    CHECK_ERROR(read_u32(ctx, (uint32_t *)&hrp->prefix.len));
    print_u16("hrp size:", hrp->prefix.len);
    if (hrp->prefix.len > 0) {
        hrp->prefix.ptr = ctx->buffer.ptr + ctx->offset;
        CTX_CHECK_AND_ADVANCE(ctx, hrp->prefix.len);
    }
    print_buffer(&hrp->prefix, "hrp");
    print_buffer_str(&hrp->prefix, "hrp_str");

    return parser_ok;
}

parser_error_t read_byte_display(parser_context_t *ctx, byte_display_t *display) {
    CHECK_INPUT(display);
    CHECK_INPUT(ctx);

    // read type
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&display->type));
    print_u8("type byte display:", display->type);

    switch (display->type) {
        case BYTE_DISPLAY_HEX:
            print_string("byte display hex");
            break;
        case BYTE_DISPLAY_DECIMAL:
            print_string("byte display decimal");
            break;
        case BYTE_DISPLAY_BECH32:
            print_string("byte display bech32");
            CHECK_ERROR(read_hrp(ctx, &display->bech32.prefix));
            break;
        case BYTE_DISPLAY_BECH32M:
            print_string("byte display bech32m");
            CHECK_ERROR(read_hrp(ctx, &display->bech32m.prefix));
            break;
        case BYTE_DISPLAY_BASE58:
            print_string("byte display base58");
            break;
        default:
            print_string("byte display unknown!!!!");
            return parser_no_data;
    }

    return parser_ok;
}

parser_error_t read_structured_display_overrides(parser_context_t *ctx, structured_display_overrides_t *display) {
    CHECK_INPUT(display);
    CHECK_INPUT(ctx);

    // read has_title_elements
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&display->has_title_elements));
    print_u8("has_title_elements:", display->has_title_elements);
    if (display->has_title_elements) {
        // read title_elements
        CHECK_ERROR(read_u64(ctx, (uint64_t *)&display->title_elements));
        print_u64("title_elements:", display->title_elements);
    }

    // read has_title_lines
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&display->has_title_lines));
    print_u8("has_title_lines:", display->has_title_lines);
    if (display->has_title_lines) {
        // read title_lines
        CHECK_ERROR(read_u64(ctx, (uint64_t *)&display->title_lines));
        print_u64("title_lines:", display->title_lines);
    }

    return parser_ok;
}

parser_error_t read_primitive_integer(parser_context_t *ctx, primitive_integer_t *primitive) {
    CHECK_INPUT(primitive);
    CHECK_INPUT(ctx);

    // read type
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&primitive->type));
    print_u8("type primitive integer:", primitive->type);

    // read display
    CHECK_ERROR(read_integer_display(ctx, &primitive->display));

    return parser_ok;
}

parser_error_t read_primitive_byte_array(parser_context_t *ctx, primitive_byte_array_t *primitive) {
    CHECK_INPUT(primitive);
    CHECK_INPUT(ctx);

    // read length
    CHECK_ERROR(read_u64(ctx, (uint64_t *)&primitive->len));
    print_u64("length primitive byte array:", primitive->len);

    // read display
    CHECK_ERROR(read_byte_display(ctx, &primitive->display));

    return parser_ok;
}

parser_error_t read_immediate(parser_context_t *ctx, primitive_t *primitive) {
    CHECK_INPUT(primitive);
    CHECK_INPUT(ctx);

    // read type
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&primitive->type));
    print_u8("type immediate:", primitive->type);

    // read data
    switch (primitive->type) {
        case PRIMITIVE_INTEGER:
            print_string("Primitive integer");
            CHECK_ERROR(read_primitive_integer(ctx, &primitive->integer));
            break;
        case PRIMITIVE_BYTE_ARRAY:
            print_string("Primitive byte array");
            CHECK_ERROR(read_primitive_byte_array(ctx, &primitive->byte_array));
            break;
        case PRIMITIVE_BYTE_VEC:
            print_string("Primitive byte vec");
            CHECK_ERROR(read_byte_display(ctx, &primitive->byte_vec.display));
            break;
        case PRIMITIVE_FLOAT32:
            print_string("Primitive float32");
            break;
        case PRIMITIVE_FLOAT64:
            print_string("Primitive float64");
            break;
        case PRIMITIVE_STRING:
            print_string("Primitive string");
            break;
        case PRIMITIVE_BOOLEAN:
            print_string("Primitive boolean");
            break;
        case PRIMITIVE_SKIP:
            print_string("Primitive skip");
            break;
        default:
            print_string("link unknown!!!!");
            return parser_no_data;
    }
    return parser_ok;
}

parser_error_t read_link(parser_context_t *ctx, link_t *link) {
    CHECK_INPUT(link);
    CHECK_INPUT(ctx);

    // read tag
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&link->tag));
    print_u8("link tag:", link->tag);

    // read data
    switch (link->tag) {
        case LINK_BY_INDEX:
            CHECK_ERROR(read_u64(ctx, (uint64_t *)&link->data.by_index));
            print_u64("link by index:", link->data.by_index);
            break;
        case LINK_IMMEDIATE:
            CHECK_ERROR(read_immediate(ctx, &link->data.immediate));
            break;
        default:
            // TODO: change this
            print_string("link unknown!!!!");
            return parser_no_data;
    }

    return parser_ok;
}

parser_error_t read_enum_variant(parser_context_t *ctx, enum_variant_t *variant) {
    CHECK_INPUT(variant);
    CHECK_INPUT(ctx);

    // read name
    CHECK_ERROR(read_u32(ctx, (uint32_t *)&variant->name.len));
    if (variant->name.len > 0) {
        variant->name.ptr = ctx->buffer.ptr + ctx->offset;
        CTX_CHECK_AND_ADVANCE(ctx, variant->name.len);
    }
    print_buffer(&variant->name, "name");
    print_buffer_str(&variant->name, "name_str");

    // read discriminant
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&variant->discriminant));
    print_u8("discriminant:", variant->discriminant);

    // read hide_tag
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&variant->hide_tag));
    print_u8("hide_tag:", variant->hide_tag);

    // read has_value
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&variant->has_value));
    print_u8("has_value:", variant->has_value);
    if (variant->has_value) {
        // read value
        CHECK_ERROR(read_link(ctx, &variant->value));
    }

    return parser_ok;
}

parser_error_t read_named_field(parser_context_t *ctx, named_field_t *field) {
    CHECK_INPUT(field);
    CHECK_INPUT(ctx);

    // read display_name
    CHECK_ERROR(read_u32(ctx, (uint32_t *)&field->display_name.len));
    if (field->display_name.len > 0) {
        field->display_name.ptr = ctx->buffer.ptr + ctx->offset;
        CTX_CHECK_AND_ADVANCE(ctx, field->display_name.len);
    }
    print_buffer(&field->display_name, "display_name");
    print_buffer_str(&field->display_name, "display_name_str");

    // read silent
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&field->silent));
    print_u8("silent:", field->silent);

    // read is_expert
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&field->is_expert));
    print_u8("is_expert:", field->is_expert);

    // read value
    CHECK_ERROR(read_link(ctx, &field->value));

    // read doc
    CHECK_ERROR(read_u32(ctx, (uint32_t *)&field->doc.len));
    if (field->doc.len > 0) {
        field->doc.ptr = ctx->buffer.ptr + ctx->offset;
        CTX_CHECK_AND_ADVANCE(ctx, field->doc.len);
    }
    print_buffer(&field->doc, "doc");

    return parser_ok;
}

parser_error_t read_unnamed_field(parser_context_t *ctx, unnamed_field_t *field) {
    CHECK_INPUT(field);
    CHECK_INPUT(ctx);

    // read value
    CHECK_ERROR(read_link(ctx, &field->value));

    // read silent
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&field->silent));
    print_u8("silent:", field->silent);

    // read is_expert
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&field->is_expert));
    print_u8("is_expert:", field->is_expert);

    // read doc
    CHECK_ERROR(read_u32(ctx, (uint32_t *)&field->doc.len));
    if (field->doc.len > 0) {
        field->doc.ptr = ctx->buffer.ptr + ctx->offset;
        CTX_CHECK_AND_ADVANCE(ctx, field->doc.len);
    }
    print_buffer(&field->doc, "doc");

    return parser_ok;
}

parser_error_t read_enum(parser_context_t *ctx, schema_enum_t *schema_enum) {
    CHECK_INPUT(schema_enum);
    CHECK_INPUT(ctx);

    // read length of display name
    CHECK_ERROR(read_u32(ctx, (uint32_t *)&schema_enum->type_name.len));
    if (schema_enum->type_name.len > 0) {
        schema_enum->type_name.ptr = ctx->buffer.ptr + ctx->offset;
        CTX_CHECK_AND_ADVANCE(ctx, schema_enum->type_name.len);
    }
    print_buffer(&schema_enum->type_name, "type_name");
    print_buffer_str(&schema_enum->type_name, "type_name_str");

    // read variants_qty
    CHECK_ERROR(read_u32(ctx, &schema_enum->variants_qty));
    print_u32("variants_qty:", schema_enum->variants_qty);
    if (schema_enum->variants_qty > MAX_VARIANTS_QTY) {
        return parser_too_many_variants;
    }
    for (uint32_t i = 0; i < schema_enum->variants_qty; i++) {
        enum_variant_t variant = {0};
        CHECK_ERROR(read_enum_variant(ctx, &variant));
        schema_enum->variants[i] = variant;
    }

    // read hide_tag
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&schema_enum->hide_tag));
    print_u8("hide_tag:", schema_enum->hide_tag);

    // read structured_display_overrides
    CHECK_ERROR(read_structured_display_overrides(ctx, &schema_enum->structured_display_overrides));

    return parser_ok;
}

parser_error_t read_struct(parser_context_t *ctx, schema_struct_t *schema_struct) {
    CHECK_INPUT(schema_struct);
    CHECK_INPUT(ctx);

    // read length of display name
    CHECK_ERROR(read_u32(ctx, (uint32_t *)&schema_struct->type_name.len));
    if (schema_struct->type_name.len > 0) {
        schema_struct->type_name.ptr = ctx->buffer.ptr + ctx->offset;
        CTX_CHECK_AND_ADVANCE(ctx, schema_struct->type_name.len);
    }
    print_buffer(&schema_struct->type_name, "type_name");
    print_buffer_str(&schema_struct->type_name, "type_name_str");

    // read has_show_as
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&schema_struct->has_show_as));
    print_u8("has_show_as:", schema_struct->has_show_as);
    if (schema_struct->has_show_as) {
        CHECK_ERROR(read_u32(ctx, (uint32_t *)&schema_struct->show_as.len));
        if (schema_struct->show_as.len == 0) {
            return parser_unexpected_error;
        }
        if (schema_struct->show_as.len > 0) {
            schema_struct->show_as.ptr = ctx->buffer.ptr + ctx->offset;
            CTX_CHECK_AND_ADVANCE(ctx, schema_struct->show_as.len);
        }
        print_buffer(&schema_struct->show_as, "show_as");
        print_buffer_str(&schema_struct->show_as, "show_as_str");
    }

    // read has_structured_show_as
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&schema_struct->has_structured_show_as));
    print_u8("has_structured_show_as:", schema_struct->has_structured_show_as);
    if (schema_struct->has_structured_show_as) {
        CHECK_ERROR(read_u32(ctx, (uint32_t *)&schema_struct->structured_show_as.len));
        if (schema_struct->structured_show_as.len == 0) {
            return parser_unexpected_error;
        }
        if (schema_struct->structured_show_as.len > 0) {
            schema_struct->structured_show_as.ptr = ctx->buffer.ptr + ctx->offset;
            CTX_CHECK_AND_ADVANCE(ctx, schema_struct->structured_show_as.len);
        }
        print_buffer(&schema_struct->structured_show_as, "structured_show_as");
        print_buffer_str(&schema_struct->structured_show_as, "structured_show_as_str");
    }

    // read peekable
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&schema_struct->peekable));
    print_u8("peekable:", schema_struct->peekable);

    // read fields
    CHECK_ERROR(read_u32(ctx, &schema_struct->fields_qty));
    if (schema_struct->fields_qty > MAX_FIELDS_QTY) {
        return parser_too_many_fields;
    }
    print_u32("fields_qty:", schema_struct->fields_qty);
    for (uint32_t i = 0; i < schema_struct->fields_qty; i++) {
        named_field_t field = {0};
        CHECK_ERROR(read_named_field(ctx, &field));
        schema_struct->fields[i] = field;
    }

    // read structured_display_overrides
    CHECK_ERROR(read_structured_display_overrides(ctx, &schema_struct->structured_display_overrides));

    return parser_ok;
}

parser_error_t read_tuple(parser_context_t *ctx, schema_tuple_t *schema_tuple) {
    CHECK_INPUT(schema_tuple);
    CHECK_INPUT(ctx);

    // read has_show_as
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&schema_tuple->has_show_as));
    print_u8("has_show_as:", schema_tuple->has_show_as);
    if (schema_tuple->has_show_as) {
        CHECK_ERROR(read_u32(ctx, (uint32_t *)&schema_tuple->show_as.len));
        if (schema_tuple->show_as.len == 0) {
            return parser_unexpected_error;
        }
        if (schema_tuple->show_as.len > 0) {
            schema_tuple->show_as.ptr = ctx->buffer.ptr + ctx->offset;
            CTX_CHECK_AND_ADVANCE(ctx, schema_tuple->show_as.len);
        }
        print_buffer(&schema_tuple->show_as, "show_as");
        print_buffer_str(&schema_tuple->show_as, "show_as_str");
    }

    // read has_structured_show_as
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&schema_tuple->has_structured_show_as));
    print_u8("has_structured_show_as:", schema_tuple->has_structured_show_as);
    if (schema_tuple->has_structured_show_as) {
        CHECK_ERROR(read_u32(ctx, (uint32_t *)&schema_tuple->structured_show_as.len));
        if (schema_tuple->structured_show_as.len == 0) {
            return parser_unexpected_error;
        }
        if (schema_tuple->structured_show_as.len > 0) {
            schema_tuple->structured_show_as.ptr = ctx->buffer.ptr + ctx->offset;
            CTX_CHECK_AND_ADVANCE(ctx, schema_tuple->structured_show_as.len);
        }
        print_buffer(&schema_tuple->structured_show_as, "structured_show_as");
        print_buffer_str(&schema_tuple->structured_show_as, "structured_show_as_str");
    }

    // read peekable
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&schema_tuple->peekable));
    print_u8("peekable:", schema_tuple->peekable);

    // read fields
    CHECK_ERROR(read_u32(ctx, &schema_tuple->fields_qty));
    if (schema_tuple->fields_qty > MAX_FIELDS_QTY) {
        return parser_too_many_fields;
    }
    print_u32("fields_qty:", schema_tuple->fields_qty);
    for (uint32_t i = 0; i < schema_tuple->fields_qty; i++) {
        unnamed_field_t field = {0};
        CHECK_ERROR(read_unnamed_field(ctx, &field));
        schema_tuple->fields[i] = field;
    }

    // read structured_display_overrides
    CHECK_ERROR(read_structured_display_overrides(ctx, &schema_tuple->structured_display_overrides));

    return parser_ok;
}

parser_error_t read_array(parser_context_t *ctx, schema_array_t *schema_array) {
    CHECK_INPUT(schema_array);
    CHECK_INPUT(ctx);

    // read length
    CHECK_ERROR(read_u64(ctx, (uint64_t *)&schema_array->len));
    print_u64("array length:", schema_array->len);

    // read vec_type
    CHECK_ERROR(read_link(ctx, &schema_array->value));

    return parser_ok;
}

parser_error_t read_root_type_indices(parser_context_t *ctx, root_type_indices_t *root_type_indices) {
    CHECK_INPUT(root_type_indices);
    CHECK_INPUT(ctx);

    // read length
    CHECK_ERROR(read_u32(ctx, &root_type_indices->qty));
    print_u32("root_type_indices.qty:", root_type_indices->qty);
    for (uint32_t i = 0; i < root_type_indices->qty; i++) {
        CHECK_ERROR(read_u64(ctx, &root_type_indices->indices[i]));
        print_u8("root_indice", i);
        print_u64("root_type:", root_type_indices->indices[i]);
    }

    return parser_ok;
}

parser_error_t read_chain_data(parser_context_t *ctx, chain_data_t *chain_data) {
    CHECK_INPUT(chain_data);
    CHECK_INPUT(ctx);

    // read chain_id
    CHECK_ERROR(read_u64(ctx, &chain_data->chain_id));
    print_u64("chain_data.chain_id:", chain_data->chain_id);

    // read chain_name
    CHECK_ERROR(read_u32(ctx, (uint32_t *)&chain_data->chain_name.len));
    if (chain_data->chain_name.len > 0) {
        chain_data->chain_name.ptr = ctx->buffer.ptr + ctx->offset;
        CTX_CHECK_AND_ADVANCE(ctx, chain_data->chain_name.len);
    }
    print_buffer(&chain_data->chain_name, "chain_name");
    print_buffer_str(&chain_data->chain_name, "chain_name_str");

    // read gas_token_decimals
    CHECK_ERROR(read_u8(ctx, &chain_data->gas_token_decimals));
    print_u8("chain_data.gas_token_decimals:", chain_data->gas_token_decimals);

    return parser_ok;
}

parser_error_t read_schema_type(parser_context_t *ctx, uint8_t type) {
    CHECK_INPUT(ctx);

    switch (type) {
        case LINKING_SCHEME_ENUM:
            print_string("READING ENUM");
            schema_enum_t enum_type = {0};
            CHECK_ERROR(read_enum(ctx, &enum_type));
            print_string("READING ENUM DONE\n");
            break;
        case LINKING_SCHEME_STRUCT:
            print_string("READING STRUCT");
            schema_struct_t struct_type = {0};
            CHECK_ERROR(read_struct(ctx, &struct_type));
            print_string("READING STRUCT DONE\n");
            break;
        case LINKING_SCHEME_TUPLE:
            print_string("READING TUPLE");
            schema_tuple_t tuple_type = {0};
            CHECK_ERROR(read_tuple(ctx, &tuple_type));
            print_string("READING TUPLE DONE\n");
            break;
        case LINKING_SCHEME_OPTION:
            print_string("READING OPTION");
            link_t option_type = {0};
            CHECK_ERROR(read_link(ctx, &option_type));
            print_string("READING OPTION DONE\n");
            break;
        case LINKING_SCHEME_INTEGER:
            print_string("READING INTEGER");
            primitive_integer_t integer_type = {0};
            CHECK_ERROR(read_primitive_integer(ctx, &integer_type));
            print_string("READING INTEGER DONE\n");
            break;
        case LINKING_SCHEME_BYTE_ARRAY:
            print_string("READING BYTE ARRAY");
            primitive_byte_array_t byte_array_type = {0};
            CHECK_ERROR(read_primitive_byte_array(ctx, &byte_array_type));
            print_string("READING BYTE ARRAY DONE\n");
            break;
        case LINKING_SCHEME_FLOAT32:
            print_string("READING FLOAT32");
            break;
        case LINKING_SCHEME_FLOAT64:
            print_string("READING FLOAT64");
            break;
        case LINKING_SCHEME_STRING:
            print_string("READING STRING");
            break;
        case LINKING_SCHEME_BOOLEAN:
            print_string("READING BOOLEAN");
            break;
        case LINKING_SCHEME_SKIP:
            print_string("READING SKIP");
            uint64_t skip_type = 0;
            CHECK_ERROR(read_u64(ctx, &skip_type));
            print_u64("skip_type:", skip_type);
            print_string("READING SKIP DONE\n");
            break;
        case LINKING_SCHEME_BYTE_VEC:
            print_string("READING BYTE VEC");
            byte_display_t byte_vec_type = {0};
            CHECK_ERROR(read_byte_display(ctx, &byte_vec_type));
            print_string("READING BYTE VEC DONE\n");
            break;
        case LINKING_SCHEME_ARRAY:
            print_string("READING ARRAY");
            schema_array_t array_type = {0};
            CHECK_ERROR(read_array(ctx, &array_type));
            print_string("READING ARRAY DONE\n");
            break;
        case LINKING_SCHEME_VEC:
            print_string("READING VEC");
            link_t vec_type = {0};
            CHECK_ERROR(read_link(ctx, &vec_type));
            print_string("READING VEC DONE\n");
            break;
        case LINKING_SCHEME_MAP:
            print_string("READING MAP");
            link_t key = {0};
            link_t value = {0};
            CHECK_ERROR(read_link(ctx, &key));
            CHECK_ERROR(read_link(ctx, &value));
            print_string("READING MAP DONE\n");
            break;
        default:
            print_u8("UNKNOWN TYPE:", type);
            return parser_no_data;
    }

    return parser_ok;
}

parser_error_t get_enum_runtime_call_link_index(schema_enum_t *schema_enum, uint32_t field_index[], uint16_t *qty, uint16_t max_indexes) {
    CHECK_INPUT(schema_enum);
    CHECK_INPUT(field_index);
    CHECK_INPUT(qty);
    
    for (uint32_t i = 0; i < schema_enum->variants_qty; i++) {
        if (schema_enum->variants[i].discriminant == RUNTIME_CALL_BANK) {
            if (schema_enum->variants[i].value.tag == LINK_BY_INDEX) {
                field_index[(*qty)++] = schema_enum->variants[i].value.data.by_index;
            }
        }
    }

    return (*qty >= max_indexes) ? parser_unexpected_error : parser_ok;
}   

parser_error_t get_enum_link_index(schema_enum_t *schema_enum, uint64_t field_index[], uint16_t *qty, uint16_t max_indexes) {
    CHECK_INPUT(schema_enum);
    CHECK_INPUT(field_index);
    CHECK_INPUT(qty);
    
    for (uint32_t i = 0; i < schema_enum->variants_qty; i++) {
        if (schema_enum->variants[i].value.tag == LINK_BY_INDEX) {
            field_index[(*qty)++] = schema_enum->variants[i].value.data.by_index;
        }
    }

    return (*qty >= max_indexes) ? parser_unexpected_error : parser_ok;
}   

parser_error_t get_struct_link_index(schema_struct_t *schema_struct, uint32_t field_index[], uint16_t *qty, uint16_t max_indexes) {
    CHECK_INPUT(schema_struct);
    CHECK_INPUT(field_index);
    CHECK_INPUT(qty);

    named_field_t *field = schema_struct->fields;
    named_field_t *end = field + schema_struct->fields_qty;
    
    while (field < end && *qty < max_indexes) {
        if (field->value.tag == LINK_BY_INDEX) {
            field_index[(*qty)++] = field->value.data.by_index;
        }
        field++;
    }

    return (*qty >= max_indexes) ? parser_unexpected_error : parser_ok;
}

parser_error_t get_tuple_link_index(schema_tuple_t *schema_tuple, uint32_t field_index[], uint16_t *qty, uint16_t max_indexes) {
    CHECK_INPUT(schema_tuple);
    CHECK_INPUT(field_index);
    CHECK_INPUT(qty);
    
    for (uint32_t i = 0; i < schema_tuple->fields_qty; i++) {
        if (schema_tuple->fields[i].value.tag == LINK_BY_INDEX) {
            field_index[(*qty)++] = schema_tuple->fields[i].value.data.by_index;
        }
    }

    return (*qty >= max_indexes) ? parser_unexpected_error : parser_ok; 
}

parser_error_t get_schema_indexes_recursive(parser_tx_t *txObj, uint32_t index, uint32_t runtime_call_index, uint32_t field_index[], uint16_t *qty, uint8_t *visited, uint16_t max_indexes) {
    CHECK_INPUT(txObj);
    CHECK_INPUT(field_index);
    CHECK_INPUT(qty);
    CHECK_INPUT(visited);

    // Check array bounds and visited status
    if (index >= MAX_SCHEMES_QTY || *qty >= max_indexes) {
        return parser_unexpected_error;
    }

    // Check if we've already visited this index
    if (visited[index]) {
        return parser_ok;
    }
    visited[index] = 1;

    // Get linking scheme data
    linking_scheme_t *scheme = &txObj->schema.types.schemes[index];
    print_u32("SEARCHING index:", index);
    print_u8("scheme->type:", scheme->type);

    uint16_t current_qty = *qty;
    switch (scheme->type) {
        case LINKING_SCHEME_ENUM: {
            schema_enum_t enum_type = {0};
            CHECK_ERROR(read_enum(&scheme->data, &enum_type));
            if (index == runtime_call_index) {
                CHECK_ERROR(get_enum_runtime_call_link_index(&enum_type, field_index, qty, max_indexes));
            } else if (index == 4) {
                CHECK_ERROR(get_enum_link_index(&enum_type, field_index, qty, max_indexes));
            }
            break;
        }
        case LINKING_SCHEME_STRUCT: {
            schema_struct_t struct_type = {0};
            CHECK_ERROR(read_struct(&scheme->data, &struct_type));
            CHECK_ERROR(get_struct_link_index(&struct_type, field_index, qty, max_indexes));
            break;
        }
        case LINKING_SCHEME_TUPLE: {
            schema_tuple_t tuple_type = {0};
            CHECK_ERROR(read_tuple(&scheme->data, &tuple_type));
            CHECK_ERROR(get_tuple_link_index(&tuple_type, field_index, qty, max_indexes));
            break;
        }

        default:
            break;
    }

    for (uint16_t i = current_qty; i < *qty && i < max_indexes; i++) {
        print_u32("Recursive Local field_index:", field_index[i]);
    }

    for (uint16_t i = current_qty; i < *qty && i < max_indexes; i++) {
        CHECK_ERROR(get_schema_indexes_recursive(txObj, field_index[i], runtime_call_index, field_index, qty, visited, max_indexes));
    }

    return parser_ok;
}

parser_error_t get_schema_unsigned_transaction_index(parser_tx_t *txObj, uint8_t *indices, uint32_t qty) {
    CHECK_INPUT(indices);
    CHECK_INPUT(txObj);

    // Get root index
    if (txObj->schema.root_type_indices.qty <= ROLLUP_ROOTS_UNSIGNED_TRANSACTION || txObj->schema.root_type_indices.qty <= ROLLUP_ROOTS_RUNTIME_CALL) {
        return parser_root_type_indices_overflow;
    }

    uint32_t root_index = txObj->schema.root_type_indices.indices[ROLLUP_ROOTS_UNSIGNED_TRANSACTION];
    uint32_t runtime_call_index = txObj->schema.root_type_indices.indices[ROLLUP_ROOTS_RUNTIME_CALL];

    if (root_index > txObj->schema.types.qty || runtime_call_index > txObj->schema.types.qty) {
        return parser_scheme_indices_overflow;
    }
    print_u32("root_index:", root_index);
    print_u32("runtime_call_index:", runtime_call_index);

    uint32_t field_index[MAX_FIELDS_QTY] = {0};
    uint16_t field_count = 0;
    uint8_t visited[MAX_SCHEMES_QTY] = {0};

    CHECK_ERROR(get_schema_indexes_recursive(txObj, root_index, runtime_call_index, field_index, &field_count, visited, MAX_FIELDS_QTY));

    for (uint32_t i = 0; i < field_count; i++) {
        print_u32("field_index:", field_index[i]);
    }

    return parser_ok;
}

parser_error_t get_call_message_index(parser_tx_t *txObj) {
    CHECK_INPUT(txObj);

    // Get root index
    if (txObj->schema.root_type_indices.qty <= ROLLUP_ROOTS_RUNTIME_CALL) {
        return parser_root_type_indices_overflow;
    }

    uint32_t runtime_call_index = txObj->schema.root_type_indices.indices[ROLLUP_ROOTS_RUNTIME_CALL];

    if (runtime_call_index > txObj->schema.types.qty) {
        return parser_scheme_indices_overflow;
    }

    uint16_t field_count = 0;
    schema_enum_t enum_type = {0};
    CHECK_ERROR(read_enum(&txObj->schema.types.schemes[runtime_call_index].data, &enum_type));
    txObj->schema.types.schemes[runtime_call_index].data.offset = 0;
    CHECK_ERROR(get_enum_link_index(&enum_type, &txObj->schema.call_message_index, &field_count, MAX_FIELDS_QTY));

    return parser_ok;
}

parser_error_t metadata_read(parser_context_t *ctx, parser_tx_t *txObj) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(txObj);

    CHECK_ERROR(read_u32(ctx, &txObj->schema.types.qty));
    print_u32("types.qty:", txObj->schema.types.qty);
    if (txObj->schema.types.qty > MAX_SCHEMES_QTY) {
        return parser_too_many_schemes;
    }

    for (uint32_t i = 0; i < txObj->schema.types.qty; i++) {
        // read type
        CHECK_ERROR(read_u8(ctx, (uint8_t *)&txObj->schema.types.schemes[i].type));
        uint16_t offset_mem = ctx->offset;
        const uint8_t *ptr_mem = ctx->buffer.ptr + ctx->offset;
        CHECK_ERROR(read_schema_type(ctx, txObj->schema.types.schemes[i].type));
        txObj->schema.types.schemes[i].data.buffer.ptr = ptr_mem;
        txObj->schema.types.schemes[i].data.buffer.len = ctx->offset - offset_mem;
    }

    // read root_type_indices
    CHECK_ERROR(read_root_type_indices(ctx, &txObj->schema.root_type_indices));

    // read chain_data
    CHECK_ERROR(read_chain_data(ctx, &txObj->schema.chain_data));

    // read extra_metadata_hash
    txObj->schema.extra_metadata_hash.len = 32;
    txObj->schema.extra_metadata_hash.ptr = ctx->buffer.ptr + ctx->offset;
    CTX_CHECK_AND_ADVANCE(ctx, txObj->schema.extra_metadata_hash.len);
    print_buffer(&txObj->schema.extra_metadata_hash, "extra_metadata_hash");

    // TODO: check that we have consumed all data
    if (ctx->offset != ctx->buffer.len) {
        print_string("Failed to parse metadata\n");
        return parser_unexpected_error;
    } else {
        print_string("Successfully parsed metadata\n");
    }

    CHECK_ERROR(get_call_message_index(txObj));

    print_string("SEARCHING UNSIGNED TRANSACTION INDEX:");
    uint8_t indices[1] = {0};
    CHECK_ERROR(get_schema_unsigned_transaction_index(txObj, indices, 1));

    return parser_ok;
}
