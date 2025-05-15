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
#include "schema_reader.h"
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

parser_error_t schema_get_unsigned_transaction_index(parser_tx_t *txObj, uint64_t *root_index) {
    CHECK_INPUT(txObj);

    // Get root index
    if (txObj->schema.root_type_indices.qty <= ROLLUP_ROOTS_UNSIGNED_TRANSACTION) {
        return parser_root_type_indices_overflow;
    }

    for (uint8_t i = 0; i <= (uint16_t)ROLLUP_ROOTS_UNSIGNED_TRANSACTION; i++) {
        CHECK_ERROR(read_u64(&txObj->schema.root_type_indices.indices, root_index));
    }
    txObj->schema.root_type_indices.indices.offset = 0;

    return parser_ok;
}

bool is_link_skip(link_t *link) {
    if (link->tag == LINK_IMMEDIATE && link->data.immediate.type == PRIMITIVE_SKIP) {
        return true;
    }
    return false;
}

bool should_show_field(link_t *value, bool silent, bool field_is_expert, bool expert_mode) {
    if (silent && is_link_skip(value)) {
        return false;
    }

    if (field_is_expert && !expert_mode) {
        return false;
    }

    return true;
}
