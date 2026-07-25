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

The attached app is **unsigned**. macOS Gatekeeper therefore refuses it on
first open; the release notes tell people to right-click → Open once. That is
acceptable for now but it is not a good first impression, and fixing it needs
an Apple Developer account — so it needs the maintainer, not CI as configured.

To turn it on:

1. In the Apple Developer account, create a **Developer ID Application**
   certificate. Export it as a `.p12` with a password.
2. Create an **App Store Connect API key** (Issuer ID, Key ID, `.p8`).
3. Add repository secrets:

   | Secret | From |
   |--------|------|
   | `MACOS_CERTIFICATE` | base64 of the `.p12` |
   | `MACOS_CERTIFICATE_PWD` | its password |
   | `MACOS_SIGNING_IDENTITY` | e.g. `Developer ID Application: Your Name (TEAMID)` |
   | `AC_API_KEY_ID`, `AC_API_ISSUER_ID`, `AC_API_KEY` | the App Store Connect key |

4. In the `app` job, replace `CODE_SIGNING_ALLOWED=NO` with the identity,
   then after zipping:

   ```sh
   xcrun notarytool submit SliP-macos.zip \
     --key AuthKey.p8 --key-id "$AC_API_KEY_ID" --issuer "$AC_API_ISSUER_ID" \
     --wait
   xcrun stapler staple build/Release/SwiftUI-CPP.app
   ```

   Re-zip after stapling — the ticket is stapled to the `.app`, not to the
   archive.

5. Drop the right-click note from the release body.

Until then the icon is a placeholder: a `π` on a rounded square, generated
rather than designed. Replacing the PNGs in
`SwiftUI-CPP/Assets.xcassets/AppIcon.appiconset` is enough; nothing reads them
by name except the catalog.
