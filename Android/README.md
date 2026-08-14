# SliP on Android

The same app the other platforms have, on the same interpreter. `C++/` is
compiled in through the NDK and reached from Kotlin over JNI; the screen follows
[`SwiftUI-CPP/ContentView.swift`](../SwiftUI-CPP/ContentView.swift), which is the
original.

## Build and run

Needs the Android SDK with NDK 29 and CMake 3.31. `local.properties` points at
the SDK and is not committed; create it with `sdk.dir=/path/to/Android/sdk`, or
just open the directory in Android Studio, which writes it.

```sh
cd Android && ./gradlew :app:installDebug
```

Or, without the wrapper:

```sh
gradle :app:assembleDebug && adb install -r app/build/outputs/apk/debug/app-debug.apk
```

## What it is

| | |
|---|---|
| `app/src/main/cpp/jni.cpp` | The bridge, in the same shape as [`Bridge/BH.mm`](../Bridge/BH.mm) |
| `app/src/main/cpp/CMakeLists.txt` | Compiles `C++/` directly — no copy, no Android fork |
| `app/src/main/kotlin/…/SliP.kt` | Kotlin's view of the engine, after [`Bridge/SliPEngine.swift`](../Bridge/SliPEngine.swift) |
| `app/src/main/kotlin/…/MainActivity.kt` | The screen, in Compose |

Both modes are the ones the other hosts have: **Calculator** reads a line as the
contents of a sentence, so `2πr` behaves as `( 2πr )` and a failing line does not
hide the ones below it; **Programming** reads toplevel forms and stops at the
first error, because a later form usually depends on an earlier one. The switch
beside them keeps bindings from the previous run instead of resetting.

## Two things worth knowing

**Strings cross as bytes, not as `jstring`.** JNI's `NewStringUTF` and
`GetStringUTFChars` speak *modified* UTF-8, which encodes anything outside the
BMP as a surrogate pair in six bytes rather than as the four bytes of real UTF-8.
SliP has such a character in its alphabet — `𝑒` is U+1D452 — so a `jstring` round
trip would corrupt the constant most likely to be typed right after `π`. The
native methods take and return `ByteArray`, and Kotlin decodes as UTF-8.

**`compat/stdckdint.h` is on the include path first.** The NDK sysroot has no
`<stdckdint.h>` at all, and `JP.h` includes it on every target that is not Apple.
The same header serves the ESP32 port, where picolibc *has* the header but hides
its macros from C++ — see [`compat/stdckdint.h`](../compat/stdckdint.h).

## The package name

`tokyo.sat.slip` — `sat.tokyo` reversed. The other apps identify as
`tokyo.828.SliP`, which an Android package name cannot be: every segment has to
start with a letter, and `828` does not. It is written once, in
`app/build.gradle.kts`. **Play binds an app to this string permanently**, so it
is worth being sure before the first upload.

## Releasing

R8 is on for the release build, so the JNI entry points are kept by name in
`proguard-rules.pro` — verified by running a release build on a device, not only
by watching it compile. 12 MB debug, 4.5 MB release, 5.0 MB as an AAB.

Signing is read from `Android/keystore.properties`, which is not committed and
is not here: it names a keystore and its passwords, and those are yours to
create. Without it the release variant still builds, unsigned.

[PLAY.md](PLAY.md) is the step-by-step for the store, in Japanese.

## The icon

Generated from the iOS artwork by [`icon.py`](icon.py), so the two cannot drift:

```sh
python3 Android/icon.py
```

iOS ships one square with the rounding baked in. Android wants two layers and
masks them itself — a circle on one launcher, a squircle on the next — so the
flattened tile would have its corners cut twice. The script splits that same
drawing: the gradient becomes the background, extended over the whole square so
the mask always lands on colour, and the marks are lifted onto the foreground by
how far each pixel leans from the navy toward the orange, which keeps their
antialiasing. The marks are then shrunk to 0.80 and re-centred, because a round
launcher clipped the `¡` at full size. The 512 for the Play listing keeps them at
the size iOS gives them, since Play rounds corners rather than cutting a circle.

The store's feature graphic comes from the same two pieces, in
[`feature.py`](feature.py): the marks, and the gradient, with the wordmark set in
Papyrus because that is what the site's `font-family: fantasy` resolves to.

## Not done here

- The graphics operators are absent, as they are everywhere except the browser —
  they reach the DOM, and this host has no DOM. `∥` runs its branches on real
  threads here, unlike the browser and the ESP32.
