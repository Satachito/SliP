# SliP on Windows

The same app the other platforms have, on the same interpreter. `C++/` is
compiled straight in and the screen follows
[`SwiftUI-CPP/ContentView.swift`](../SwiftUI-CPP/ContentView.swift), which is the
original. One `SliP.exe`, statically linked, nothing to install beside it.

## Build

Cross-compiled from macOS or Linux with mingw-w64 — there is no Visual Studio
here and none is needed.

```sh
brew install mingw-w64      # macOS;  apt install mingw-w64 on Debian
sh Windows/build.sh
```

| | |
|---|---|
| `Windows/build/SliP.exe` | the app, 1.6 MB |
| `Windows/build/slip-cli.exe` | the console build, so the conformance suite can be run on this target |

`-static` links libstdc++, libgcc and winpthread in, so the exe runs on a machine
with nothing installed on it.

## Three things this host had to get right

**No JSON.** The browser, Mac and Android hosts sit on the far side of a
language boundary and read the JSON that `C++/Embed.hpp` writes for them. This
host is C++, so it calls `SessionRun` and walks the list. That entry point was
added for this port, and `REPL` / `Sugared` are now that same list with
`json_escape` applied — the modes' semantics live in one place rather than being
written out again per host.

**UTF-16 at the edge.** Win32 is UTF-16 and the interpreter is UTF-8, so every
string is converted at the boundary with `MultiByteToWideChar` /
`WideCharToMultiByte`. It has to be the real conversion, because SliP's alphabet
reaches past the BMP: `𝑒` is U+1D452, a surrogate pair in UTF-16 and four bytes
in UTF-8. This is the same hazard the Android port has with JNI's modified
UTF-8, arriving from the other direction.

**Header order.** `JP.h` opens with `using namespace std;`, and `rpcndr.h` —
reached from `windows.h` — declares `typedef unsigned char byte`. With `std`
already in scope that name is ambiguous against `std::byte` and `oaidl.h` stops
compiling on its own declarations. `main.cpp` includes the Windows headers
first, and says so where it does.

`compat/stdckdint.h` is first on the include path, as it is for the ESP32 and
Android builds: mingw ships the header but hides its `ckd_*` macros from C++,
exactly as picolibc does.

## The icon

[`icon.py`](icon.py) makes `slip.ico` from the iOS artwork. Windows draws an icon
as it is given — no adaptive mask, no safe zone — so unlike the Android icon this
one keeps the iOS tile whole, rounded corners and all. The only thing to undo is
the white the artwork sits on, which becomes transparent so the icon is not a
white square on a dark taskbar.

## Not done here

- **No installer and no code signing.** `SliP.exe` runs as it is, but SmartScreen
  will warn about an unsigned binary downloaded from the internet.
- No file association for `.slip`, which the Mac app has.
- The graphics operators are absent, as they are everywhere except the browser —
  they reach the DOM, and this host has no DOM.
