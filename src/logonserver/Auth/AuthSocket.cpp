/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "LogonStdAfx.h"
#include "AuthSocket.h"
#include <openssl/md5.h>

void AuthSocket::sendAuthProof(Sha1Hash sha)
{
    LOG_DEBUG(" called.");

    if (m_challenge.build == 5875)
    {
        sAuthLogonProof_S proof;
        memcpy(proof.M2, sha.GetDigest(), 20);
        proof.cmd = 1;
        proof.error = 0;
        proof.unk2 = 0;

        Send(reinterpret_cast<uint8*>(&proof), sizeof(sAuthLogonProof_S) - 6);
    }
    else
    {
        sAuthLogonProof_S proof;
        memcpy(proof.M2, sha.GetDigest(), 20);
        proof.cmd = 1;
        proof.error = 0;
        proof.unk2 = 0;
        proof.unk3 = 0;
        proof.unk203 = 0;

        Send(reinterpret_cast<uint8*>(&proof), sizeof(sAuthLogonProof_S));
    }
}
