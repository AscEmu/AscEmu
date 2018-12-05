#include "SplineManager.h"

void SplineManager::deleteExistingSplinePacket(uint64_t guid)
{
    const auto iter = m_splinePackets.find(guid);
    if (iter == m_splinePackets.end())
    {
        return;
    }

    delete iter->second;
    m_splinePackets.erase(iter);
}

void SplineManager::addSplinePacket(uint64_t guid, ByteBuffer * packet)
{
    deleteExistingSplinePacket(guid);
    m_splinePackets.insert(SplineMap::value_type(guid, packet));
}

ByteBuffer* SplineManager::popSplinePacket(uint64_t guid)
{
    const auto iter = m_splinePackets.find(guid);
    if (iter == m_splinePackets.end())
    {
        return nullptr;
    }

    const auto packet = iter->second;
    m_splinePackets.erase(iter);
    return packet;
}

void SplineManager::clearSplinePackets()
{
    for (const auto packet : m_splinePackets)
    {
        delete packet.second;
    }

    m_splinePackets.clear();
}
