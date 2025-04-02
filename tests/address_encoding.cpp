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
#include "address_encoding.h"

using namespace std;

string toHexString(const uint8_t *data, size_t length) {
    std::stringstream hexStream;
    hexStream << std::hex << std::setfill('0');
    for (size_t i = 0; i < length; ++i) {
        hexStream << std::setw(2) << static_cast<int>(data[i]);
    }
    return hexStream.str();
}

TEST(Keys, WalletAddressEncoding) {
    for (const auto &testcase : testvectorWallet) {
        uint8_t internalPubkey[PK_LEN_25519] = {0};

        // Read pubkey from testvectors and set up wallet account
        parseHexString(internalPubkey, sizeof(internalPubkey), testcase.publicKey.c_str());

        uint8_t address[66] = {0};
        zxerr_t err = crypto_computeAddress(address, sizeof(address), internalPubkey);
        EXPECT_EQ(err, zxerr_ok);

        std::string str(reinterpret_cast<const char *>(address));
        EXPECT_EQ(str, testcase.address);
    }
}
