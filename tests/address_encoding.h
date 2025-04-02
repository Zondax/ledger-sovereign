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
#include <hexutils.h>

#include <iostream>
#include <vector>

#include "bech32.h"
#include "crypto_helper.h"
#include "gmock/gmock.h"
#include "parser_txdef.h"

using namespace std;
struct SovereignWalletAddress {
    string publicKey;
    string address;
};

vector<SovereignWalletAddress> testvectorWallet{
    {"2e33229dddfd44920beed86f97ce1b592e97339506de1c32c0a96be5ca91d740",
     "sov1rhu8cvf43me79tpjl4mayzxssudvp3x4a3kzryt7cf806penpz9"},
    {"c4b804eef9fa5766d33a29d8a63d7bd01c945f6121240510d05205d43d6acbd7",
     "sov13pv2720a03mnphs4c4wh3kp56jn2f5anr6gd6chphewygq9vjgg"},
    {"4734451bca47ac4acbae30d295ed4c621a53d91b2fa04964847d719eb8d72fee",
     "sov1pzvrqssdwed9w94r503z9ra6ued2uml7yhn6k9lnpmauzsynjce"},
    {"10ce0f90b441d491057e7e2acd49946df4e0515c45d2e8744a2b7c8e26fbb0b1",
     "sov16nmgjh63msqcpa93jq7lzlgt7ktdad955xslj8p8z00x2ulvsd0"},
    {"83c9786a4dea80a80c816100fb63c2e234f5eb317ae138b2396ceec39e94dbc4",
     "sov1w3p9h0kf8axy0yw4prnlq2h785qdu3vr3fsufxask6jhz732hll"},
    {"a6447b995558ef0020674169524c5f924512a0050c60fb8895970f5620071330",
     "sov1a93a884fv4r5p9k0d3rcg4rg6h8kfg6hrq4em47pr5fyj0pswm9"},
    {"7bc7cfad45193f99f5edc0d4fb9fdeb4653947dd66e089cb2cbdfb7cd7ba8a65",
     "sov14w2wm6nddwm6hrf3eenq53r8ypgm7njrtyhrwug8pzy874ygxkl"},
    {"c0ca884d75394e4be4974a8d910052533ee8f685fc5437efb472476fc517e675",
     "sov146ylxg8upt7mkvh0dcshgthekvjnps9nqcaydszejxhnxjg736a"},
    {"a21a95f31ac404801d073eec53855f0a249a088eb5217a2dbb3e8c940a28d1e1",
     "sov1eskgyhc4uudsg9engucv2765wx03dw93h3qjvulwcjq26xm0geh"},
    {"8af72f368b5ba4a5c0e3e5abec273e8f06d14bfa4545a3dbee0944a14f8548a8",
     "sov17sd8hykgre5m84687jefvhrpncu4c6ze7uelrqhfgkzy6h65fhg"},
    {"904c292ffffd019e7e097806a594b70f0c1238400d0be4a7a2b8c55b0ae485f9",
     "sov1t9cqvgpwlc2l0t4vw3042qv7pt8f9s0n9zp0n5k07glg6lp9tn4"},
    {"e4bed52478ebbaca17520c3ee6ab6bdb688f5829bc1db53a36816d80138a89e2",
     "sov1ft8p9nmuepj6te9f97lzcyne270dvedwjenewtf4230zvswsg64"},
    {"d18d34f1c6caa7ff310ea5ab64ea369c85d3deb2f7795faead98338c286d5217",
     "sov1lapltfa3aya0d9wl3szv2cyjshrlcjv3sv85s7qjqykqzyg0c4f"},
    {"b328e8634c51a71533d4d62fdf80a60568aaea79da552941d446ce25bd54e76d",
     "sov1yurvxdguf5vy52kxr3293vuuh2df4c0sp7a9yfs0n0a55auknvd"},
    {"a9efb20639f2da4401c5245681106f57396cdda6aaf8829b0252215709f0e639",
     "sov1g7jc6yl3xz3dxks57l7qhhvcphpmem8uu6r6elmccs23qw56h6c"},
    {"cfc890a9db31386ad225c0131df3900c4b7c2f26848ed0fafb68b191b20b4ded",
     "sov1qh60z8m0nuupql3qj6l8w9z2nzutnjry94qx655tl2wscng97xn"},
    {"beb556981b708bdef21452a3485093a477e210b012c3ced7506e09d7f5040b9f",
     "sov14dsk9kwypw2nmcvmj759wekczujk20e5hnqr3m6dlpak7cxn7xm"},
    {"0422edf97a5a2e71a621b0d5c4465d21f31ae3896093280edb635e445276933c",
     "sov1gsm7dts4e8ngldh463p80ayn6zx0satzs877chyla6y6w9x0h9u"},
    {"cda09be7dbbd231c0c96bda4941085a693249a19ee6f78924d5b67f0f06ca46d",
     "sov1xvlkvtlw4dnc3cdjmjaqmpehnfrynetrevppvzcrx5zm65knksf"},
    {"d2b19ddfec078ee8f1ebb726e272c964aa78f77ae5022ed165171d759766ed5b",
     "sov1swkndgqxxd5a36sxj9yeq89cltht3yh76wepa5eucjydu30tdqn"},
};
