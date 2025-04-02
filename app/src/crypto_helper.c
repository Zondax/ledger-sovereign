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
#include "crypto_helper.h"

#include "bech32.h"
#include "zxformat.h"
#if defined(LEDGER_SPECIFIC)
#include "cx.h"
#else
#include "picohash.h"
#define CX_SHA256_SIZE 32
#endif

zxerr_t crypto_computeSha256(uint8_t *output, uint16_t outputLen, const uint8_t *input, uint16_t inputLen) {
    if (output == NULL || outputLen == 0 || input == NULL) {
        return zxerr_invalid_crypto_settings;
    }

    if (outputLen < PK_LEN_25519) {
        return zxerr_invalid_crypto_settings;
    }

    MEMZERO(output, outputLen);

#if defined(LEDGER_SPECIFIC)
    cx_hash_sha256(input, inputLen, output, outputLen);
#else
    picohash_ctx_t ctx;
    picohash_init_sha256(&ctx);
    picohash_update(&ctx, input, inputLen);
    picohash_final(&ctx, output);
#endif
    return zxerr_ok;
}

zxerr_t crypto_computeAddress(uint8_t *address, uint16_t addressLen, const uint8_t *pubkey) {
    if (address == NULL || addressLen == 0 || pubkey == NULL) {
        return zxerr_invalid_crypto_settings;
    }

    if (addressLen < ADDRESS_MAX_LENGTH) {
        return zxerr_invalid_crypto_settings;
    }

    // address = Bech32(SHA256(pubkey)[0..28])
    MEMZERO(address, addressLen);

    uint8_t sha[PK_LEN_25519] = {0};
    CHECK_ZXERR(crypto_computeSha256(sha, sizeof(sha), pubkey, PK_LEN_25519));
    CHECK_ZXERR(
        bech32EncodeFromBytes((char *)address, ADDRESS_MAX_LENGTH, HRP, sha, PUBKEY_SHA_LEN, 1, BECH32_ENCODING_BECH32M));

    return zxerr_ok;
}
