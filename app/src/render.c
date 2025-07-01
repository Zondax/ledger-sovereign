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

#include "app_mode.h"
#include "bech32.h"
#include "borsh.h"
#include "schema_helper.h"
#include "schema_reader.h"
#include "schema_txn_parser.h"
#include "zxerror.h"
#include "zxformat.h"

#define NONE_STRING "None"

static parser_error_t uint128_to_str(uint64_t high, uint64_t low, char *out, uint16_t out_len) {
    CHECK_INPUT(out);
    if (out_len < U128_STR_MAX_LEN) return parser_value_out_of_range;

    MEMZERO(out, out_len);
    char *p = out;

    if (high == 0 && low == 0) {
        *(p++) = '0';
        return parser_ok;
    }

    uint64_t temp_high = high;
    uint64_t temp_low = low;

    while (temp_high != 0 || temp_low != 0) {
        if (p - out >= (out_len - 1)) return parser_value_out_of_range;

        uint64_t quotient_high = 0;
        uint64_t quotient_low = 0;
        uint64_t remainder = 0;
        uint64_t current;

        current = temp_high;
        quotient_high = current / 10;
        remainder = current % 10;

        current = (remainder << 32) | (temp_low >> 32);
        uint64_t q = current / 10;
        remainder = current % 10;
        quotient_low = (q << 32);

        current = (remainder << 32) | (temp_low & 0xFFFFFFFF);
        q = current / 10;
        remainder = current % 10;
        quotient_low |= q;

        *(p++) = (char)('0' + remainder);
        temp_high = quotient_high;
        temp_low = quotient_low;
    }

    while (p > out) {
        p--;
        char z = *out;
        *out = *p;
        *p = z;
        out++;
    }

    return parser_ok;
}

static parser_error_t render_number(uint64_t high, uint64_t low, uint8_t decimalPlaces, const char *postfix,
                                    const char *prefix, char *outValue, uint16_t outValueLen) {
    CHECK_INPUT(outValue);
    if (outValueLen < U128_STR_MAX_LEN) return parser_value_out_of_range;

    CHECK_ERROR(uint128_to_str(high, low, outValue, outValueLen));

    if (intstr_to_fpstr_inplace(outValue, outValueLen, decimalPlaces) == 0) {
        return parser_unexpected_value;
    }

    if (z_str3join(outValue, outValueLen, prefix, postfix) != zxerr_ok) {
        return parser_unexpected_buffer_end;
    }

    number_inplace_trimming(outValue, 1);

    return parser_ok;
}

static parser_error_t find_name_registry(parser_tx_t *txObj, bytes_t *name, bytes_t *input_token, char *outValue,
                                         uint16_t outValueLen) {
    CHECK_INPUT(txObj);
    CHECK_INPUT(name);
    CHECK_INPUT(input_token);
    CHECK_INPUT(outValue);

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
                    MEMZERO(outValue, outValueLen);
                    MEMCPY(outValue, registry.name.ptr, registry.name.len);
                    return parser_ok;
                }
            }
        }
    }

    return parser_name_registry_not_found;
}

parser_error_t render_fixed_point(parser_context_t *ctx, fixed_point_display_t display, uint128_t value, char *outValue,
                                  uint16_t outValueLen) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(outValue);

    MEMZERO(outValue, outValueLen);

    switch (display.type) {
        case FIXED_POINT_DISPLAY_DECIMALS:
            // TODO: Implement me
            print_string("render_fixed_point IMPLEMENT ME 0");
            return parser_unexpected_type;
        case FIXED_POINT_DISPLAY_FROM_SIBLING_FIELD:
            // get offset
            if (ctx->offset + display.from_sibling_field.byte_offset >= ctx->buffer.len) {
                return parser_unexpected_buffer_end;
            }
            uint8_t offset = ctx->buffer.ptr[ctx->offset + display.from_sibling_field.byte_offset];

            CHECK_ERROR(render_number(value.hi, value.lo, offset, "", "", outValue, outValueLen));

            return parser_ok;
        default:
            return parser_unexpected_type;
    }
}

parser_error_t render_primitive_integer(parser_context_t *ctx, integer_display_t display, uint128_t value, char *outValue,
                                        uint16_t outValueLen) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(outValue);
    MEMZERO(outValue, outValueLen);

    switch (display.type) {
        case INTEGER_DISPLAY_HEX:
            // TODO: Implement me
            print_string("render_primitive_integer IMPLEMENT ME 0");
            return parser_unexpected_type;
        case INTEGER_DISPLAY_DECIMAL:
            CHECK_ERROR(render_number(value.hi, value.lo, 0, "", "", outValue, outValueLen));
            break;
        case INTEGER_DISPLAY_FIXED_POINT:
            CHECK_ERROR(render_fixed_point(ctx, display.fixed_point, value, outValue, outValueLen));
            break;
        default:
            return parser_unexpected_type;
    }

    return parser_ok;
}

parser_error_t render_integer(parser_context_t *ctx, primitive_integer_t *primitive, char *outValue, uint16_t outValueLen) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(primitive);

    uint128_t value = {0};
    switch (primitive->type) {
        case INTEGER_I8:
        case INTEGER_U8: {
            uint8_t *ptr = (uint8_t *)&value;
            CHECK_ERROR(read_u8(ctx, ptr));
            break;
        }
        case INTEGER_I16:
        case INTEGER_U16: {
            uint16_t *ptr = (uint16_t *)&value;
            CHECK_ERROR(read_u16(ctx, ptr));
            break;
        }
        case INTEGER_I32:
        case INTEGER_U32: {
            uint32_t *ptr = (uint32_t *)&value;
            CHECK_ERROR(read_u32(ctx, ptr));
            break;
        }
        case INTEGER_I64:
        case INTEGER_U64: {
            CHECK_ERROR(read_u64(ctx, &value.lo));
            break;
        }
        case INTEGER_I128:
        case INTEGER_U128: {
            CHECK_ERROR(read_u64(ctx, &value.lo));
            CHECK_ERROR(read_u64(ctx, &value.hi));
            break;
        }
        default:
            return parser_unexpected_type;
    }

    CHECK_ERROR(render_primitive_integer(ctx, primitive->display, value, outValue, outValueLen));

    return parser_ok;
}

parser_error_t render_byte_array(parser_context_t *ctx, parser_tx_t *txObj, primitive_byte_array_t *byte_array,
                                 char *outValue, uint16_t outValueLen) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(txObj);
    CHECK_INPUT(byte_array);
    CHECK_INPUT(outValue);

    bytes_t array = {0};
    array.len = byte_array->len;
    array.ptr = ctx->buffer.ptr + ctx->offset;
    CTX_CHECK_AND_ADVANCE(ctx, byte_array->len)

    if (byte_array->has_name_registry) {
        // check if name registry exist
        parser_error_t err = find_name_registry(txObj, &byte_array->name_registry, &array, outValue, outValueLen);
        if (err == parser_ok || err != parser_name_registry_not_found) {
            return err;
        }
    }

    if (byte_array->len > MAX_INPUT_CHUNK) {
        return parser_ui_buffer_too_small;
    }

    MEMZERO(outValue, outValueLen);

    switch (byte_array->display.type) {
        case BYTE_DISPLAY_HEX:
            // TODO: Implement me
            print_string("render_byte_array IMPLEMENT ME 0");
            return parser_unexpected_type;
        case BYTE_DISPLAY_DECIMAL:
            // TODO: Implement me
            print_string("render_byte_array IMPLEMENT ME 1");
            return parser_unexpected_type;
        case BYTE_DISPLAY_BECH32:
            // TODO: Implement me
            print_string("render_byte_array IMPLEMENT ME 2");
            return parser_unexpected_type;
        case BYTE_DISPLAY_BECH32M: {
            char hrp[MAX_HRP_LEN + 1] = {0};
            MEMCPY(hrp, byte_array->display.bech32m.prefix.prefix.ptr, byte_array->display.bech32m.prefix.prefix.len);
            MAP_ZXERR_TO_PARSER_ERR(
                bech32EncodeFromBytes(outValue, outValueLen, hrp, array.ptr, array.len, 1, BECH32_ENCODING_BECH32M));
            break;
        }
        case BYTE_DISPLAY_BASE58:
            // TODO: Implement me
            print_string("render_byte_array IMPLEMENT ME 3");
            return parser_unexpected_type;
        default:
            return parser_unexpected_type;
    }

    return parser_ok;
}

parser_error_t render_primitive(parser_context_t *ctx, parser_tx_t *txObj, primitive_t *primitive, char *outValue,
                                uint16_t outValueLen) {
    CHECK_INPUT(ctx);
    CHECK_INPUT(txObj);
    CHECK_INPUT(outValue);

    switch (primitive->type) {
        case PRIMITIVE_INTEGER:
            CHECK_ERROR(render_integer(ctx, &primitive->integer, outValue, outValueLen));
            break;
        case PRIMITIVE_BYTE_ARRAY:
            CHECK_ERROR(render_byte_array(ctx, txObj, &primitive->byte_array, outValue, outValueLen));
            break;
        case PRIMITIVE_BYTE_VEC:
            // TODO: Implement me
            print_string("render_primitive IMPLEMENT ME 0");
            return parser_unexpected_type;
        case PRIMITIVE_FLOAT32:
            // TODO: Implement me
            print_string("render_primitive IMPLEMENT ME 1");
            return parser_unexpected_type;
        case PRIMITIVE_FLOAT64:
            // TODO: Implement me
            print_string("render_primitive IMPLEMENT ME 2");
            return parser_unexpected_type;
        case PRIMITIVE_STRING:
            MEMCPY(outValue, ctx->buffer.ptr, ctx->buffer.len);
            break;
        case PRIMITIVE_BOOLEAN:
            // TODO: Implement me
            print_string("render_primitive IMPLEMENT ME 4");
            return parser_unexpected_type;
        default:
            print_u8("Unknown type: ", primitive->type);
            return parser_unexpected_type;
    }

    print_u8("render_primitive finished: ", primitive->type);

    return parser_ok;
}
