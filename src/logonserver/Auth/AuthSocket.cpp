/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AuthSocket.h"
#include "Logging/Logger.hpp"

#include <openssl/md5.h>

void AuthSocket::sendAuthProof(Sha1Hash sha)
{
    sLogger.debug(" called.");

    if (m_challenge.build == 5875)
    {
        sAuthLogonProof_S proof;
        memcpy(proof.M2, sha.getDigest(), 20);
        proof.cmd = 1;
        proof.error = 0;
        proof.unk2 = 0;

        Send(reinterpret_cast<uint8_t*>(&proof), sizeof(sAuthLogonProof_S) - 6);
    }
    else
    {
        sAuthLogonProof_S proof;
        memcpy(proof.M2, sha.getDigest(), 20);
        proof.cmd = 1;
        proof.error = 0;
        proof.unk2 = 0;
        proof.unk3 = 0;
        proof.unk203 = 0;

        Send(reinterpret_cast<uint8_t*>(&proof), sizeof(sAuthLogonProof_S));
    }
}
