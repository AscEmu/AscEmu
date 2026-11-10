/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include <cstdint>
#include <vector>

#include "UpdateManager.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Opcodes.hpp"
#include "Server/WorldSession.h"
#include "Server/World.h"

UpdateManager::UpdateManager(Player* owner, size_t compressionThreshold, size_t creationBufferInitialSize, size_t updateBufferInitialSize, size_t outOfRangeIdsInitialSize)
    :
    m_compressionThreshold(compressionThreshold),
    m_creationBuffer(creationBufferInitialSize),
    m_creationCount(0),
    m_updateBuffer(updateBufferInitialSize),
    m_processPending(false),
    m_outOfRangeIds(outOfRangeIdsInitialSize),
    m_updateCount(0),
    m_outOfRangeIdCount(0),
    m_owner(owner)
{
}

void UpdateManager::clearPendingUpdates()
{
    std::scoped_lock lock(m_mutexUpdateBuffer, m_mutexDelayedPackets);

    m_processPending = false;

    m_creationCount = 0;
    m_updateCount = 0;
    m_outOfRangeIdCount = 0;

    m_creationBuffer.clear();
    m_updateBuffer.clear();
    m_outOfRangeIds.clear();
}

void UpdateManager::pushCreationData(const ByteBuffer* data, uint32_t updateCount)
{
    std::scoped_lock lock(m_mutexUpdateBuffer, m_mutexDelayedPackets);

    internalPushUpdatesIfBufferIsFull(data->size());

    m_creationCount += updateCount;
    m_creationBuffer.append(*data);

    internalUpdateMapMgr();
}

void UpdateManager::pushOutOfRangeGuid(const WoWGuid& guid)
{
    std::scoped_lock lock(m_mutexUpdateBuffer, m_mutexDelayedPackets);

    internalPushUpdatesIfBufferIsFull(static_cast<size_t>(8));

    // Classic / TBC
    if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK))
    {
        if (guid.isGameObject() && !guid.isTransport() && !guid.isTransporter())
            m_owner->sendDestroyObjectPacket(guid.getRawGuid());
    }

    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_WoD))
    {
        m_outOfRangeIds << guid.toGuid128(worldConfig.realm.id, m_owner->GetMapId());
    }
    else
    {
        m_outOfRangeIds << guid;
    }

    ++m_outOfRangeIdCount;

    internalUpdateMapMgr();
}

void UpdateManager::pushUpdateData(const ByteBuffer* data, uint32_t updateCount)
{
    std::scoped_lock lock(m_mutexUpdateBuffer, m_mutexDelayedPackets);

    internalPushUpdatesIfBufferIsFull(data->size());

    m_updateCount += updateCount;
    m_updateBuffer.append(*data);

    internalUpdateMapMgr();
}

void UpdateManager::processPendingUpdates()
{
    std::scoped_lock lock(m_mutexUpdateBuffer, m_mutexDelayedPackets);

    internalProcessPendingUpdates();
}

void UpdateManager::queueDelayedPacket(std::unique_ptr<WorldPacket> packet)
{
    {
        std::scoped_lock lock(m_mutexDelayedPackets);
        m_delayedPackets.emplace_back(std::move(packet));
    }

    // Delayed packets are pending work on their own. Schedule the owner for
    // processing even when no object create/value/out-of-range update exists.
    internalUpdateMapMgr();
}

size_t UpdateManager::calculateBufferSize() const
{
    const size_t baseSize = 10 + (m_outOfRangeIds.size() * 9);

    if (m_creationBuffer.size() > m_updateBuffer.size())
        return m_creationBuffer.size() + baseSize;

    return m_updateBuffer.size() + baseSize;
}

bool UpdateManager::readyForUpdate() const
{
    return m_creationBuffer.size() != 0 || m_updateBuffer.size() != 0 || m_outOfRangeIds.size() != 0 || !m_delayedPackets.empty();
}

void UpdateManager::internalProcessPendingUpdates()
{
    if (!readyForUpdate())
        return;

    ByteBuffer buffer(calculateBufferSize());

    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_WoD))
    {
        // 6.x and 7.x: block count, map, the objects that left the range, size of the blocks, the blocks;
        // 11.x: map, block count, a set bit, then the objects that left the range
        const auto sendBlocks = [this, &buffer](uint32_t blockCount, ByteBuffer& blocks, bool withOutOfRange)
        {
            buffer.clear();
            if (WoW::isServerExpansionAtLeast(WoW::Expansion::_TheWarWithin))
            {
                buffer << uint16_t(m_owner->GetMapId());
                buffer << uint32_t(blockCount);
                buffer.writeBit(true);
            }
            else
            {
                buffer << uint32_t(blockCount);
                buffer << uint16_t(m_owner->GetMapId());
            }

            const bool hasOutOfRange = withOutOfRange && m_outOfRangeIdCount > 0;
            buffer.writeBit(hasOutOfRange);
            buffer.flushBits();

            if (hasOutOfRange)
            {
                buffer << uint16_t(m_outOfRangeIdCount); // removed at once, without fading out
                buffer << uint32_t(m_outOfRangeIdCount);
                buffer.append(m_outOfRangeIds);
                m_outOfRangeIds.clear();
                m_outOfRangeIdCount = 0;
            }

            buffer << uint32_t(blocks.size());
            buffer.append(blocks);

            WorldPacket packet(SMSG_UPDATE_OBJECT, buffer.wpos());
            packet.append(buffer.contents(), buffer.wpos());
            m_owner->getSession()->SendPacket(&packet);
        };

        if (m_creationBuffer.size() > 0 || m_outOfRangeIdCount > 0)
        {
            sendBlocks(m_creationCount, m_creationBuffer, true);
            m_creationBuffer.clear();
            m_creationCount = 0;
        }

        if (m_updateBuffer.size() > 0)
        {
            sendBlocks(m_updateCount, m_updateBuffer, false);
            m_updateBuffer.clear();
            m_updateCount = 0;
        }
    }
    else
    {
        if (m_creationBuffer.size() > 0 || m_outOfRangeIdCount > 0)
        {
            if (WoW::isServerExpansionAtLeast(WoW::Expansion::_Cata))
                buffer << uint16_t(m_owner->GetMapId());

            if (m_outOfRangeIds.size() > 0)
                buffer << uint32_t(m_creationCount + 1);
            else
                buffer << uint32_t(m_creationCount);


            if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK))
                buffer << uint8_t(1);

            if (m_outOfRangeIdCount > 0)
            {
                buffer << getWireUpdateType(UPDATETYPE_OUT_OF_RANGE_OBJECTS);
                buffer << uint32_t(m_outOfRangeIdCount);
                buffer.append(m_outOfRangeIds);
                m_outOfRangeIds.clear();
                m_outOfRangeIdCount = 0;
            }

            if (m_creationBuffer.size() > 0)
            {
                buffer.append(m_creationBuffer);
                m_creationBuffer.clear();
                m_creationCount = 0;
            }

            auto sent_packet = false;
            if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_Cata))
            {
                if (buffer.wpos() > m_compressionThreshold)
                    sent_packet = m_owner->compressAndSendUpdateBuffer(uint32_t(buffer.wpos()), buffer.contents());
            }

            if (!sent_packet)
                m_owner->getSession()->OutPacket(SMSG_UPDATE_OBJECT, uint16_t(buffer.wpos()), buffer.contents());
        }

        if (m_updateBuffer.size() > 0)
        {
            buffer.clear();

            if (WoW::isServerExpansionAtLeast(WoW::Expansion::_Cata))
                buffer << uint16_t(m_owner->GetMapId());

            if (m_outOfRangeIds.size() > 0)
                buffer << uint32_t(m_updateCount + 1);
            else
                buffer << uint32_t(m_updateCount);

            if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK))
                buffer << uint8_t(1);

            buffer.append(m_updateBuffer);
            m_updateBuffer.clear();
            m_updateCount = 0;

            auto sent_packet = false;
            if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_Cata))
            {
                if (buffer.wpos() > m_compressionThreshold)
                    sent_packet = m_owner->compressAndSendUpdateBuffer(uint32_t(buffer.wpos()), buffer.contents());
            }

            if (!sent_packet)
                m_owner->getSession()->OutPacket(SMSG_UPDATE_OBJECT, uint16_t(buffer.wpos()), buffer.contents());
        }
    }

    m_processPending = false;
    internalSendDelayedPackets();
}

void UpdateManager::internalSendDelayedPackets()
{
    const auto session = m_owner->getSession();
    for (const auto& packet : m_delayedPackets)
        session->SendPacket(packet.get());

    m_delayedPackets.clear();
}

void UpdateManager::internalPushUpdatesIfBufferIsFull(size_t additionalDataSize)
{
    if (additionalDataSize + calculateBufferSize() >= m_maxUpdateSize)
        internalProcessPendingUpdates();
}

void UpdateManager::internalUpdateMapMgr()
{
    if (const auto mapMgr = m_owner->getWorldMap(); mapMgr != nullptr && !m_processPending)
    {
        m_processPending = true;
        mapMgr->pushToProcessed(m_owner);
    }
}
