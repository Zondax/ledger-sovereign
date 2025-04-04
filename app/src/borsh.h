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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <sigutils.h>
#include <stdbool.h>

#include "coin.h"
#include "zxerror.h"
#include "zxmacros.h"

typedef enum {
    BORSH_TYPE_U8,
    BORSH_TYPE_U32,
    BORSH_TYPE_STRING,
    BORSH_TYPE_STRUCT,
    BORSH_TYPE_ENUM,
    // Add other types as needed
} borsh_type_t;

parser_error_t read_u32(parser_context_t* ctx, uint32_t* val);

#ifdef __cplusplus
}
#endif
