# The macOS app

`SwiftUI-CPP` is the document-based Mac app. It embeds `C++/` — the canonical
engine — through `C++/Embed.cpp` and the Objective-C++ bridge in
`SwiftUI-CPP/BH.mm`, so it runs exactly the language this repository specifies.

The Xcode target keeps its scaffolding name; the shipped app is called **SliP**.

```sh
xcodebuild -project SliP.xcodeproj -target SwiftUI-CPP \
  -configuration Release CODE_SIGNING_ALLOWED=NO build
open build/Release/SwiftUI-CPP.app
```

## What it does

| | |
|---|---|
| **Calculator** | One expression per line. A failing line reports and the rest still run. |
| **Programming** | Toplevel forms. Evaluation stops at the first failure, as a later form usually depends on an earlier one. |
| **Run** | ⌘↩ |
| **Keep session** | Carry bindings from the previous run instead of starting clean |
| **Keypad** | The operators no keyboard has — `¶ ¤ ∅ × ÷ ± · ∈ ∋ ¿ ¬ ¡ ¦ § ∥ ⟨ ⟩ « » 𝑒 π` |
| **Settings** (⌘,) | Rounding precision, 1–17 significant digits |

Documents are `.slip` files, registered as `tokyo.828.slip` conforming to
`public.plain-text`.

## Why the editor is not a `TextEditor`

macOS text substitution rewrites `'` into `’`. In this language `'` is the
quote operator, so `( 'r = 2 )` typed into a stock SwiftUI `TextEditor`
evaluates to `Undefined name: ’r` — and `TextEditor` exposes no way to turn
substitution off. `CodeEditor.swift` wraps an `NSTextView` with quote, dash,
replacement, correction and smart-insert substitution all disabled. Anything
that replaces it has to keep those off.

## Errors are values, not exceptions

The bridge never lets a C++ exception reach Swift: an exception thrown across
that boundary has nowhere to go but `std::terminate`. Every entry point in
`BH.h` returns JSON, and a SliP error arrives inside it. See `C++/Embed.hpp`
for the shapes.

## Releasing

The `app` job in [.github/workflows/release.yml](../.github/workflows/release.yml)
builds the app on a `v*` tag, checks that the bundle is named SliP, has an
icon, and carries the same version as `SLIP_VERSION` in `C++/SliP.hpp`, then
attaches `SliP-macos.zip` to the release.

### Signing and notarization

**The workflow is already written.** It signs, notarizes and staples when the
secrets below exist, and builds unsigned when they do not — so turning this on
is entering secrets, not editing YAML. The release body says which one it was;
the unsigned case tells people to right-click → Open.

Add these under Settings → Secrets and variables → Actions. Enter them in
GitHub directly: nothing needs to read them, and the `.p8` and the `.p12`
password are the whole security of your signing identity.

| Secret | What |
|--------|------|
| `MACOS_CERTIFICATE` | The `.p12`, base64 encoded: `base64 -i cert.p12 \| pbcopy` — see the `-legacy` note below |
| `MACOS_CERTIFICATE_PWD` | The password you set when exporting it |
| `MACOS_SIGNING_IDENTITY` | e.g. `Developer ID Application: Your Name (TEAMID)` |
| `AC_API_ISSUER_ID` | UUID at the top of the App Store Connect keys page |
| `AC_API_KEY_ID` | 10 characters, per key |
| `AC_API_KEY` | The contents of `AuthKey_<KeyID>.p8` |

Getting the two credentials:

- **Developer ID Application certificate** — Apple Developer → Certificates,
  Identifiers & Profiles → Certificates → ＋ → *Developer ID Application*.
  This is **not** the same as an *Apple Development* certificate: a
  development certificate only works on your own registered machines, and an
  app signed with one is still blocked everywhere else. Export it from
  Keychain Access **with its private key** — that is what makes it a `.p12`;
  a bare `.cer` cannot sign anything.
- **App Store Connect API key** — App Store Connect → Users and Access →
  Integrations → App Store Connect API → Team Keys → ＋. The Developer role is
  enough. The `.p8` downloads exactly once.

What the job does with them, in order: imports the certificate into a
throwaway keychain, builds with the hardened runtime (a precondition for
notarization, not a nicety), submits the zip to `notarytool --wait`, staples
the ticket to the `.app`, and re-zips — the ticket goes on the bundle, not on
the archive. It then asserts the result the way a downloader's Mac will see
it, with `codesign --verify`, `stapler validate` and `spctl --assess`, so a
misconfiguration fails the release rather than shipping.

### If `security import` says the password is wrong

It usually is not. OpenSSL 3 writes PKCS#12 with PBES2 / AES-256-CBC, and
Apple's `security` tool cannot read that — it reports the failure as
`MAC verification failed during PKCS12 import ( wrong password? )`, which
sends you looking in the wrong place. Export with `-legacy`:

```sh
openssl pkcs12 -export -legacy \
  -inkey private.key -in developerID.pem -certfile DeveloperIDG2CA.pem \
  -name "Developer ID Application: Your Name (TEAMID)" \
  -out cert.p12
```

`openssl pkcs12 -in cert.p12 -nokeys -noout -info` should then report
`pbeWithSHA1And40BitRC2-CBC` and `MAC: sha1` rather than PBES2. Keychain
Access exports in the compatible format already; this only bites when the
`.p12` is built with the command line.

Check it before pushing a tag, rather than finding out from CI:

```sh
KC=/tmp/check.keychain-db
security create-keychain -p test "$KC" && security unlock-keychain -p test "$KC"
security import cert.p12 -k "$KC" -P "$PASSWORD" -T /usr/bin/codesign
security delete-keychain "$KC"
```

### The icon

A red `( ) ;` on a rounded square, generated rather than designed. Replacing
the PNGs in `SwiftUI-CPP/Assets.xcassets/AppIcon.appiconset` is the whole job;
nothing reads them by name except the catalog. If you redraw it, check 16×16 —
three glyphs at that size turn to mush unless they are bold and nearly fill
the square.
