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

#include <Cryptography/MD5.hpp>
#include "Auth/AutoPatcher.h"
#include <fstream>
#include <Logging/Logger.hpp>

#include "CommonFilesystem.hpp"

PatchMgr& PatchMgr::getInstance()
{
    static PatchMgr mInstance;
    return mInstance;
}

void PatchMgr::initialize()
{
    // Load patches
    sLogger.info("PatchMgr : Loading Patches...");

    const std::string patchDirectory = "./ClientPatches";

    if (!fs::exists(patchDirectory) || !fs::is_directory(patchDirectory))
    {
        sLogger.warning("No patches found.");
        return;
    }

    for (const auto& entry : fs::directory_iterator(patchDirectory))
    {
        if (!entry.is_regular_file())
            continue;

        std::string filePath = entry.path().string();
        std::string fileName = entry.path().filename().string();
        uint32_t srcversion;
        char locality[5] = { 0 };

        if (sscanf(fileName.c_str(), "%4s%u.", locality, &srcversion) != 2)
            continue;

        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            sLogger.failure("Cannot open {}", filePath);
            continue;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        auto pPatch = std::make_unique<Patch>();
        pPatch->FileSize = static_cast<uint32_t>(size);
        pPatch->Data = std::make_unique<uint8_t[]>(pPatch->FileSize);
        pPatch->Version = srcversion;
        for (uint32_t i = 0; i < 4; ++i)
        {
            pPatch->Locality[i] = static_cast<char>(std::tolower(locality[i]));
        }

        pPatch->Locality[4] = '\0';
        pPatch->uLocality = *reinterpret_cast<uint32_t*>(pPatch->Locality);

        if (!file.read(reinterpret_cast<char*>(pPatch->Data.get()), pPatch->FileSize))
        {
            sLogger.failure("Cannot read {}", filePath);
            continue;
        }

        file.close();

        sLogger.info("PatchMgr : Found patch for {} locale `{}` ({} bytes).", srcversion, locality, pPatch->FileSize);

        // md5hash the file
        MD5Hash md5;
        md5.initialize();
        md5.updateData(pPatch->Data.get(), pPatch->FileSize);
        md5.finalize();
        std::memcpy(pPatch->MD5, md5.getDigest(), MD5_DIGEST_LENGTH);

        // add the patch to the patchlist
        m_patches.push_back(std::move(pPatch));
    }
}

Patch* PatchMgr::FindPatchForClient(uint32_t Version, const char* Locality)
{
    char tmplocality[5];
    Patch* fallbackPatch = nullptr;

    for (uint32_t i = 0; i < 4; ++i)
        tmplocality[i] = static_cast<char>(tolower(Locality[i]));

    tmplocality[4] = 0;
    uint32_t ulocality = *(uint32_t*)tmplocality;

    for (auto itr = m_patches.begin(); itr != m_patches.end(); ++itr)
    {
        // since localities are always 4 bytes we can do a simple int compare,
        // saving a string compare ;)
        if ((*itr)->uLocality == ulocality)
        {
            if (fallbackPatch == nullptr && (*itr)->Version == 0)
                fallbackPatch = (*itr).get();

            if ((*itr)->Version == Version)
                return (*itr).get();
        }
    }

    return fallbackPatch;
}

void PatchMgr::BeginPatchJob(Patch* pPatch, AuthSocket* pClient, uint32_t Skip)
{
    auto pJob = std::make_unique<PatchJob>(pPatch, pClient, Skip);
    pClient->m_patchJob = pJob.get();

    std::lock_guard lock(m_patchJobLock);
    m_patchJobs.push_back(std::move(pJob));
}

void PatchMgr::UpdateJobs()
{
    std::lock_guard lock(m_patchJobLock);

    for (auto itr = m_patchJobs.begin(); itr != m_patchJobs.end();)
    {
        std::list<std::unique_ptr<PatchJob>>::iterator itr2 = itr;
        ++itr;

        if (!(*itr2)->Update())
        {
            (*itr2)->GetClient()->m_patchJob = nullptr;
            m_patchJobs.erase(itr2);
        }
    }
}

void PatchMgr::AbortPatchJob(PatchJob* pJob)
{
    std::lock_guard lock(m_patchJobLock);

    for (auto itr = m_patchJobs.begin(); itr != m_patchJobs.end(); ++itr)
    {
        if ((*itr).get() == pJob)
        {
            m_patchJobs.erase(itr);
            break;
        }
    }
}

// this is what blizz sends.
// Data (1412 bytes)
// Data (91 bytes)
// 1412+91=1503 (minus header bytes, 3) = 1500

#define TRANSFER_CHUNK_SIZE 1500

#pragma pack(push,1)

struct TransferInitiatePacket
{
    uint8_t cmd;
    uint8_t strsize;
    char name[6];
    uint64_t filesize;
    uint8_t md5hash[MD5_DIGEST_LENGTH];
};

struct TransferDataPacket
{
    uint8_t cmd;
    uint16_t chunk_size;
};

#pragma pack(pop)

bool PatchJob::Update()
{
    // don't update unless the write buffer is empty
    m_client->BurstBegin();
    if (m_client->writeBuffer.GetSize() != 0)
    {
        m_client->BurstEnd();
        return true;
    }

    // send 1500 byte chunks
    TransferDataPacket header;
    header.cmd = 0x31;
    header.chunk_size = static_cast<uint16_t>((m_bytesLeft > 1500) ? 1500 : m_bytesLeft);
    //LogDebug("PatchJob : Sending %u byte chunk", header.chunk_size);

    bool result = m_client->BurstSend((const uint8_t*)&header, sizeof(TransferDataPacket));
    if (result)
    {
        result = m_client->BurstSend(m_dataPointer, header.chunk_size);
        if (result)
        {
            m_dataPointer += header.chunk_size;
            m_bytesSent += header.chunk_size;
            m_bytesLeft -= header.chunk_size;
        }
    }

    if (result)
        m_client->BurstPush();

    m_client->BurstEnd();

    // no need to check the result here, could just be a full buffer and not necessarily a fatal error.
    return (m_bytesLeft > 0) ? true : false;
}

bool PatchMgr::InitiatePatch(Patch* pPatch, AuthSocket* pClient)
{
    // send initiate packet
    TransferInitiatePacket init;
    init.cmd = 0x30;
    init.strsize = 5;
    init.name[0] = 'P';
    init.name[1] = 'a';
    init.name[2] = 't';
    init.name[3] = 'c';
    init.name[4] = 'h';
    init.name[5] = '\0';
    init.filesize = pPatch->FileSize;
    memcpy(init.md5hash, pPatch->MD5, MD5_DIGEST_LENGTH);

    // send it to the client
    pClient->BurstBegin();
    bool result = pClient->BurstSend((const uint8_t*)&init, sizeof(TransferInitiatePacket));
    if (result)
        pClient->BurstPush();

    pClient->BurstEnd();
    return result;
}
