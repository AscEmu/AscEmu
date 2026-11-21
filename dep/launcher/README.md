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

The launcher accepts Forever builds **69893-71037**; 71037 is the live client of 2026-11-21. The runtime region-key patch was verified with the beta builds 69893-70205 and the live client 70959.

For a supported build the launcher:

- writes the configured Battle.net portal to `WTF/Config.wtf`;
- launches WoW with `-config Config.wtf` (the client resolves that name inside `WTF`, matching the known-good launcher behavior);
- uses the known-good Forever `none` launch path: no pre-launch portal/registry/GameCrypto memory patches are applied;
- starts the client suspended, attaches the region-key monitor and resumes it;
- scans the heap of the client for the runtime `RegionGroup 1..n` table (ids 1..n in a row, 0x28 bytes per record, at least 8 records);
- accepts a table only when its records hold Ed25519 public keys (or are empty) and record 8, the one the server signs for, holds a key; anything else with the same shape is model or layout data and stays untouched;
- replaces all 32-byte region public keys of the table when it becomes available;
- keeps monitoring and patches the table the client builds for the next connect;
- writes what it found, skipped and patched to `%LOCALAPPDATA%\AscEmu\Launcher\launcher.log`;
- plays a Windows notification sound after a successful region-key patch.

## Build requirements

- Windows 10/11 x64
- Visual Studio 18 2026 with CMake tools (optional but recommended)
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
cmake -S . -B build -G "Visual Studio 18 2026" -A x64
cmake --build build --config Release --target ascemu_launcher_publish
```

## Settings

Settings are read from `launcher.json` next to `AscEmu Launcher.exe` first, then from:

```text
%LOCALAPPDATA%\AscEmu\Launcher\launcher.json
```

Without a file the launcher uses the portal `bnet.ascemu.local:1119`. The service line shows the checked portal and the
reason when it is offline; its tooltip names the settings file in use.

## Host and certificate setup

When the portal host is a name that does not resolve (e.g. the default `bnet.ascemu.local`), the launcher maps it to
`ServerAddress` (default `127.0.0.1`) in the Windows hosts file on first start, asking once through UAC. Set
`AutoConfigureHosts` to `false` to manage the entry yourself, or point `ServerAddress` at the server when it is not local.
Set `CaCertificatePath` to a CA `.pem` to also trust it in the machine root store in the same elevation; left empty the
CA is not touched.

The client only accepts the logonserver when its certificate names the portal host (logon.conf `<TLS HostNames>`,
written into a newly created certificate) and the root of that certificate is trusted in the machine root store.

## account UI / launcher backend

The launcher already has a account UI area and a direct TLS reachability check against the configured Battle.net endpoint. A real account UI should come from an authenticated AscEmu launcher API on `bnetserver`; it is intentionally not read directly from the database or fabricated locally.

## Build note

The project uses `Microsoft.NET.Sdk` with `<UseWPF>true</UseWPF>` and explicit global .NET usings. This keeps the WPF launcher compatible with current .NET SDKs (including newer SDKs that can target .NET 8) without relying on the deprecated `Microsoft.NET.Sdk.WindowsDesktop` root SDK declaration.

## Battle.net availability

The launcher checks the configured Battle.net service every 10 seconds. The PLAY button is enabled only when the client executable is present, the Forever build is supported, and Battle.net is reachable. If the service goes offline, PLAY is disabled automatically and becomes available again when the service recovers.
