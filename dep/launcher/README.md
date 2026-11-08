# AscEmu Launcher

Native AscEmu launcher project for the Forever client. The project owns its UI, configuration, build profiles and Forever patch flow under the `AscEmu.Launcher` namespace.

## Installation

`AscEmu Launcher.exe` is intentionally designed to live directly beside the client executable: `Wow.exe` of the live Forever client (`_forever_`), or `WowB.exe` of the beta client (`_classic_beta_`).

```text
_forever_/
  Wow.exe
  AscEmu Launcher.exe
  WTF/
    Config.wtf
```

The launcher does not store or search for a World of Warcraft path. It always uses `Wow.exe` (or `WowB.exe`) from its own directory and reports a clear error if the executable is missing.


## Current Forever profile

The launcher accepts Forever builds **69893-70862**; 70862 is the live client of 2026-11-07. The runtime region-key patch was verified with the beta builds 69893-70205, the live client uses the same runtime table scan and reports when the table layout differs.

For a supported build the launcher:

- writes the configured Battle.net portal to `WTF/Config.wtf`;
- launches WoW with `-config Config.wtf` (the client resolves that name inside `WTF`, matching the known-good launcher behavior);
- uses the known-good Forever `none` launch path: no pre-launch portal/registry/GameCrypto memory patches are applied;
- starts the client suspended, attaches the region-key monitor and resumes it;
- scans for the runtime `RegionGroup 1..n` table (ids 1..n in a row, 0x28 bytes per record, at least 8 records);
- replaces all 32-byte region public keys of the table when it becomes available;
- keeps monitoring and re-applies them if the client rebuilds the table;
- plays a Windows notification sound after a successful region-key patch.

## Build requirements

- Windows 10/11 x64
- Visual Studio 2022 with CMake tools (optional but recommended)
- CMake 3.24+
- .NET 8 SDK

## Build with CMake

From a Visual Studio Developer Command Prompt:

```bat
configure.bat
build.bat
```

Publish a standalone single-file launcher:

```bat
publish.bat
```

Published output:

```text
build\publish\ascemu_launcher\
```

Or manually:

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target ascemu_launcher_publish
```

## Settings

Settings are read from `launcher.json` next to `AscEmu Launcher.exe` first, then from:

```text
%LOCALAPPDATA%\AscEmu\Launcher\launcher.json
```

Without a file the launcher uses the portal `127.0.0.1:1119`. The service line shows the checked portal and the
reason when it is offline; its tooltip names the settings file in use.

## account UI / launcher backend

The launcher already has a account UI area and a direct TLS reachability check against the configured Battle.net endpoint. A real account UI should come from an authenticated AscEmu launcher API on `bnetserver`; it is intentionally not read directly from the database or fabricated locally.

## Build note

The project uses `Microsoft.NET.Sdk` with `<UseWPF>true</UseWPF>` and explicit global .NET usings. This keeps the WPF launcher compatible with current .NET SDKs (including newer SDKs that can target .NET 8) without relying on the deprecated `Microsoft.NET.Sdk.WindowsDesktop` root SDK declaration.

## Battle.net availability

The launcher checks the configured Battle.net service every 10 seconds. The PLAY button is enabled only when the client executable is present, the Forever build is supported, and Battle.net is reachable. If the service goes offline, PLAY is disabled automatically and becomes available again when the service recovers.
