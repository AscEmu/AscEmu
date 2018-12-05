#pragma once

#include <cstdint>

#include "ByteBuffer.h"

typedef std::map<uint64_t, ByteBuffer*> SplineMap;

class SplineManager
{
    SplineMap m_splinePackets;
    void deleteExistingSplinePacket(uint64_t guid);
public:
    void addSplinePacket(uint64_t guid, ByteBuffer* packet);
    ByteBuffer* popSplinePacket(uint64_t guid);
    void clearSplinePackets();
};
