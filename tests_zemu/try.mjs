import TransportNodeHid from '@ledgerhq/hw-transport-node-hid'
import { SovereignApp } from '@zondax/ledger-sovereign'
import { ed25519 } from '@noble/curves/ed25519'

const PATH = "m/44'/1551'/0'"
const BLOB =
  'e227ae01b445203f7fd92dd6e206f9de27713ba00c95f46dce5068b68f895b30cac0b4f2a74d6aa05d07b6a83cda6ef19b50e8c26e5653d80156ab1053f3fec3d161388122a7cc6ecfd790e833d127231785b0fdc337bb8e'

async function showAddress(app) {
  let resp = await app.deviceInfo()
  console.log('Device Info', resp)
  resp = await app.getVersion()
  console.log('Version', resp)

  resp = await app.getAddressAndPubKey(PATH, true)
  console.log(resp)
  const address = resp.address
  console.log('Address: ' + address.toString('hex'))
}

async function sign(app, action) {
  const messageToSign = Buffer.from(action, 'hex')
  try {
    const responseAddr = await app.getAddressAndPubKey(PATH, false)
    const pubKey = responseAddr.pubkey

    const signatureRequest = app.sign(PATH, messageToSign)
    const signatureResponse = await signatureRequest
    console.log('Signature:', signatureResponse.signature.toString('hex'))

    const valid = ed25519.verify(signatureResponse.signature, messageToSign, pubKey)
    if (valid) {
      console.log('Valid signature')
    } else {
      console.log('Invalid signature')
    }
  } catch (e) {
    console.log(e)
  }
}

async function main() {
  const transport = await TransportNodeHid.default.open()

  const app = new SovereignApp(transport)

  // Enable/disable(uncommenting) to try this features on a real device
  await showAddress(app)
  await sign(app, BLOB)
}

;(async () => {
  await main()
})()
