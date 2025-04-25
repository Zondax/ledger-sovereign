/*******************************************************************************
 *   (c) 2018 - 2024 Zondax AG
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

#include "schema_proof.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "borsh.h"
#include "parser_common.h"
#include "parser_impl.h"
#include "schema_reader.h"
#include "stack_manager.h"

const uint8_t LEAF_PREFIX = 0x00;
const uint8_t INNER_PREFIX = 0x01;
const uint8_t HASH_BUFFER_QTY = 2;

/**
 * @brief Calculate the largest power of two which is strictly less than the argument.
 *
 * @param n The input value.
 * @return The largest power of two which is strictly less than the argument.
 */
static uint32_t next_smaller_po2(uint32_t n) {
    if (n == 0) {
        return 0;
    }

    if ((n & (n - 1)) == 0) {
        return n >> 1;
    }
    // Find the next power of two greater than or equal to n
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;

    if (n == UINT32_MAX) {
        return (UINT32_MAX >> 1) + 1;
    }

    n += 1;

    // Return the next smaller power of two
    return n >> 1;
}

/**
 * @brief Compute the size of the tree needed to prove a given index.
 *
 * @param num_right_siblings The number of right siblings needed.
 * @param index_of_last_included_leaf The index of the last included leaf.
 * @param tree_size The output buffer for the computed tree size.
 * @return parser_error_t Error code indicating the result of the operation.
 */
parser_error_t compute_tree_size(uint32_t num_right_siblings, uint32_t index_of_last_included_leaf, uint32_t *tree_size) {
    uint32_t index_of_final_node = index_of_last_included_leaf;
    uint32_t mask = 1;
    uint32_t remaining_right_siblings = num_right_siblings;

    while (remaining_right_siblings > 0) {
        if ((index_of_final_node & mask) == 0) {
            if (index_of_final_node > UINT32_MAX - mask) {
                return parser_value_out_of_range;
            }
            index_of_final_node |= mask;
            remaining_right_siblings--;
        }
        mask <<= 1;
        if (index_of_final_node == UINT32_MAX) {
            return parser_value_out_of_range;
        }
    }

    *tree_size = index_of_final_node + 1;
    return parser_ok;
}

/**
 * @brief Compute the hash of a leaf node in the proof.
 *
 * @param proof The proof structure containing the leaf node.
 * @return parser_error_t Error code indicating the result of the operation.
 */
static parser_error_t hash_index_leaf(merkle_leaves_data_t *leaves, uint64_t index, uint8_t *hash) {
    CHECK_INPUT(leaves);
    CHECK_INPUT(hash);

    // move the offset where leaves_data[index] starts
    uint32_t data_length = 0;
    for (uint64_t i = 0; i < index; i++) {
        CHECK_ERROR(read_u32(&leaves->data, &data_length));
        if (leaves->data.offset + data_length > leaves->data.buffer.len) {
            return parser_unexpected_buffer_end;
        }
        leaves->data.offset += data_length;
    }

    // get data length
    CHECK_ERROR(read_u32(&leaves->data, &data_length));

    // Compute hash from entry and store it in proof->hash
    crypto_sha256_init();
    crypto_sha256_update(&LEAF_PREFIX, 1);
    crypto_sha256_update(leaves->data.buffer.ptr + leaves->data.offset, data_length);
    crypto_sha256_final(hash);

    // reset offset
    leaves->data.offset = 0;

    return parser_ok;
}

/**
 * @brief Merge two branches in a binary tree and compute their combined hash.
 *
 * @param left The hash of the left branch.
 * @param right The hash of the right branch.
 * @param output The output buffer for the combined hash.
 * @return parser_error_t Error code indicating the result of the operation.
 */
// Declaring array sizes as static will trigger a warning if the user pass an array smaller than the size
static parser_error_t merge_branches(const uint8_t left[CX_SHA256_SIZE], const uint8_t right[CX_SHA256_SIZE],
                                     uint8_t output[CX_SHA256_SIZE]) {
    CHECK_INPUT(left);
    CHECK_INPUT(right);
    CHECK_INPUT(output);

    // TODO: remove this
    // bytes_t buffer_left = {0};
    // buffer_left.ptr = left;
    // buffer_left.len = CX_SHA256_SIZE;
    // print_buffer_u8(&buffer_left, "merge left");
    // print_buffer(&buffer_left, "merge left");

    // bytes_t buffer_right = {0};
    // buffer_right.ptr = right;
    // buffer_right.len = CX_SHA256_SIZE;
    // print_buffer_u8(&buffer_right, "merge right");
    // print_buffer(&buffer_right, "merge right");

    crypto_sha256_init();
    crypto_sha256_update(&INNER_PREFIX, 1);
    crypto_sha256_update(left, CX_SHA256_SIZE);
    crypto_sha256_update(right, CX_SHA256_SIZE);
    crypto_sha256_final(output);

    // bytes_t buffer_output = {0};
    // buffer_output.ptr = output;
    // buffer_output.len = CX_SHA256_SIZE;
    // print_buffer_u8(&buffer_output, "merge output");

    return parser_ok;
}

/**
 * @brief Get the next lemma hash.
 *
 * @param lemmas The lemmas structure containing the necessary data.
 * @param hash The output buffer for the hash.
 * @return parser_error_t Error code indicating the result of the operation.
 */
static parser_error_t get_next_lemma_hash(merkle_lemmas_t *lemmas, uint8_t *hash) {
    CHECK_INPUT(lemmas);
    CHECK_INPUT(hash);

    MEMCPY(hash, lemmas->data.buffer.ptr + lemmas->data.offset, CX_SHA256_SIZE);
    CTX_CHECK_AND_ADVANCE(&lemmas->data, CX_SHA256_SIZE);

    return parser_ok;
}

/**
 * @brief Find an index in the indices array.
 *
 * @param index The index to find.
 * @param indices The indices array.
 * @return bool True if the index is found, false otherwise.
 */
bool find_index(uint64_t index_leaf, merkle_leaves_indices_t *indices, uint64_t *index_vec) {
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
 * @brief Check if there are leaves in the range.
 *
 * @param start The start index.
 * @param end The end index.
 * @param indices The indices array.
 * @return bool True if there are leaves in the range, false otherwise.
 */
bool has_leaves(uint64_t start, uint64_t end, merkle_leaves_indices_t *indices) {
    CHECK_INPUT(indices);

    for (uint64_t i = 0; i < indices->entries; i++) {
        uint64_t index_tmp = 0;
        CHECK_ERROR(read_u64(&indices->indices, &index_tmp));
        if (index_tmp >= start && index_tmp < end) {
            indices->indices.offset = 0;
            return true;
        }
    }

    // reset offset
    indices->indices.offset = 0;
    return false;
}

/**
 * @brief Recursively check the range proof inner to merge the branches of the proof.
 *
 * @param proof The proof structure containing the necessary data.
 * @param index_start The start index.
 * @param index_end The end index.
 * @return parser_error_t Error code indicating the result of the operation.
 */
static parser_error_t verify_multiproof_inner(proof_t *proof, uint64_t index_start, uint64_t index_end, uint8_t *hash) {
    CHECK_INPUT(proof);
    CHECK_INPUT(hash);

    if (index_end <= index_start) {
        return parser_unexpected_buffer_end;
    }

    // If this is a single node, return the hash of the node
    if (index_end - index_start == 1) {
        uint64_t index_vec = 0;
        if (find_index(index_start, &proof->indices, &index_vec)) {
            CHECK_ERROR(hash_index_leaf(&proof->leaves, index_vec, hash));

            return parser_ok;
        }

        CHECK_ERROR(get_next_lemma_hash(&proof->lemmas, hash));

        return parser_ok;
    }

    uint32_t mid = index_start + next_smaller_po2(index_end - index_start);
    if (mid > index_end) {
        return parser_unexpected_buffer_end;
    }

    bool left_has_leaves = has_leaves(index_start, mid, &proof->indices);
    bool right_has_leaves = has_leaves(mid, index_end, &proof->indices);

    // Check left subtree
    CHECK_ERROR(checkStack());
    uint8_t left[CX_SHA256_SIZE];
    if (left_has_leaves) {
        CHECK_ERROR(verify_multiproof_inner(proof, index_start, mid, left));
    } else {
        CHECK_ERROR(get_next_lemma_hash(&proof->lemmas, left));
    }

    // Check right subtree
    CHECK_ERROR(checkStack());
    uint8_t right[CX_SHA256_SIZE];
    if (right_has_leaves) {
        CHECK_ERROR(verify_multiproof_inner(proof, mid, index_end, right));
    } else {
        CHECK_ERROR(get_next_lemma_hash(&proof->lemmas, right));
    }

    merge_branches(left, right, hash);

    // free stack for left and right
    return freeStack(HASH_BUFFER_QTY);
}

/**
 * @brief Compute the root hash for a given metadata structure.
 *
 * @param metadata The metadata structure containing the necessary data.
 * @param metadataDigest The output buffer for the computed metadata digest.
 * @return parser_error_t Error code indicating the result of the operation.
 */
parser_error_t get_root_hash(const merkle_proof_t *metadata, uint8_t *hash) {
    CHECK_INPUT(metadata);
    CHECK_INPUT(hash);

    proof_t proof = {0};
    proof.leaves = metadata->leaves;
    proof.lemmas = metadata->lemmas;
    proof.indices = metadata->indices;
    proof.lemma_index = metadata->lemmas.entries - 1;
    proof.tree_size = metadata->tree_size;

    print_buffer(&proof.leaves.data.buffer, "leaves data");
    print_u32("leaves.qty:", proof.leaves.entries);
    print_buffer(&proof.indices.indices.buffer, "indices");
    print_u32("indices.qty:", proof.indices.entries);
    print_buffer(&proof.lemmas.data.buffer, "lemmas");
    print_u32("lemmas.qty:", proof.lemmas.entries);
    print_u64("tree_size:", proof.tree_size);

    if (proof.tree_size > INT32_MAX) {
        return parser_value_out_of_range;
    }

    CHECK_ERROR(verify_multiproof_inner(&proof, 0, proof.tree_size, hash));

    return parser_ok;
}

parser_error_t verify_merkle_proofs(const merkle_proof_t *metadata) {
    CHECK_INPUT(metadata);
    CHECK_INPUT(metadata->root_hash.ptr);

    if (metadata->root_hash.len < CX_SHA256_SIZE) {
        return parser_unexpected_buffer_end;
    }

    uint8_t root_hash[CX_SHA256_SIZE] = {0};
    CHECK_ERROR(get_root_hash(metadata, root_hash));

    // compare root_hash received with computed root_hash
    if (memcmp(metadata->root_hash.ptr, root_hash, CX_SHA256_SIZE) != 0) {
        return parser_unexpected_root_hash;
    }

    print_string("root_hash matches");

    return parser_ok;
}
