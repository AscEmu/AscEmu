# Forever protocol profile

WoW Forever 1.60.1 clients log in through Battle.net with the v2 protobuf services like Midnight and use the
wide AES world transport of the 12.x engine (`src/version/Shared/World/WorldSocketAes.*`).

Pinned client:

- Version: 1.60.1
- Build: 70862

`World/WorldProfile.hpp` holds the values of this client. No build auth key of a Forever build is known, the
seed list is empty and the session digest of CMSG_AUTH_SESSION is accepted unverified; the session key and
the packet encryption do not depend on it.

## Status

Transport only, written 2026-11-08 and not tested with a client yet. The Forever client takes the Midnight
opcode table and object layouts until its own are known; the expansion helpers treat Forever as a modern
classic timeline, so the gameplay checks of the legacy expansions do not apply to it yet.

## Client

The client is started through the AscEmu Launcher (aaron02/CustomLauncher) next to `Wow.exe`: it writes the
portal into `WTF\Config.wtf` and replaces the Ed25519 region keys in memory with the public key of the pair in
`src/version/Shared/World/WorldConnectKey.hpp`.
