/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "version/Forever/OpcodeTable.hpp"

namespace AscEmu::Version::Forever
{
    namespace
    {
        const std::vector<OpcodeEntry> OpcodeStore =
        {
#define CMSG_ENTRY(name, value) { Opcode::name, value, #name, OpcodeDirection::Client }
#define SMSG_ENTRY(name, value) { Opcode::name, value, #name, OpcodeDirection::Server }
            CMSG_ENTRY(CMSG_PING, 0x00450006),
            CMSG_ENTRY(CMSG_ENUM_CHARACTERS, 0x00440014),
            CMSG_ENTRY(CMSG_CHECK_CHARACTER_NAME_AVAILABILITY, 0x00440071),
            CMSG_ENTRY(CMSG_CREATE_CHARACTER, 0x00440070),
            CMSG_ENTRY(CMSG_PLAYER_LOGIN, 0x00440016),
            CMSG_ENTRY(CMSG_CHAR_DELETE, 0x004400CB),
            CMSG_ENTRY(CMSG_DB_QUERY_BULK, 0x00440010),
            CMSG_ENTRY(CMSG_HOTFIX_REQUEST, 0x00440011),
            CMSG_ENTRY(CMSG_QUERY_CREATURE, 0x003E0143),
            CMSG_ENTRY(CMSG_QUERY_GAME_OBJECT, 0x003E0144),           // verified 69913: uint32 entry + modern packed gameobject GUID
            CMSG_ENTRY(CMSG_UNKNOWN_PLAYER_GUID_003E002D, 0x003E002D), // observed with a packed Player GUID; exact semantics unverified
            CMSG_ENTRY(CMSG_UNKNOWN_NPC_INTERACT_003F0029, 0x003F0029), // observed 69913 near NPC interaction; semantics unverified
            CMSG_ENTRY(CMSG_CLOSE_INTERACTION, 0x003F002A),
            CMSG_ENTRY(CMSG_LIST_INVENTORY, 0x003F0038),             // verified working on Forever 69913 vendor interaction
            CMSG_ENTRY(CMSG_SET_SELECTION, 0x003F00CD),
            CMSG_ENTRY(CMSG_GET_UNDELETE_CHARACTER_COOLDOWN_STATUS, 0x0044010F),
            CMSG_ENTRY(CMSG_BATTLE_PAY_GET_PURCHASE_LIST, 0x004400EA),
            CMSG_ENTRY(CMSG_BATTLE_PAY_GET_PRODUCT_LIST, 0x004400E9),
            CMSG_ENTRY(CMSG_UPDATE_VAS_PURCHASE_STATES, 0x00440123),
            CMSG_ENTRY(CMSG_QUICK_JOIN_AUTO_ACCEPT_REQUESTS, 0x00440132),
            CMSG_ENTRY(CMSG_GET_LAST_CATALOG_FETCH, 0x002B0036),
            CMSG_ENTRY(CMSG_SOCIAL_CONTRACT_REQUEST, 0x00440176),
            CMSG_ENTRY(CMSG_SOCIAL_CONTRACT_ACCEPT, 0x00440177),
            CMSG_ENTRY(CMSG_SERVER_TIME_OFFSET_REQUEST, 0x004400CA),
            CMSG_ENTRY(CMSG_CHARACTER_SELECT_GATE_ACK, 0x0044013A),
            CMSG_ENTRY(CMSG_CHARACTER_LIST_ACK, 0x00440197),
            CMSG_ENTRY(CMSG_UPDATE_ACCOUNT_DATA, 0x004400C4),
            CMSG_ENTRY(CMSG_LOGOUT_REQUEST, 0x003F0074),           // verified 69913: observed when pressing Logout
            CMSG_ENTRY(CMSG_LOGOUT_CANCEL, 0x003F0075),            // verified 69913: observed when cancelling Logout

            // Forever 69913 movement opcodes.
            // "verified" values were observed directly in Forever captures.
            // The remaining entries use the same low-ID movement layout observed in 69913 and retained
            // by modern clients; Forever uses the 0x42 client movement group.
            CMSG_ENTRY(CMSG_MOVE_CHANGE_TRANSPORT, 0x0042004E),       // inferred
            CMSG_ENTRY(CMSG_MOVE_JUMP, 0x00420006),                  // verified
            CMSG_ENTRY(CMSG_MOVE_DOUBLE_JUMP, 0x00420007),           // inferred
            CMSG_ENTRY(CMSG_MOVE_FALL_LAND, 0x00420017),             // inferred
            CMSG_ENTRY(CMSG_MOVE_FALL_RESET, 0x00420037),            // inferred
            CMSG_ENTRY(CMSG_MOVE_UPDATE_FALL_SPEED, 0x00420038),     // inferred
            CMSG_ENTRY(CMSG_MOVE_HEARTBEAT, 0x0042002E),             // verified 69913
            CMSG_ENTRY(CMSG_MOVE_INIT_ACTIVE_MOVER_COMPLETE, 0x00420064), // verified 69913 login sequence
            CMSG_ENTRY(CMSG_MOVE_INITIAL_OBJECT_UPDATE_COMPLETE_ACK, 0x00420083), // modern movement layout, observed in 69913 captures
            CMSG_ENTRY(CMSG_MOVE_SPLINE_DONE, 0x00420036),           // verified 69913: MovementInfo + spline id
            CMSG_ENTRY(CMSG_MOVE_SET_ADV_FLY, 0x00420070),           // inferred
            CMSG_ENTRY(CMSG_MOVE_SET_WALK_MODE, 0x0042000F),         // inferred
            CMSG_ENTRY(CMSG_MOVE_SET_RUN_MODE, 0x0042000E),          // inferred
            CMSG_ENTRY(CMSG_MOVE_SET_FLY, 0x00420047),               // inferred
            CMSG_ENTRY(CMSG_MOVE_SET_PITCH, 0x00420028),             // inferred
            CMSG_ENTRY(CMSG_MOVE_SET_FACING, 0x00420027),            // observed candidate; matches modern layout
            CMSG_ENTRY(CMSG_MOVE_SET_FACING_HEARTBEAT, 0x0042007F),  // observed candidate; matches modern layout

            CMSG_ENTRY(CMSG_MOVE_START_ASCEND, 0x00420048),          // inferred
            CMSG_ENTRY(CMSG_MOVE_START_BACKWARD, 0x00420001),        // verified
            CMSG_ENTRY(CMSG_MOVE_START_DESCEND, 0x0042004F),         // inferred
            CMSG_ENTRY(CMSG_MOVE_START_FORWARD, 0x00420000),         // verified
            CMSG_ENTRY(CMSG_MOVE_START_PITCH_DOWN, 0x0042000C),      // inferred
            CMSG_ENTRY(CMSG_MOVE_START_PITCH_UP, 0x0042000B),        // inferred
            CMSG_ENTRY(CMSG_MOVE_START_STRAFE_LEFT, 0x00420003),     // inferred
            CMSG_ENTRY(CMSG_MOVE_START_STRAFE_RIGHT, 0x00420004),    // inferred
            CMSG_ENTRY(CMSG_MOVE_START_SWIM, 0x00420018),            // inferred
            CMSG_ENTRY(CMSG_MOVE_START_TURN_LEFT, 0x00420008),       // inferred
            CMSG_ENTRY(CMSG_MOVE_START_TURN_RIGHT, 0x00420009),      // inferred

            CMSG_ENTRY(CMSG_MOVE_STOP, 0x00420002),                  // verified
            CMSG_ENTRY(CMSG_MOVE_STOP_ASCEND, 0x00420049),           // inferred
            CMSG_ENTRY(CMSG_MOVE_STOP_PITCH, 0x0042000D),            // inferred
            CMSG_ENTRY(CMSG_MOVE_STOP_STRAFE, 0x00420005),           // inferred
            CMSG_ENTRY(CMSG_MOVE_STOP_SWIM, 0x00420019),             // inferred
            CMSG_ENTRY(CMSG_MOVE_STOP_TURN, 0x0042000A),             // inferred

            SMSG_ENTRY(SMSG_PONG, 0x004D0009),
            SMSG_ENTRY(SMSG_ENUM_CHARACTERS_RESULT, 0x00460018),
            SMSG_ENTRY(SMSG_CHARACTER_LIST_STATE, 0x00460019),
            SMSG_ENTRY(SMSG_CHECK_CHARACTER_NAME_AVAILABILITY_RESULT, 0x0046001B),
            SMSG_ENTRY(SMSG_CREATE_CHAR, 0x004601AB),
            SMSG_ENTRY(SMSG_DELETE_CHAR, 0x004601AC),
            SMSG_ENTRY(SMSG_DB_REPLY, 0x004A0000),
            SMSG_ENTRY(SMSG_AVAILABLE_HOTFIXES, 0x004A0001),
            SMSG_ENTRY(SMSG_CACHE_VERSION, 0x004A000E),
            SMSG_ENTRY(SMSG_QUERY_CREATURE_RESPONSE, 0x004A0006),
            SMSG_ENTRY(SMSG_QUERY_GAME_OBJECT_RESPONSE, 0x004A0007), // verified 69913
            SMSG_ENTRY(SMSG_VENDOR_INVENTORY, 0x0046005C),
            SMSG_ENTRY(SMSG_UNDELETE_COOLDOWN_STATUS_RESPONSE, 0x00460276),
            SMSG_ENTRY(SMSG_SOCIAL_CONTRACT_REQUEST_RESPONSE, 0x00460325),
            SMSG_ENTRY(SMSG_SERVER_TIME_OFFSET, 0x004601BF),
            SMSG_ENTRY(SMSG_ACCOUNT_ITEM_COLLECTION_DATA, 0x00460362),
            SMSG_ENTRY(SMSG_UPDATE_ACCOUNT_DATA_COMPLETE, 0x004601B4),
            SMSG_ENTRY(SMSG_LOGOUT_RESPONSE, 0x0046012F), // verified 69913
            SMSG_ENTRY(SMSG_LOGOUT_COMPLETE, 0x00460130), // verified 69913
            SMSG_ENTRY(SMSG_ACCOUNT_DATA_TIMES, 0x004601B5),
            SMSG_ENTRY(SMSG_FEATURE_SYSTEM_STATUS, 0x00460063),
            SMSG_ENTRY(SMSG_SET_TIME_ZONE_INFORMATION, 0x00460123),
            SMSG_ENTRY(SMSG_LOGIN_VERIFY_WORLD, 0x0046002F),
            SMSG_ENTRY(SMSG_LOGIN_SET_TIME_SPEED, 0x004601B8),
            SMSG_ENTRY(SMSG_UPDATE_OBJECT, 0x005D0000),

            // Forever 69913 server movement group. These values are present repeatedly in the
            // official captures; the low IDs match the modern movement opcode layout.
            SMSG_ENTRY(SMSG_TIME_SYNC_REQUEST, 0x005F0000),
            SMSG_ENTRY(SMSG_ON_MONSTER_MOVE, 0x005F0002),
            SMSG_ENTRY(SMSG_MOVE_UPDATE, 0x005F000E),
            SMSG_ENTRY(SMSG_CHARACTER_ENUM_PRELUDE, 0x0046021D),
            SMSG_ENTRY(SMSG_CHARACTER_ENUM_PRELUDE_EXTENDED, 0x0046021C),
            SMSG_ENTRY(SMSG_CHARACTER_SELECT_STATUS, 0x0046029D),
            SMSG_ENTRY(SMSG_CHARACTER_SELECT_GATE, 0x00460382)
#undef CMSG_ENTRY
#undef SMSG_ENTRY
        };
    }

    OpcodeTable& OpcodeTable::instance()
    {
        static OpcodeTable instance;
        return instance;
    }

    Opcode OpcodeTable::getInternalIdForHex(uint32_t rawOpcode) const
    {
        for (const auto& entry : OpcodeStore)
            if (entry.rawOpcode == rawOpcode && entry.direction == OpcodeDirection::Client)
                return entry.id;
        return Opcode::NONE;
    }

    uint32_t OpcodeTable::getHexValueForInternalId(Opcode opcode) const
    {
        for (const auto& entry : OpcodeStore)
            if (entry.id == opcode)
                return entry.rawOpcode;
        return 0;
    }

    std::string_view OpcodeTable::getNameForOpcode(uint32_t rawOpcode) const
    {
        for (const auto& entry : OpcodeStore)
            if (entry.rawOpcode == rawOpcode)
                return entry.name;
        return "UNKNOWN_OPCODE";
    }

    std::string_view OpcodeTable::getNameForInternalId(Opcode opcode) const
    {
        for (const auto& entry : OpcodeStore)
            if (entry.id == opcode)
                return entry.name;
        return "UNKNOWN_OPCODE";
    }

    const std::vector<OpcodeEntry>& OpcodeTable::entries() const
    {
        return OpcodeStore;
    }
}
