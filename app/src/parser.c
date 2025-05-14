/*******************************************************************************
 *   (c) 2018 - 2023 Zondax AG
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

#include "common/parser.h"

#include <stdio.h>
#include <zxformat.h>
#include <zxmacros.h>
#include <zxtypes.h>

#include "borsh.h"
#include "coin.h"
#include "crypto.h"
#include "parser_common.h"
#include "parser_impl.h"
#include "render.h"
#include "ui_item_manager.h"

parser_error_t parser_init_context(parser_context_t *ctx, const uint8_t *buffer, uint16_t bufferSize) {
    ctx->offset = 0;
    ctx->buffer.ptr = NULL;
    ctx->buffer.len = 0;

    if (bufferSize == 0 || buffer == NULL) {
        // Not available, use defaults
        return parser_init_context_empty;
    }

    ctx->buffer.ptr = buffer;
    ctx->buffer.len = bufferSize;
    return parser_ok;
}

parser_error_t parser_parse(parser_context_t *ctx, const uint8_t *data, size_t dataLen, parser_tx_t *tx_obj) {
    CHECK_ERROR(parser_init_context(ctx, data, dataLen))

    return _read(ctx, tx_obj);
}

parser_error_t parser_validate(parser_tx_t *txObj) {
    // Iterate through all items to check that all can be shown and are valid
    uint8_t numItems = 0;
    CHECK_ERROR(parser_getNumItems(txObj, &numItems))

    char tmpKey[40] = {0};
    char tmpVal[40] = {0};

    for (uint8_t idx = 0; idx < numItems; idx++) {
        uint8_t pageCount = 0;
        CHECK_ERROR(parser_getItem(txObj, idx, tmpKey, sizeof(tmpKey), tmpVal, sizeof(tmpVal), 0, &pageCount))
    }
    return parser_ok;
}

parser_error_t parser_getNumItems(const parser_tx_t *txObj, uint8_t *num_items) {
    *num_items = txObj->ui_items_new.qty;
    if (*num_items == 0) {
        return parser_unexpected_number_items;
    }
    return parser_ok;
}

static void cleanOutput(char *outKey, uint16_t outKeyLen, char *outVal, uint16_t outValLen) {
    MEMZERO(outKey, outKeyLen);
    MEMZERO(outVal, outValLen);
    snprintf(outKey, outKeyLen, "?");
    snprintf(outVal, outValLen, " ");
}

static parser_error_t checkSanity(uint8_t numItems, uint8_t displayIdx) {
    if (displayIdx >= numItems) {
        return parser_display_idx_out_of_range;
    }
    return parser_ok;
}

parser_error_t page_title(char *outKey, uint16_t outKeyLen, const char *inValue) {
    CHECK_INPUT(outKey);
    CHECK_INPUT(inValue);

    if (outKeyLen == 0) {
        return parser_ui_buffer_too_small;
    }
    outKeyLen--;

    MEMZERO(outKey, outKeyLen);

    clear_item_title_buffer();
    init_item_title_buffer(inValue);

    uint8_t items_qty = 0;
    CHECK_ERROR(get_title_item_qty(&items_qty))

    if (items_qty == 0) {
        return parser_ui_item_title_empty;
    }

    CHECK_ERROR(create_item_title(items_qty - 1, items_qty, outKey, outKeyLen));

    return parser_ok;
}

parser_error_t page_item(const parser_tx_t *txObj, char *outValue, uint16_t outValueLen, const char *title,
                         primitive_t *primitive, parser_context_t *data_context, uint8_t pageIdx, uint8_t *pageCount) {
    CHECK_INPUT(outValue);
    CHECK_INPUT(title);
    CHECK_INPUT(primitive);
    CHECK_INPUT(data_context);
    CHECK_INPUT(pageCount);

    MEMZERO(outValue, outValueLen);
    *pageCount = 0;

    if (outValueLen == 0) {
        return parser_ui_buffer_too_small;
    }
    outValueLen--;

    clear_item_title_buffer();
    init_item_title_buffer(title);

    uint8_t items_qty = 0;
    CHECK_ERROR(get_title_item_qty(&items_qty))

    char ui_buffer[200] = {0};
    uint8_t page_count_title = 0;
    if (items_qty > 1) {
        CHECK_ERROR(create_item_title(0, items_qty - 1, ui_buffer, sizeof(ui_buffer)));
        strncat(ui_buffer, ":", 1);
        page_count_title = (uint8_t)(strlen(ui_buffer) / outValueLen);
        const uint16_t lastChunkLen_title = (strlen(ui_buffer) % outValueLen);
        if (lastChunkLen_title > 0) {
            page_count_title++;
        }
    }

    uint8_t page_count_content = 0;
    char ui_data_buffer[200] = {0};
    CHECK_ERROR(render_primitive(data_context, txObj, primitive, ui_data_buffer, sizeof(ui_data_buffer)));
    data_context->offset = 0;
    page_count_content = (uint8_t)(strlen(ui_data_buffer) / outValueLen);
    const uint16_t lastChunkLen_content = (strlen(ui_data_buffer) % outValueLen);
    if (lastChunkLen_content > 0) {
        page_count_content++;
    }

    uint16_t aux = 0;
    if (pageIdx < page_count_title) {
        pageString(outValue, outValueLen, ui_buffer, pageIdx, &aux);
    } else {
        pageString(outValue, outValueLen, ui_data_buffer, pageIdx - page_count_title, &aux);
    }

    *pageCount = page_count_title + page_count_content;

    return parser_ok;
}

parser_error_t parser_getItem(const parser_tx_t *txObj, uint8_t displayIdx, char *outKey, uint16_t outKeyLen, char *outVal,
                              uint16_t outValLen, uint8_t pageIdx, uint8_t *pageCount) {
    UNUSED(pageIdx);
    *pageCount = 1;
    uint8_t numItems = 0;
    CHECK_ERROR(parser_getNumItems(txObj, &numItems))
    CHECK_APP_CANARY()

    CHECK_ERROR(checkSanity(numItems, displayIdx))
    cleanOutput(outKey, outKeyLen, outVal, outValLen);

    CHECK_ERROR(page_title(outKey, outKeyLen, txObj->ui_items_new.items[displayIdx].title))
    CHECK_ERROR(page_item(txObj, outVal, outValLen, txObj->ui_items_new.items[displayIdx].title,
                          &txObj->ui_items_new.items[displayIdx].primitive,
                          &txObj->ui_items_new.items[displayIdx].data_context, pageIdx, pageCount))

    return parser_ok;
}
