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

#include "schema_helper.h"

#include "borsh.h"

// parser_error_t get_enum_runtime_call_link_index(schema_enum_t *schema_enum, uint32_t field_index[], uint16_t *qty,
// uint16_t max_indexes) {
//     CHECK_INPUT(schema_enum);
//     CHECK_INPUT(field_index);
//     CHECK_INPUT(qty);

//     for (uint32_t i = 0; i < schema_enum->variants_qty; i++) {
//         if (schema_enum->variants[i].discriminant == RUNTIME_CALL_BANK) {
//             if (schema_enum->variants[i].value.tag == LINK_BY_INDEX) {
//                 field_index[(*qty)++] = schema_enum->variants[i].value.data.by_index;
//             }
//         }
//     }

//     return (*qty >= max_indexes) ? parser_unexpected_error : parser_ok;
// }

// parser_error_t get_enum_transfer_index(schema_enum_t *schema_enum, uint32_t field_index[], uint16_t *qty, uint16_t
// max_indexes) {
//     CHECK_INPUT(schema_enum);
//     CHECK_INPUT(field_index);
//     CHECK_INPUT(qty);

//     for (uint32_t i = 0; i < schema_enum->variants_qty; i++) {
//         if (schema_enum->variants[i].discriminant == BANK_CALL_MESSAGE_TRANSFER) {
//             if (schema_enum->variants[i].value.tag == LINK_BY_INDEX) {
//                 field_index[(*qty)++] = schema_enum->variants[i].value.data.by_index;
//             }
//         }
//     }

//     return (*qty >= max_indexes) ? parser_unexpected_error : parser_ok;
// }

// parser_error_t get_enum_link_index(schema_enum_t *schema_enum, uint64_t field_index[], uint16_t *qty, uint16_t
// max_indexes) {
//     CHECK_INPUT(schema_enum);
//     CHECK_INPUT(field_index);
//     CHECK_INPUT(qty);

//     for (uint32_t i = 0; i < schema_enum->variants_qty; i++) {
//         if (schema_enum->variants[i].value.tag == LINK_BY_INDEX) {
//             field_index[(*qty)++] = schema_enum->variants[i].value.data.by_index;
//         }
//     }

//     return (*qty >= max_indexes) ? parser_unexpected_error : parser_ok;
// }

// parser_error_t get_schema_indexes_recursive(parser_tx_t *txObj, uint32_t index, uint32_t runtime_call_index, uint32_t
// field_index[], uint16_t *qty, uint8_t *visited, uint16_t max_indexes) {
//     CHECK_INPUT(txObj);
//     CHECK_INPUT(field_index);
//     CHECK_INPUT(qty);
//     CHECK_INPUT(visited);

//     // Check array bounds and visited status
//     if (index >= MAX_SCHEMES_QTY || *qty >= max_indexes) {
//         return parser_unexpected_error;
//     }

//     // Check if we've already visited this index
//     if (visited[index]) {
//         return parser_ok;
//     }
//     visited[index] = 1;

//     // Get linking scheme data
//     linking_scheme_t *scheme = &txObj->schema.types.schemes[index];
//     print_u32("SEARCHING index:", index);
//     print_u8("scheme->type:", scheme->type);

//     uint16_t current_qty = *qty;
//     switch (scheme->type) {
//         case LINKING_SCHEME_ENUM: {
//             schema_enum_t enum_type = {0};
//             CHECK_ERROR(read_enum(&scheme->data, &enum_type));
//             if (index == runtime_call_index) {
//                 CHECK_ERROR(get_enum_runtime_call_link_index(&enum_type, field_index, qty, max_indexes));
//             } else if (index == 4) {
//                 CHECK_ERROR(get_enum_link_index(&enum_type, field_index, qty, max_indexes));
//             } else if (index == 5) {
//                 CHECK_ERROR(get_enum_transfer_index(&enum_type, field_index, qty, max_indexes));
//             } else {
//                 CHECK_ERROR(get_enum_link_index(&enum_type, field_index, qty, max_indexes));
//             }
//             break;
//         }
//         case LINKING_SCHEME_STRUCT: {
//             schema_struct_t struct_type = {0};
//             CHECK_ERROR(read_struct(&scheme->data, &struct_type));
//             CHECK_ERROR(get_struct_link_index(&struct_type, field_index, qty, max_indexes));
//             break;
//         }
//         case LINKING_SCHEME_TUPLE: {
//             schema_tuple_t tuple_type = {0};
//             CHECK_ERROR(read_tuple(&scheme->data, &tuple_type));
//             CHECK_ERROR(get_tuple_link_index(&tuple_type, field_index, qty, max_indexes));
//             break;
//         }

//         default:
//             break;
//     }

//     for (uint16_t i = current_qty; i < *qty && i < max_indexes; i++) {
//         print_u32("Recursive Local field_index:", field_index[i]);
//     }

//     for (uint16_t i = current_qty; i < *qty && i < max_indexes; i++) {
//         CHECK_ERROR(get_schema_indexes_recursive(txObj, field_index[i], runtime_call_index, field_index, qty, visited,
//         max_indexes));
//     }

//     return parser_ok;
// }

// parser_error_t get_schema_unsigned_transaction_index(parser_tx_t *txObj, uint8_t *indices, uint32_t qty) {
//     CHECK_INPUT(indices);
//     CHECK_INPUT(txObj);

//     // Get root index
//     if (txObj->merkle_proofs.root_type_indices.qty <= ROLLUP_ROOTS_UNSIGNED_TRANSACTION ||
//         txObj->merkle_proofs.root_type_indices.qty <= ROLLUP_ROOTS_RUNTIME_CALL) {
//         return parser_root_type_indices_overflow;
//     }

//     uint64_t root_index;
//     txObj->merkle_proofs.root_type_indices.indices.offset = OFFSET_U64 * ROLLUP_ROOTS_UNSIGNED_TRANSACTION;
//     CHECK_ERROR(read_u64(&txObj->merkle_proofs.root_type_indices.indices, &root_index));
//     txObj->merkle_proofs.root_type_indices.indices.offset = 0;

//     print_u64("root_index:", root_index);

//     return parser_ok;
// }

// parser_error_t get_call_message_index(parser_tx_t *txObj) {
//     CHECK_INPUT(txObj);

//     // Get root index
//     if (txObj->schema.root_type_indices.qty <= ROLLUP_ROOTS_RUNTIME_CALL) {
//         return parser_root_type_indices_overflow;
//     }

//     uint32_t runtime_call_index = txObj->schema.root_type_indices.indices[ROLLUP_ROOTS_RUNTIME_CALL];

//     if (runtime_call_index > txObj->schema.types.qty) {
//         return parser_scheme_indices_overflow;
//     }

//     uint16_t field_count = 0;
//     schema_enum_t enum_type = {0};
//     CHECK_ERROR(read_enum(&txObj->schema.types.schemes[runtime_call_index].data, &enum_type));
//     txObj->schema.types.schemes[runtime_call_index].data.offset = 0;
//     CHECK_ERROR(get_enum_link_index(&enum_type, &txObj->schema.call_message_index, &field_count, MAX_FIELDS_QTY));

//     return parser_ok;
// }

/**
 * @brief Find an index in the indices array.
 *
 * @param index The index to find.
 * @param indices The indices array.
 * @return bool True if the index is found, false otherwise.
 */
bool schema_find_index(uint64_t index_leaf, merkle_leaves_indices_t *indices, uint64_t *index_vec) {
    CHECK_INPUT(indices);
    CHECK_INPUT(index_vec);

    *index_vec = 0;
    for (uint64_t i = 0; i < indices->entries; i++) {
        uint64_t index_tmp = 0;
        CHECK_ERROR(read_u64(&indices->indices, &index_tmp));
        if (index_tmp == index_leaf) {
            *index_vec = i;
            indices->indices.offset = 0;
            return true;
        }
    }

    // reset offset
    indices->indices.offset = 0;
    return false;
}

/**
 * @brief Move the offset of the leaves data.
 *
 * @param leaves The leaves data.
 * @param index The index to move the offset to.
 * @return parser_error_t The error code.
 */
parser_error_t schema_move_leaf_offset(merkle_leaves_data_t *leaves, uint64_t index) {
    CHECK_INPUT(leaves);
    uint32_t data_length = 0;
    for (uint64_t i = 0; i < index; i++) {
        CHECK_ERROR(read_u32(&leaves->data, &data_length));
        if (leaves->data.offset + data_length > leaves->data.buffer.len) {
            return parser_unexpected_buffer_end;
        }
        leaves->data.offset += data_length;
    }
    return parser_ok;
}

/**
 * @brief Reset the offset of the leaves data.
 *
 * @param leaves The leaves data.
 * @return parser_error_t The error code.
 */
parser_error_t schema_reset_leaf_offset(merkle_leaves_data_t *leaves) {
    CHECK_INPUT(leaves);
    leaves->data.offset = 0;
    return parser_ok;
}

/**
 * @brief Get the link index of the fields.
 *
 * @param fields The fields.
 * @param fields_qty The number of fields.
 * @param field_index The field index.
 * @param qty The quantity of fields.
 * @param max_indexes The maximum number of fields.
 * @return parser_error_t The error code.
 */
parser_error_t get_named_link_index(named_field_t *fields, uint32_t fields_qty, uint32_t field_index[], uint16_t *qty,
                                    uint16_t max_indexes) {
    CHECK_INPUT(fields);
    CHECK_INPUT(field_index);
    CHECK_INPUT(qty);

    for (uint32_t i = 0; i < fields_qty; i++) {
        if (fields[i].value.tag == LINK_BY_INDEX) {
            field_index[(*qty)++] = fields[i].value.data.by_index;
        }
    }

    return (*qty >= max_indexes) ? parser_scheme_named_link_index_overflow : parser_ok;
}

parser_error_t get_unnamed_link_index(unnamed_field_t *fields, uint32_t fields_qty, uint32_t field_index[], uint16_t *qty,
                                      uint16_t max_indexes) {
    CHECK_INPUT(fields);
    CHECK_INPUT(field_index);
    CHECK_INPUT(qty);
    *qty = 0;

    for (uint32_t i = 0; i < fields_qty; i++) {
        if (fields[i].value.tag == LINK_BY_INDEX) {
            field_index[(*qty)++] = fields[i].value.data.by_index;
        }
    }

    return (*qty >= max_indexes) ? parser_scheme_unnamed_link_index_overflow : parser_ok;
}

/**
 * @brief Get the link index of the variants.
 *
 * @param variants The variants.
 * @param variants_qty The number of variants.
 * @param field_index The field index.
 * @param qty The quantity of fields.
 * @param max_indexes The maximum number of fields.
 * @return parser_error_t The error code.
 */
parser_error_t get_variant_link_index(enum_variant_t *variants, uint32_t variants_qty, uint32_t field_index[], uint16_t *qty,
                                      uint16_t max_indexes) {
    CHECK_INPUT(variants);
    CHECK_INPUT(field_index);
    CHECK_INPUT(qty);

    for (uint32_t i = 0; i < variants_qty; i++) {
        if (variants[i].value.tag == LINK_BY_INDEX) {
            field_index[(*qty)++] = variants[i].value.data.by_index;
        }
    }

    return (*qty >= max_indexes) ? parser_scheme_variant_index_overflow : parser_ok;
}

parser_error_t get_schema_type(parser_tx_t *txObj, uint32_t index, uint8_t *type) {
    CHECK_INPUT(txObj);
    CHECK_INPUT(type);

    uint64_t mem_offset = txObj->merkle_proofs.leaves.data.offset;
    txObj->merkle_proofs.leaves.data.offset = 0;

    uint64_t index_vec = 0;
    if (!schema_find_index(index, &txObj->merkle_proofs.indices, &index_vec)) {
        return parser_schema_index_not_found;
    }

    CHECK_ERROR(schema_move_leaf_offset(&txObj->merkle_proofs.leaves, index_vec));

    uint32_t len = 0;
    CHECK_ERROR(read_u32(&txObj->merkle_proofs.leaves.data, &len));

    CHECK_ERROR(read_u8(&txObj->merkle_proofs.leaves.data, type));

    txObj->merkle_proofs.leaves.data.offset = mem_offset;

    return parser_ok;
}

bool is_link_skip(link_t *link) {
    if (link->tag == LINK_IMMEDIATE && link->data.immediate.type == PRIMITIVE_SKIP) {
        return true;
    }
    return false;
}
