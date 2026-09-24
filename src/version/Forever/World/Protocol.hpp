#pragma once

#include <cstdint>
#include <string_view>

namespace AscEmu::Version::Forever::WorldProtocol
{
    inline constexpr std::string_view ServerInitializer =
        "WORLD OF WARCRAFT CONNECTION - SERVER TO CLIENT - V2\n";
    inline constexpr std::string_view ClientInitializer =
        "WORLD OF WARCRAFT CONNECTION - CLIENT TO SERVER - V2\n";

    inline constexpr uint32_t AuthTagSize = 12;
    inline constexpr uint32_t ServerHeaderSize = 4 + AuthTagSize;
    inline constexpr uint32_t ClientHeaderSize = ServerHeaderSize + 4;
    inline constexpr uint32_t MaxPacketSize = 0x10000;
    inline constexpr uint32_t ClientIvMagic = 0x544E4C43; // "CLNT"
    inline constexpr uint32_t ServerIvMagic = 0x52565253; // "SRVR"
    inline constexpr int32_t EnterEncryptedModeRegionGroup = 8;

    // Observed World-V2 auth packet shape:
    // 32-byte DoS challenge + 32-byte server challenge + 1-byte zero-bits.
    inline constexpr uint32_t AuthChallengePayloadSize = 65;

    // CMSG_AUTH_SESSION fixed prefix before the uint32 ticket length.
    // uint64 DoS response
    // uint32 region
    // uint32 battlegroup
    // uint32 realm
    // uint8  local challenge[32]
    // uint8  digest[24]
    // uint8  useIPv6
    inline constexpr uint32_t AuthSessionFixedSize = 77;

    // Forever 1.60.1.69893 world opcodes observed in the current protocol work.
    inline constexpr uint32_t SMSG_AUTH_CHALLENGE = 0x004D0000;
    inline constexpr uint32_t CMSG_AUTH_SESSION = 0x00450001;
    inline constexpr uint32_t CMSG_AUTH_CONTINUED_SESSION = 0x00450003;
    inline constexpr uint32_t SMSG_RESUME_COMMS = 0x004D0006;
    inline constexpr uint32_t SMSG_CONNECT_TO = 0x004D0008;

    // Observed recurring 8-byte client ping for Forever 1.60.1.69893.
    inline constexpr uint32_t CMSG_PING = 0x00450006;
    inline constexpr uint32_t SMSG_PONG = 0x004D0009;

    // Kept here for the next phase after the build-auth key is verified.
    inline constexpr uint32_t SMSG_ENTER_ENCRYPTED_MODE = 0x004D0004;
    inline constexpr uint32_t CMSG_ENTER_ENCRYPTED_MODE_ACK = 0x00450005;

    // First encrypted server block observed immediately after the ACK in
    // the official Forever 1.60.1.69893 capture.
    inline constexpr uint32_t SMSG_POST_AUTH_650007 = 0x00650007;
    inline constexpr uint32_t SMSG_POST_AUTH_4602CE = 0x004602CE;
    inline constexpr uint32_t SMSG_AUTH_RESPONSE = 0x00460001;
    inline constexpr uint32_t SMSG_SET_TIME_ZONE_INFORMATION = 0x00460123;
    inline constexpr uint32_t SMSG_FEATURE_SYSTEM_STATUS_GLUE_SCREEN = 0x00460064;
    inline constexpr uint32_t SMSG_POST_AUTH_CONFIG = 0x00460371;

    // Official 69893 glue bootstrap immediately before the first character enum.
    inline constexpr uint32_t SMSG_GLUE_BOOTSTRAP_EMPTY = 0x00460003;
    inline constexpr uint32_t SMSG_GLUE_BOOTSTRAP_STATE = 0x004602CB;
    inline constexpr uint32_t SMSG_TUTORIAL_FLAGS = 0x00460268;

    inline constexpr uint32_t SMSG_ACCOUNT_DATA_TIMES = 0x004601B5;

    // Application/game opcodes live in version/Forever/OpcodeTable.cpp.
    // Keep this file limited to World-V2 framing/auth/bootstrap constants.
}
