/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

class ByteBuffer;

namespace AscEmu::Version::Forever::ObjectUpdate::Detail
{
    void writeCreatureCreateFragments(ByteBuffer& data, bool hasVendorFragment);
    void writeGameObjectCreateFragments(ByteBuffer& data);
    void writeItemCreateFragments(ByteBuffer& data);
    void writePlayerCreateFragments(ByteBuffer& data, bool ownerVisible);
    void writeEmptyPlayerHouseInfoComponentCreate(ByteBuffer& data);
    void writeEmptyPlayerInitiativeComponentCreate(ByteBuffer& data);
}
