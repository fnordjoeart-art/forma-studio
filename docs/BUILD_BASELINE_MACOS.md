# macOS Apple Silicon build baseline

## Scope

This document records the known-good baseline build of the unmodified Bambu Studio fork before the FORMA kiosk prototype branding and functional work.

- Date: 2026-08-02
- Repository: `fnordjoeart-art/forma-studio`
- Branch: `forma-kiosk-prototype`
- Baseline commit: `6be9b71c0f26ff78bab036003120836e933722f7`
- Target architecture: Apple Silicon `arm64`

The `master` branch was not modified.

## Environment

| Component | Version |
| --- | --- |
| macOS | 26.5.2 (build 25F84) |
| Architecture | Apple Silicon `arm64` |
| Xcode | 26.6 (build 17F113) |
| AppleClang | 21.0.0 |
| Homebrew prefix | `/opt/homebrew` |
| CMake | 4.4.2 |

Xcode was selected through:

```text
/Applications/Xcode.app/Contents/Developer
```

## Homebrew dependencies

The following dependencies required by `BuildMac.sh` were present:

| Formula | Version |
| --- | --- |
| `autoconf` | 2.73 |
| `automake` | 1.18.1_1 |
| `cmake` | 4.4.2 |
| `libtool` | 2.6.2 |
| `nasm` | 3.02 |
| `pkgconf` | 3.0.4 |
| `texinfo` | 7.3 |
| `x264` | r3222 |
| `yasm` | 1.3.0_2 |

Homebrew also installed `m4` 1.4.21 as a transitive dependency. No personal taps, `brew extract`, forced links, formula pins, or `sudo` commands were used.

The build system downloaded its pinned web toolchain into the local cache:

- Node.js 22.22.2
- pnpm 10.12.1
- pnpm SHA-256: `8b39b2129a19eeec9511eb7cdde2450b604e389551d59ad738167f7495b56d52`

## Build command

From the repository root:

```bash
./BuildMac.sh -a arm64
```

Homebrew CMake 4.4.2 configured and compiled the repository successfully; CMake 3.31.0 was not required.

## Result

The baseline completed with `BUILD SUCCEEDED`. The generated application launched successfully on Apple Silicon and its main executable was verified as a 64-bit arm64 Mach-O binary.

Final bundle:

```text
/Users/utente/forma-studio/build/arm64/BambuStudio/BambuStudio.app
```
