# App Store release — macOS and iOS

One target, `SwiftUI-CPP`, builds both: `SUPPORTED_PLATFORMS = iphoneos
iphonesimulator macosx`.  So there is one version to keep and two archives to
make.

- bundle ID: `tokyo.828.SliP`
- version: `2.2.0`  ( `MARKETING_VERSION` )
- build: `14`  ( `CURRENT_PROJECT_VERSION` )
- minimum system: macOS 13.3, iOS 17.0
- architectures: Apple silicon and Intel
- App Sandbox: enabled
- document access: user-selected read/write only
- data collection: none

The existing GitHub release workflow is for Developer ID distribution. App
Store builds use an Apple Distribution certificate and a Mac App Store
provisioning profile through Xcode's Organizer.

## Before uploading

1. Accept the latest agreements in App Store Connect.
2. Register `tokyo.828.SliP` under Certificates, Identifiers & Profiles if it
   is not already registered.
3. Create a macOS app record in App Store Connect using that bundle ID.
4. Publish `Web/Privacy.html` so the metadata privacy URL is
   `https://slip.828.tokyo/Privacy.html`.
5. In Xcode, select the `SwiftUI-CPP` scheme and `Any Mac`, then use
   **Product → Archive**.
6. In Organizer, run **Validate App**, then **Distribute App → App Store
   Connect → Upload**.
7. Confirm the processed build under TestFlight before submitting it for
   review.
8. Switch the destination to `Any iOS Device (arm64)` and repeat 5 to 7.  App
   Store Connect counts build numbers per platform, so the same build number
   serves both.

## The two version numbers

`MARKETING_VERSION` is what a user sees and must match
[CHANGELOG.md](../CHANGELOG.md).  `CURRENT_PROJECT_VERSION` is the build number,
and it has to **increase for every upload** — including a second upload of the
same marketing version after a rejection.  Bumping only the first one gets the
upload refused.

They live in six and two places respectively in `SliP.xcodeproj/project.pbxproj`
( Debug and Release, across the app and its two test targets; only the app's
build number matters ).  Editing them on the project in Xcode sets all of them.

    xcodebuild -project SliP.xcodeproj -target SwiftUI-CPP -showBuildSettings \
    | grep -E 'MARKETING_VERSION|CURRENT_PROJECT_VERSION'

is how to check what Xcode will actually use.

## Suggested listing — English

**Name:** SliP

**Subtitle:** A programmable symbolic calculator

**Promotional text:** Calculate naturally, then grow your expressions into a
small, expressive language.

**Description:**

SliP is a programmable symbolic calculator for Mac. Start with compact
mathematical expressions such as `2πr` and `cosπ`, then build reusable
definitions, lists, matrices, JSON values, and small programs.

Calculator mode evaluates one expression per line and keeps going when a line
contains an error. Programming mode evaluates top-level forms in order, so
later definitions can build on earlier ones.

Features:

- Native document-based Mac app
- Dedicated keypad for mathematical and SliP operators
- Independent interpreter session for every document window
- Optional binding persistence between runs
- Selectable floating-point display precision
- Plain-text `.slip` documents
- Runs locally with no account, analytics, advertising, or data collection

**Keywords:** calculator,symbolic,math,programming,lisp,language,matrix

## Suggested listing — Japanese

**名前:** SliP

**サブタイトル:** プログラム可能な記号計算機

**プロモーションテキスト:** 自然な数式から始めて、そのまま小さく表現力のあるプログラムへ。

**説明:**

SliPはMac用のプログラム可能な記号計算機です。`2πr`や`cosπ`のような
簡潔な数式から始めて、再利用できる定義、リスト、行列、JSON値、小さな
プログラムへ発展させられます。

計算モードは1行に1つの式を評価し、途中にエラーがあっても次の行へ進み
ます。プログラミングモードはトップレベルのフォームを順番に評価するため、
前に書いた定義を後から利用できます。

主な機能：

- macOSネイティブの文書ベースアプリ
- 数学記号とSliP演算子の専用パッド
- 文書ウインドウごとに独立した評価セッション
- 実行間で定義を保持するオプション
- 小数の表示精度設定
- プレーンテキストの`.slip`文書
- アカウント、広告、アクセス解析、データ収集なし

**キーワード:** 計算機,記号計算,数学,プログラミング,Lisp,言語,行列

## Review notes

SliP is a local, document-based symbolic calculator and programming language.
No login, network service, purchase, or special hardware is required.

To exercise the core functionality:

1. Launch the app. A new document contains three calculator examples.
2. Select **Calculator** and press Command-Return.
3. Select **Programming** and enter `( 'r = 2 )` followed by `( 2 π r )`,
   then press Command-Return.
4. Enable **Keep session**, define a value, and use it in a subsequent run.
5. Open a second document to confirm that its interpreter session is
   independent.

The app collects no data. Its privacy policy is available from the Help menu
and at `https://slip.828.tokyo/Privacy.html`.

## Screenshots

`AppStore/Screenshots` has the six that 2.1.1 shipped with — Mac 1440x900,
iPhone 6.5 1242x2688 and iPad 2048x2732, each in Japanese and English.  The
version is in the file name, so a set taken for a new version is renamed to
match.  2.2.0 did not change the Mac or iOS interface, so the existing ones are
still accurate.

## Screenshot plan

Use a clean 1440 × 900 or larger desktop and capture:

1. Calculator mode showing the bundled examples and results.
2. Programming mode showing a named definition and result.
3. Two document windows to communicate independent workspaces.
4. The symbol keypad and selected output.
5. The built-in help window.

Avoid showing unrelated desktop files, account names, debug menus, or unsigned
build warnings.
