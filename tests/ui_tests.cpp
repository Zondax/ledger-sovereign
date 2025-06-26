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

#include <hexutils.h>

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "app_mode.h"
#include "gmock/gmock.h"
#include "parser.h"
#include "utils/common.h"

using ::testing::TestWithParam;

typedef struct {
    uint64_t index;
    std::string name;
    std::string transaction_blob;
    std::string leaves_data;
    std::string leaves_index;
    std::string lemmas;
    std::string tree_size;
    std::string root_hash;
    std::string root_type_index;
    std::string chain_data;
    std::string extra_data_hash;
    std::string chain_hash;
    std::vector<std::string> expected;
    std::vector<std::string> expected_expert;
} testcase_t;

class JsonTestsA : public ::testing::TestWithParam<testcase_t> {
   public:
    struct PrintToStringParamName {
        template <class ParamType>
        std::string operator()(const testing::TestParamInfo<ParamType> &info) const {
            auto p = static_cast<testcase_t>(info.param);
            std::stringstream ss;
            ss << p.index << "_" << p.name;
            return ss.str();
        }
    };
};

// Retrieve testcases from json file
std::vector<testcase_t> GetJsonTestCases(std::string jsonFile) {
    auto answer = std::vector<testcase_t>();

    nlohmann::json obj;

    std::string fullPathJsonFile = std::string(TESTVECTORS_DIR) + jsonFile;

    std::ifstream inFile(fullPathJsonFile);
    if (!inFile.is_open()) {
        return answer;
    }

    // Retrieve all test cases
    obj = nlohmann::json::parse(inFile);
    std::cout << "Number of testcases: " << obj.size() << std::endl;

    for (int i = 0; i < obj.size(); i++) {
        auto outputs = std::vector<std::string>();
        for (auto s : obj[i]["output"]) {
            outputs.push_back(s.get<std::string>());
        }

        auto outputs_expert = std::vector<std::string>();
        for (auto s : obj[i]["output_expert"]) {
            outputs_expert.push_back(s.get<std::string>());
        }

        answer.push_back(testcase_t{
            obj[i]["index"].get<uint64_t>(), std::string("test_") + std::to_string(obj[i]["index"].get<uint64_t>()),
            obj[i]["transaction_blob"].get<std::string>(), obj[i]["leaves_data"].get<std::string>(),
            obj[i]["leaves_index"].get<std::string>(), obj[i]["lemmas"].get<std::string>(),
            obj[i]["tree_size"].get<std::string>(), obj[i]["root_hash"].get<std::string>(),
            obj[i]["root_type_index"].get<std::string>(), obj[i]["chain_data"].get<std::string>(),
            obj[i]["extra_data_hash"].get<std::string>(), obj[i]["chain_hash"].get<std::string>(), outputs, outputs_expert});
    }

    return answer;
}

void check_testcase(const testcase_t &tc, bool expert_mode) {
    app_mode_set_expert(expert_mode);

    parser_context_t ctx;
    parser_error_t err;

    uint8_t buffer[12000];
    uint16_t bufferLen = parseHexString(buffer, sizeof(buffer), tc.leaves_data.c_str());
    bufferLen += parseHexString(buffer + bufferLen, sizeof(buffer), tc.leaves_index.c_str());
    bufferLen += parseHexString(buffer + bufferLen, sizeof(buffer), tc.lemmas.c_str());
    bufferLen += parseHexString(buffer + bufferLen, sizeof(buffer), tc.tree_size.c_str());
    bufferLen += parseHexString(buffer + bufferLen, sizeof(buffer), tc.root_hash.c_str());
    bufferLen += parseHexString(buffer + bufferLen, sizeof(buffer), tc.root_type_index.c_str());
    bufferLen += parseHexString(buffer + bufferLen, sizeof(buffer), tc.chain_data.c_str());
    bufferLen += parseHexString(buffer + bufferLen, sizeof(buffer), tc.extra_data_hash.c_str());

    bufferLen += parseHexString(buffer + bufferLen, sizeof(buffer), tc.transaction_blob.c_str());
    bufferLen += parseHexString(buffer + bufferLen, sizeof(buffer), tc.chain_hash.c_str());

    parser_tx_t tx_obj;
    memset(&tx_obj, 0, sizeof(tx_obj));

    err = parser_parse(&ctx, buffer, bufferLen, &tx_obj);
    ASSERT_EQ(err, parser_ok) << parser_getErrorDescription(err);

    auto output = dumpUI(&tx_obj, 39, 39);

    std::cout << std::endl;
    for (const auto &i : output) {
        std::cout << i << std::endl;
    }
    std::cout << std::endl << std::endl;

    std::vector<std::string> expected = app_mode_expert() ? tc.expected_expert : tc.expected;

    EXPECT_EQ(output.size(), expected.size());
    for (size_t i = 0; i < expected.size(); i++) {
        if (i < output.size()) {
            EXPECT_THAT(output[i], testing::Eq(expected[i]));
        }
    }
}

INSTANTIATE_TEST_SUITE_P

    (JsonTestCasesCurrentTxVer, JsonTestsA, ::testing::ValuesIn(GetJsonTestCases("testcases.json")),
     JsonTestsA::PrintToStringParamName());
TEST_P(JsonTestsA, CheckUIOutput_CurrentTX_Expert) { check_testcase(GetParam(), true); }
TEST_P(JsonTestsA, CheckUIOutput_CurrentTX_Normal) { check_testcase(GetParam(), false); }