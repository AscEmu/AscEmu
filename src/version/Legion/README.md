# Legion protocol profile

Legion 7.3.5 clients log in through Battle.net with the protobuf services (the same v1 services as WoD 6.2.4)
and select the Battle.net server topology.

Pinned client:

- Version: 7.3.5
- Build: 26972

`World/WorldProfile.hpp` holds the values of this client for the world connection shared with WoD
(`src/version/Shared/World`): line feed terminated initializers, the 6 byte header, the build auth seeds of
the three client platforms and the SMSG_ENABLE_ENCRYPTION handshake before the world encryption starts.
