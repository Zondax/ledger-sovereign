#include <cassert>
#include <cstdint>
#include <cstdio>

#include "parser.h"
#include "parser_common.h"
#include "parser_txdef.h"
#include "schema_reader.h"
#include "schema_txn_parser.h"
#include "zxmacros_x64.h"

#ifdef NDEBUG
#error "This fuzz target won't work correctly with NDEBUG defined, which will cause asserts to be eliminated"
#endif

using std::size_t;

namespace {
char PARSER_KEY[16384];
char PARSER_VALUE[16384];

// Helper function to test individual parsing functions with fresh contexts
void test_individual_functions(const parser_context_t &base_ctx, const parser_tx_t &base_txObj) {
    // Test merkle proofs reading independently
    {
        parser_context_t ctx = base_ctx;
        parser_tx_t txObj = base_txObj;
        schema_merkle_proofs_read(&ctx, &txObj);
    }

    // Test extra data reading independently
    {
        parser_context_t ctx = base_ctx;
        parser_tx_t txObj = base_txObj;
        schema_extra_data_read(&ctx, &txObj);
    }

    // Test transaction parsing independently
    {
        parser_context_t ctx = base_ctx;
        parser_tx_t txObj = base_txObj;
        schema_parser_transaction(&ctx, &txObj);
    }

    // Test chain hash reading independently
    {
        parser_context_t ctx = base_ctx;
        parser_tx_t txObj = base_txObj;
        schema_chain_hash_read(&ctx, &txObj);
    }
}

// Helper function to test partial pipelines and return successful result
bool test_partial_pipelines(const parser_context_t &base_ctx, const parser_tx_t &base_txObj, parser_tx_t &result_txObj) {
    // Test pipeline starting from merkle proofs
    {
        parser_context_t ctx = base_ctx;
        parser_tx_t txObj = base_txObj;

        if (schema_merkle_proofs_read(&ctx, &txObj) == parser_ok) {
            schema_extra_data_read(&ctx, &txObj);
            if (schema_parser_transaction(&ctx, &txObj) == parser_ok) {
                schema_chain_hash_read(&ctx, &txObj);
                result_txObj = txObj;
                return true;
            }
        }
    }

    // Test pipeline starting from extra data (skip merkle)
    {
        parser_context_t ctx = base_ctx;
        parser_tx_t txObj = base_txObj;

        schema_extra_data_read(&ctx, &txObj);
        if (schema_parser_transaction(&ctx, &txObj) == parser_ok) {
            schema_chain_hash_read(&ctx, &txObj);
            result_txObj = txObj;
            return true;
        }
    }

    // Test pipeline starting from transaction parsing (skip merkle and extra)
    {
        parser_context_t ctx = base_ctx;
        parser_tx_t txObj = base_txObj;

        if (schema_parser_transaction(&ctx, &txObj) == parser_ok) {
            schema_chain_hash_read(&ctx, &txObj);
            result_txObj = txObj;
            return true;
        }
    }

    return false;
}

// Helper function to test and iterate through parsed items
void test_parser_output(parser_tx_t &txObj) {
    if (parser_validate(&txObj) != parser_ok) {
        return;
    }

    uint8_t num_items;
    if (parser_getNumItems(&txObj, &num_items) != parser_ok) {
        assert(false);
    }

    for (uint8_t i = 0; i < num_items; i += 1) {
        uint8_t page_idx = 0;
        uint8_t page_count = 1;

        while (page_idx < page_count) {
            const parser_error_t rc = parser_getItem(&txObj, i, PARSER_KEY, sizeof(PARSER_KEY), PARSER_VALUE,
                                                     sizeof(PARSER_VALUE), page_idx, &page_count);

            if (rc != parser_ok) {
                (void)fprintf(stderr, "error getting item %u at page index %u: %s\n", (unsigned)i, (unsigned)page_idx,
                              parser_getErrorDescription(rc));
                assert(false);
            }

            page_idx += 1;
        }
    }
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    parser_tx_t txObj;
    MEMZERO(&txObj, sizeof(txObj));
    parser_context_t ctx;

    const parser_error_t rc = parser_init_context(&ctx, data, size);
    if (rc != parser_ok) {
        return 0;
    }

    // Test individual parsing functions independently to maximize coverage
    // This prevents early validation failures from blocking coverage of later stages
    test_individual_functions(ctx, txObj);

    // Test partial pipelines with different entry points
    parser_tx_t result_txObj;
    if (test_partial_pipelines(ctx, txObj, result_txObj)) {
        // Test parser output with the successful result (no redundant parsing)
        test_parser_output(result_txObj);
    }

    return 0;
}
