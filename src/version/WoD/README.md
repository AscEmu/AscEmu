# WoD protocol profile

WoD 6.2.4 clients log in through Battle.net with the protobuf services (the same v1 services as Legion)
and select the Battle.net server topology.

Pinned client:

- Version: 6.2.4
- Build: 21742

`World/WorldProfile.hpp` holds the values of this client for the world connection shared with Legion
(`src/version/Shared/World`): connection initializer with magic, the 4 byte setup header before and the
6 byte header after CMSG_AUTH_SESSION, and the digest key without a build seed. The shared transport does
the RC4 on the size field, the compression of large encrypted packets and the authentication with the
Battle.net session announced through BattleNetComm. Everything after authentication goes through the
central opcode table and the managed packets.
