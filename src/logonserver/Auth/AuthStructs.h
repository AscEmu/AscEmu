/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AUTHSTRUCTS_H
#define AUTHSTRUCTS_H

#include <cstdint>

#pragma pack(push,1)
typedef struct
{
    uint8_t   cmd;
    uint8_t   error;          // 0x00
    uint16_t  size;           // 0x0026
    uint8_t   gamename[4];    // 'WoW'
    uint8_t   version1;       // 0x00
    uint8_t   version2;       // 0x08 (0.8.0)
    uint8_t   version3;       // 0x00
    uint16_t  build;          // 3734
    uint8_t   platform[4];    // 'x86'
    uint8_t   os[4];          // 'Win'
    uint8_t   country[4];     // 'enUS'
    uint32_t  timezone_bias;  // -419
    uint32_t  ip;             // client ip
    uint8_t   I_len;          // length of account name
    uint8_t   I[50];          // account name
} sAuthLogonChallenge_C;

typedef sAuthLogonChallenge_C sAuthReconnectChallenge_C;

typedef struct
{
    uint8_t   cmd;            // 0x00 CMD_AUTH_LOGON_CHALLENGE
    uint8_t   error;          // 0 - ok
    uint8_t   unk2;           // 0x00
    uint8_t   B[32];
    uint8_t   g_len;          // 0x01
    uint8_t   g;
    uint8_t   N_len;          // 0x20
    uint8_t   N[32];
    uint8_t   s[32];
    uint8_t   unk3[16];
    uint8_t   unk4;
} sAuthLogonChallenge_S;

typedef struct
{
    uint8_t   cmd;            // 0x01
    uint8_t   A[32];
    uint8_t   M1[20];
    uint8_t   crc_hash[20];
    uint8_t   number_of_keys;
    uint8_t   unk;
} sAuthLogonProof_C;

typedef struct
{
    uint16_t  unk1;
    uint32_t  unk2;
    uint8_t   unk3[4];
    uint16_t  unk4[20];       // sha1(A,g,?)
}  sAuthLogonProofKey_C;

typedef struct
{
    uint8_t   cmd;            // 0x01 CMD_AUTH_LOGON_PROOF
    uint8_t   error;
    uint8_t   M2[20];
    uint32_t  unk2;
    uint32_t  unk3;
    uint16_t  unk203;
} sAuthLogonProof_S;

#pragma pack(pop)

#endif  //AUTHSTRUCTS_H
