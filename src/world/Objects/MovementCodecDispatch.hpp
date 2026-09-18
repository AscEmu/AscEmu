/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "MovementCodec.hpp"
#include "Server/ClientProtocol.hpp"

// selects the movement codec of a client version at runtime, versions without a codec use the configured expansion
class MovementCodecDispatch
{
public:
    static bool hasDescriptor(WoW::Expansion expansion, uint16_t opcode, bool read)
    {
        switch (resolve(expansion))
        {
            case WoW::Expansion::_Classic: return MovementCodec<WoW::Expansion::_Classic>::hasDescriptor(opcode, read);
            case WoW::Expansion::_TBC: return MovementCodec<WoW::Expansion::_TBC>::hasDescriptor(opcode, read);
            case WoW::Expansion::_WotLK: return MovementCodec<WoW::Expansion::_WotLK>::hasDescriptor(opcode, read);
            case WoW::Expansion::_Cata: return MovementCodec<WoW::Expansion::_Cata>::hasDescriptor(opcode, read);
            default: return MovementCodec<WoW::Expansion::_Mop>::hasDescriptor(opcode, read);
        }
    }

    static void read(WoW::Expansion expansion, ByteBuffer& buffer, MovementInfo& movementInfo, uint16_t opcode)
    {
        switch (resolve(expansion))
        {
            case WoW::Expansion::_Classic: MovementCodec<WoW::Expansion::_Classic>::read(buffer, movementInfo, opcode); break;
            case WoW::Expansion::_TBC: MovementCodec<WoW::Expansion::_TBC>::read(buffer, movementInfo, opcode); break;
            case WoW::Expansion::_WotLK: MovementCodec<WoW::Expansion::_WotLK>::read(buffer, movementInfo, opcode); break;
            case WoW::Expansion::_Cata: MovementCodec<WoW::Expansion::_Cata>::read(buffer, movementInfo, opcode); break;
            default: MovementCodec<WoW::Expansion::_Mop>::read(buffer, movementInfo, opcode); break;
        }
    }

    static void write(WoW::Expansion expansion, ByteBuffer& data, MovementInfo const& movementInfo, uint16_t opcode, bool withGuid)
    {
        switch (resolve(expansion))
        {
            case WoW::Expansion::_Classic: MovementCodec<WoW::Expansion::_Classic>::write(data, movementInfo, opcode, withGuid); break;
            case WoW::Expansion::_TBC: MovementCodec<WoW::Expansion::_TBC>::write(data, movementInfo, opcode, withGuid); break;
            case WoW::Expansion::_WotLK: MovementCodec<WoW::Expansion::_WotLK>::write(data, movementInfo, opcode, withGuid); break;
            case WoW::Expansion::_Cata: MovementCodec<WoW::Expansion::_Cata>::write(data, movementInfo, opcode, withGuid); break;
            default: MovementCodec<WoW::Expansion::_Mop>::write(data, movementInfo, opcode, withGuid); break;
        }
    }

private:
    [[nodiscard]] static WoW::Expansion resolve(WoW::Expansion const expansion) noexcept
    {
        return WoW::isSupportedExpansion(expansion) ? expansion : WoW::getServerExpansion();
    }
};
