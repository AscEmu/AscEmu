/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <array>
#include <cstdint>

namespace cp::patches
{

    namespace windows
    {
        namespace x86
        {
            inline constexpr std::array<std::uint8_t, 3> Send  {{0x33, 0xC0, 0x90}};
            inline constexpr std::array<std::uint8_t, 3> Email {{0x90, 0x90, 0xEB}};
            inline constexpr std::array<std::uint8_t, 1> User  {{0x00}};
            inline constexpr std::array<std::uint8_t, 5> RaF   {{0x90, 0x90, 0x90, 0x90, 0x90}};
            inline constexpr std::array<std::uint8_t, 3> Rcv   {{0xFF, 0x0F, 0x84}};
        }

        namespace x64
        {
            inline constexpr std::array<std::uint8_t, 5> Send  {{0x45, 0x31, 0xC0, 0x90, 0x90}};
            inline constexpr std::array<std::uint8_t, 1> Email {{0xEB}};
            inline constexpr std::array<std::uint8_t, 1> User  {{0x00}};
            inline constexpr std::array<std::uint8_t, 5> RaF   {{0x90, 0x90, 0x90, 0x90, 0x90}};
            inline constexpr std::array<std::uint8_t, 3> Rcv   {{0xFF, 0x0F, 0x84}};
        }
    } // namespace windows

    namespace mac
    {
        namespace x86
        {
            inline constexpr std::array<std::uint8_t, 3> Send  {{0x31, 0xC9, 0x90}};
            inline constexpr std::array<std::uint8_t, 1> Email {{0xEB}};
            inline constexpr std::array<std::uint8_t, 1> User  {{0x00}};
            inline constexpr std::array<std::uint8_t, 5> RaF   {{0x31, 0xC0, 0xEB, 0x72, 0x90}};
            inline constexpr std::array<std::uint8_t, 3> Rcv   {{0xFF, 0x0F, 0x84}};
        }

        namespace x64
        {
            inline constexpr std::array<std::uint8_t, 3> Send  {{0x31, 0xC9, 0x90}};
            inline constexpr std::array<std::uint8_t, 1> Email {{0xEB}};
            inline constexpr std::array<std::uint8_t, 1> User  {{0x00}};
            inline constexpr std::array<std::uint8_t, 7> RaF   {{0x31, 0xC0, 0xE9, 0xE5, 0x02, 0x00, 0x00}};
            inline constexpr std::array<std::uint8_t, 3> Rcv   {{0xFF, 0x0F, 0x84}};
        }
    } // namespace mac

} // namespace cp::patches
