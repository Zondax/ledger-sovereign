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

#include "schema_reader.h"

#include "borsh.h"
#include "crypto_helper.h"
#include "schema_helper.h"
#include "schema_proof.h"

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

    // read has_name_registry
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&primitive->has_name_registry));
    print_u8("has_name_registry:", primitive->has_name_registry);

    // read name_registry
    if (primitive->has_name_registry) {
        CHECK_ERROR(read_u32(ctx, (uint32_t *)&primitive->name_registry.len));
        if (primitive->name_registry.len > 0) {
            primitive->name_registry.ptr = ctx->buffer.ptr + ctx->offset;
            CTX_CHECK_AND_ADVANCE(ctx, primitive->name_registry.len);
        }
        print_buffer(&primitive->name_registry, "name_registry");
        print_buffer_str(&primitive->name_registry, "name_registry_str");
    }

    return parser_ok;
}

parser_error_t read_primitive_byte_vec(parser_context_t *ctx, primitive_byte_vec_t *primitive) {
    CHECK_INPUT(primitive);
    CHECK_INPUT(ctx);

    // read display
    CHECK_ERROR(read_byte_display(ctx, &primitive->display));

    // read has_name_registry
    CHECK_ERROR(read_u8(ctx, (uint8_t *)&primitive->has_name_registry));
    print_u8("has_name_registry:", primitive->has_name_registry);

    // read name_registry
    if (primitive->has_name_registry) {
        CHECK_ERROR(read_u32(ctx, (uint32_t *)&primitive->name_registry.len));
        if (primitive->name_registry.len > 0) {
            primitive->name_registry.ptr = ctx->buffer.ptr + ctx->offset;
            CTX_CHECK_AND_ADVANCE(ctx, primitive->name_registry.len);
        }
        print_buffer(&primitive->name_registry, "name_registry");
        print_buffer_str(&primitive->name_registry, "name_registry_str");
    }

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
            CHECK_ERROR(read_primitive_byte_vec(ctx, &primitive->byte_vec));
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

    schema_enum->enum_variants.buffer.ptr = ctx->buffer.ptr + ctx->offset;
    uint16_t offset_mem = ctx->offset;
    for (uint32_t i = 0; i < schema_enum->variants_qty; i++) {
        enum_variant_t variant = {0};
        CHECK_ERROR(read_enum_variant(ctx, &variant));
    }
    schema_enum->enum_variants.buffer.len = ctx->offset - offset_mem;

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
    schema_struct->named_fields.buffer.ptr = ctx->buffer.ptr + ctx->offset;
    uint16_t offset_mem = ctx->offset;
    for (uint32_t i = 0; i < schema_struct->fields_qty; i++) {
        named_field_t field = {0};
        CHECK_ERROR(read_named_field(ctx, &field));
    }
    schema_struct->named_fields.buffer.len = ctx->offset - offset_mem;

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

parser_error_t read_option(parser_context_t *ctx, schema_option_t *schema_option) {
    CHECK_INPUT(schema_option);
    CHECK_INPUT(ctx);

    // read value
    CHECK_ERROR(read_link(ctx, &schema_option->value));

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

    // save complete borsh data
    root_type_indices->complete_borsh_data.ptr = ctx->buffer.ptr + ctx->offset;
    uint16_t offset_mem_complete_borsh_data = ctx->offset;

    // read length
    CHECK_ERROR(read_u32(ctx, &root_type_indices->qty));
    print_u32("root_type_indices.qty:", root_type_indices->qty);

    const uint8_t *ptr_mem_indices = ctx->buffer.ptr + ctx->offset;
    uint16_t offset_mem_indices = ctx->offset;
    for (uint32_t i = 0; i < root_type_indices->qty; i++) {
        uint64_t index = 0;
        CHECK_ERROR(read_u64(ctx, &index));
        print_u8("root_indice", i);
        print_u64("root_type:", index);
    }
    root_type_indices->indices.buffer.ptr = ptr_mem_indices;
    root_type_indices->indices.buffer.len = ctx->offset - offset_mem_indices;

    root_type_indices->complete_borsh_data.len = ctx->offset - offset_mem_complete_borsh_data;
    print_buffer_u8(&root_type_indices->complete_borsh_data, "root_type_indices.complete_borsh_data");

    return parser_ok;
}

parser_error_t read_registry(parser_context_t *ctx, registry_t *registry) {
    CHECK_INPUT(registry);
    CHECK_INPUT(ctx);
    // read data
    CHECK_ERROR(read_u32(ctx, (uint32_t *)&registry->data.len));
    if (registry->data.len > 0) {
        registry->data.ptr = ctx->buffer.ptr + ctx->offset;
        CTX_CHECK_AND_ADVANCE(ctx, registry->data.len);
        print_buffer(&registry->data, "registry.data");
    }
    // read name
    CHECK_ERROR(read_u32(ctx, (uint32_t *)&registry->name.len));
    if (registry->name.len > 0) {
        registry->name.ptr = ctx->buffer.ptr + ctx->offset;
        CTX_CHECK_AND_ADVANCE(ctx, registry->name.len);
        print_buffer(&registry->name, "registry.name");
        print_buffer_str(&registry->name, "registry.name_str");
    }

    return parser_ok;
}

parser_error_t read_name_registry(parser_context_t *ctx, name_registry_t *name_registry) {
    CHECK_INPUT(name_registry);
    CHECK_INPUT(ctx);

    // read name
    CHECK_ERROR(read_u32(ctx, (uint32_t *)&name_registry->name.len));
    if (name_registry->name.len > 0) {
        name_registry->name.ptr = ctx->buffer.ptr + ctx->offset;
        CTX_CHECK_AND_ADVANCE(ctx, name_registry->name.len);
        print_buffer(&name_registry->name, "name_registry.name");
        print_buffer_str(&name_registry->name, "name_registry.name_str");
    }

    // read qty
    CHECK_ERROR(read_u32(ctx, &name_registry->qty));
    print_u32("name_registry.qty:", name_registry->qty);

    // read registries
    name_registry->registry.buffer.ptr = ctx->buffer.ptr + ctx->offset;
    uint16_t offset_mem = ctx->offset;
    for (uint32_t i = 0; i < name_registry->qty; i++) {
        registry_t registry = {0};
        CHECK_ERROR(read_registry(ctx, &registry));
    }
    name_registry->registry.buffer.len = ctx->offset - offset_mem;
    return parser_ok;
}

parser_error_t read_name_registries(parser_context_t *ctx, name_registries_t *name_registries) {
    CHECK_INPUT(name_registries);
    CHECK_INPUT(ctx);

    // read qty
    CHECK_ERROR(read_u32(ctx, &name_registries->qty));
    print_u32("name_registries.qty:", name_registries->qty);

    // read registries
    name_registries->vec_registries.buffer.ptr = ctx->buffer.ptr + ctx->offset;
    uint16_t offset_mem = ctx->offset;
    for (uint32_t i = 0; i < name_registries->qty; i++) {
        name_registry_t name_registry = {0};
        CHECK_ERROR(read_name_registry(ctx, &name_registry));
    }
    name_registries->vec_registries.buffer.len = ctx->offset - offset_mem;
    return parser_ok;
}

parser_error_t read_chain_data(parser_context_t *ctx, chain_data_t *chain_data) {
    CHECK_INPUT(chain_data);
    CHECK_INPUT(ctx);

    // save complete borsh data
    chain_data->complete_borsh_data.ptr = ctx->buffer.ptr + ctx->offset;
    uint16_t offset_mem = ctx->offset;

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

    // read name_registries
    CHECK_ERROR(read_name_registries(ctx, &chain_data->name_registries));

    chain_data->complete_borsh_data.len = ctx->offset - offset_mem;

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

parser_error_t metadata_read(parser_context_t *ctx, parser_tx_t *txObj) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(txObj);

    uint32_t qty = 0;
    CHECK_ERROR(read_u32(ctx, &qty));
    print_u32("types.qty:", qty);

    for (uint32_t i = 0; i < qty; i++) {
        // read type
        uint8_t type = 0;
        CHECK_ERROR(read_u8(ctx, (uint8_t *)&type));
        CHECK_ERROR(read_schema_type(ctx, type));
    }

    // read root_type_indices
    CHECK_ERROR(read_root_type_indices(ctx, &txObj->schema.root_type_indices));

    // read chain_data
    CHECK_ERROR(read_chain_data(ctx, &txObj->schema.chain_data));

    // read extra_metadata_hash
    txObj->schema.extra_metadata_hash.ptr = ctx->buffer.ptr + ctx->offset;
    txObj->schema.extra_metadata_hash.len = CX_SHA256_SIZE;
    CTX_CHECK_AND_ADVANCE(ctx, CX_SHA256_SIZE);
    print_buffer(&txObj->schema.extra_metadata_hash, "extra_metadata_hash");

    // TODO: check that we have consumed all data
    if (ctx->offset != ctx->buffer.len) {
        print_string("Failed to parse metadata\n");
        return parser_unexpected_error;
    } else {
        print_string("Successfully parsed metadata\n");
    }

    return parser_ok;
}

parser_error_t compute_internal_data_hash(parser_tx_t *txObj, uint8_t *internal_data_hash) {
    CHECK_INPUT(txObj);
    CHECK_INPUT(internal_data_hash);

    crypto_sha256_init();
    crypto_sha256_update(txObj->schema.root_type_indices.complete_borsh_data.ptr,
                         txObj->schema.root_type_indices.complete_borsh_data.len);
    crypto_sha256_update(txObj->schema.chain_data.complete_borsh_data.ptr, txObj->schema.chain_data.complete_borsh_data.len);
    crypto_sha256_final(internal_data_hash);

    return parser_ok;
}

parser_error_t compute_chain_hash(parser_tx_t *txObj) {
    CHECK_INPUT(txObj);

    uint8_t internal_data_hash[CX_SHA256_SIZE] = {0};
    uint8_t computed_chain_hash[CX_SHA256_SIZE] = {0};
    CHECK_ERROR(compute_internal_data_hash(txObj, internal_data_hash));

    crypto_sha256_init();
    crypto_sha256_update(txObj->merkle_proofs.root_hash.ptr, txObj->merkle_proofs.root_hash.len);
    crypto_sha256_update(internal_data_hash, CX_SHA256_SIZE);
    crypto_sha256_update(txObj->schema.extra_metadata_hash.ptr, txObj->schema.extra_metadata_hash.len);
    crypto_sha256_final(computed_chain_hash);

    if (MEMCMP(computed_chain_hash, txObj->schema.chain_hash.ptr, CX_SHA256_SIZE) != 0) {
        return parser_unexpected_chain_hash;
    }
    return parser_ok;
}

// | borsh(leaves_data) | borsh(indices_leaves) | borsh(lemmas) | borsh(tree_size) | borsh(root_hash) | borsh(root_indexes) |
// borsh(chain_data)
parser_error_t merkle_proofs_read(parser_context_t *ctx, parser_tx_t *txObj) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(txObj);

    // read leaf data
    CHECK_ERROR(read_u32(ctx, &txObj->merkle_proofs.leaves.entries));
    print_u32("leaves.qty:", txObj->merkle_proofs.leaves.entries);
    const uint8_t *ptr_mem = ctx->buffer.ptr + ctx->offset;
    uint16_t offset_mem = ctx->offset;
    for (uint32_t i = 0; i < txObj->merkle_proofs.leaves.entries; i++) {
        uint32_t length = 0;
        CHECK_ERROR(read_u32(ctx, &length));
        print_u32("length:", length);
        uint8_t schema_type = 0;
        CHECK_ERROR(read_u8(ctx, (uint8_t *)&schema_type));
        print_u8("schema.type:", schema_type);
        CHECK_ERROR(read_schema_type(ctx, schema_type));
    }
    txObj->merkle_proofs.leaves.data.buffer.ptr = ptr_mem;
    txObj->merkle_proofs.leaves.data.buffer.len = ctx->offset - offset_mem;
    print_buffer(&txObj->merkle_proofs.leaves.data.buffer, "leaves data");

    // read indices
    CHECK_ERROR(read_u32(ctx, &txObj->merkle_proofs.indices.entries));
    print_u32("indices.qty:", txObj->merkle_proofs.indices.entries);
    if (txObj->merkle_proofs.indices.entries != txObj->merkle_proofs.leaves.entries) {
        return parser_unexpected_error;
    }
    const uint8_t *ptr_mem_indices = ctx->buffer.ptr + ctx->offset;
    uint16_t offset_mem_indices = ctx->offset;
    for (uint32_t i = 0; i < txObj->merkle_proofs.indices.entries; i++) {
        uint64_t index = 0;
        CHECK_ERROR(read_u64(ctx, &index));
        print_u64("index:", index);
    }
    txObj->merkle_proofs.indices.indices.buffer.ptr = ptr_mem_indices;
    txObj->merkle_proofs.indices.indices.buffer.len = ctx->offset - offset_mem_indices;
    print_buffer(&txObj->merkle_proofs.indices.indices.buffer, "indices");

    // read lemmas
    CHECK_ERROR(read_u32(ctx, &txObj->merkle_proofs.lemmas.entries));
    print_u32("lemmas.qty:", txObj->merkle_proofs.lemmas.entries);
    txObj->merkle_proofs.lemmas.data.buffer.ptr = ctx->buffer.ptr + ctx->offset;
    txObj->merkle_proofs.lemmas.data.buffer.len = txObj->merkle_proofs.lemmas.entries * 32;
    CTX_CHECK_AND_ADVANCE(ctx, txObj->merkle_proofs.lemmas.data.buffer.len);
    print_buffer(&txObj->merkle_proofs.lemmas.data.buffer, "lemmas");

    // read tree_size
    CHECK_ERROR(read_u64(ctx, &txObj->merkle_proofs.tree_size));
    print_u64("tree_size:", txObj->merkle_proofs.tree_size);

    // read root_hash
    txObj->merkle_proofs.root_hash.ptr = ctx->buffer.ptr + ctx->offset;
    txObj->merkle_proofs.root_hash.len = 32;
    CTX_CHECK_AND_ADVANCE(ctx, txObj->merkle_proofs.root_hash.len);
    print_buffer(&txObj->merkle_proofs.root_hash, "root_hash");

    // read root_type_indices
    CHECK_ERROR(read_root_type_indices(ctx, &txObj->schema.root_type_indices));

    // read chain_data
    CHECK_ERROR(read_chain_data(ctx, &txObj->schema.chain_data));

    // read extra_metadata_hash
    txObj->schema.extra_metadata_hash.ptr = ctx->buffer.ptr + ctx->offset;
    txObj->schema.extra_metadata_hash.len = CX_SHA256_SIZE;
    CTX_CHECK_AND_ADVANCE(ctx, CX_SHA256_SIZE);
    print_buffer(&txObj->schema.extra_metadata_hash, "extra_metadata_hash");

    // read chain hash
    txObj->schema.chain_hash.ptr = ctx->buffer.ptr + ctx->offset;
    txObj->schema.chain_hash.len = CX_SHA256_SIZE;
    CTX_CHECK_AND_ADVANCE(ctx, CX_SHA256_SIZE);

    CHECK_ERROR(verify_merkle_proofs(&txObj->merkle_proofs));

    // compute chain hash
    CHECK_ERROR(compute_chain_hash(txObj));

    return parser_ok;
}
