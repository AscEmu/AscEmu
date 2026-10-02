/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace AscEmu::Version::Forever::ProtocolLayout
{
    // Forever uses one wire layout across the supported 1.60.x client builds.
    // Keep protocol constants here instead of introducing build-specific branches.
    namespace ObjectType
    {
        inline constexpr uint8_t Object = 0;
        inline constexpr uint8_t Item = 1;
        inline constexpr uint8_t Container = 2;
        inline constexpr uint8_t Unit = 5;
        inline constexpr uint8_t Player = 6;
        inline constexpr uint8_t ActivePlayer = 7;
        inline constexpr uint8_t GameObject = 8;
        inline constexpr uint8_t DynamicObject = 9;
        inline constexpr uint8_t Corpse = 10;
    }

    namespace Values
    {
        inline constexpr uint8_t UpdateType = 0;
        inline constexpr uint8_t FragmentIdsChanged = 0;
        inline constexpr uint8_t ObjectContentsChangedMask = 0x01;
        inline constexpr uint8_t ItemContentsChangedMask = 0x03;
        inline constexpr uint8_t UnitContentsChangedMask = 0x03;
        inline constexpr uint8_t PlayerContentsChangedMask = 0x17;
    }

    namespace Create
    {
        inline constexpr uint8_t PlayerFieldFlags = 0x06;
        inline constexpr uint8_t SelfFieldFlags = 0x07;
        inline constexpr uint8_t CreatureFieldFlags = 0x04;
        inline constexpr uint8_t ItemFieldFlags = 0x01;
        inline constexpr uint8_t GameObjectFieldFlags = 0x00;
        inline constexpr uint8_t IndirectFragmentActivation = 0x01;

        namespace Fragment
        {
            inline constexpr uint8_t CGObject = 0x03;
            inline constexpr uint8_t Vendor = 0x12;
            inline constexpr uint8_t PlayerHouseInfo = 0x21;
            inline constexpr uint8_t PlayerInitiative = 0x26;
            inline constexpr uint8_t TagItem = 0xC8;
            inline constexpr uint8_t TagUnit = 0xCC;
            inline constexpr uint8_t TagPlayer = 0xCD;
            inline constexpr uint8_t TagGameObject = 0xCE;
            inline constexpr uint8_t End = 0xFF;
        }
    }

    namespace Movement
    {
        inline constexpr std::array<uint8_t, 7> StationaryUnitPrefix = { 0x84, 0, 0, 0, 0, 0, 0 };
        inline constexpr std::array<uint8_t, 8> StationaryUnitZeroBlock = { 0, 0, 0, 0, 0, 0, 0, 0 };
        inline constexpr std::array<uint8_t, 3> StationaryGameObjectFlags = { 0x81, 0x08, 0x00 };
        inline constexpr std::array<uint8_t, 7> ItemCreateHeader{};
        inline constexpr std::array<uint8_t, 3> PlayerCreatePrefix = { 0x8C, 0x00, 0x80 };
        inline constexpr uint32_t RootedMovementFlag = 0x00000400;
        inline constexpr std::size_t PlayerMovementPositionOffset = 12;
        inline constexpr std::size_t PlayerEntityPositionOffset = 167;
    }
}
