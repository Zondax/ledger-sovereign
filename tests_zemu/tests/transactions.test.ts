/** ******************************************************************************
 *  (c) 2018 - 2023 Zondax AG
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
 ******************************************************************************* */

import Zemu from '@zondax/zemu'
import { SovereignApp } from '@zondax/ledger-sovereign'
import { defaultOptions, models } from './common'
import { ed25519 } from '@noble/curves/ed25519'

jest.setTimeout(60000)

import { TRANSACTIONS_TEST_CASES } from './testscases/transactions'

jest.setTimeout(60000)

describe.each(TRANSACTIONS_TEST_CASES)('Tx', function (data) {
  test.concurrent.each(models)('sign transaction', async function (m) {
    const sim = new Zemu(m.path)
    try {
      await sim.start({ ...defaultOptions, model: m.name })
      const app = new SovereignApp(sim.getTransport())

      const txBlob = data.transaction
      const schema = data.schema
      const responseAddr = await app.getAddressAndPubKey(data.path, false)
      const pubKey = responseAddr.pubkey

      // do not wait here.. we need to navigate
      const signatureRequest = app.sign(data.path, txBlob, schema)

      // Wait until we are not in the main menu
      await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot())
      await sim.compareSnapshotsAndApprove('.', `${m.prefix.toLowerCase()}-sign_${data.name}`)

      const signatureResponse = await signatureRequest
      console.log(signatureResponse.signature.toString('hex'))

      const chainHash = data.schema.chainHash
      const txBlobWithChainHash = Buffer.concat([txBlob, chainHash])

      // Now verify the signature
      const valid = ed25519.verify(signatureResponse.signature, txBlobWithChainHash, pubKey)
      expect(valid).toEqual(true)
    } finally {
      await sim.close()
    }
  })
})
