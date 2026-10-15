/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/QuestMgr.h"
#include "Storage/MySQLDataStore.hpp"

#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    class SmsgQuestPoiQueryResponse : public ManagedPacket
    {
    public:
        uint32_t questCount;
        std::vector<uint32_t> questIds;

        SmsgQuestPoiQueryResponse() : SmsgQuestPoiQueryResponse(0, {})
        {
        }

        SmsgQuestPoiQueryResponse(uint32_t questCount, std::vector<uint32_t> questIds) :
            ManagedPacket(SMSG_QUEST_POI_QUERY_RESPONSE, 4 + (4 + 4) * questCount),
            questCount(questCount),
            questIds(std::move(questIds))
        {
        }

    protected:
        size_t expectedSize() const override { return 4 + (4 + 4) * questCount; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // quests with their blobs; 8.x and later address the blobs by ui map, the world map areas of the data
                // build do not fit there, these quests go out without blobs
                const bool withBlobs = m_protocol.isWoD() || m_protocol.isLegion();
                const size_t count = questIds.size() < questCount ? questIds.size() : questCount;

                packet << int32_t(count);
                packet << int32_t(count);
                for (size_t i = 0; i < count; ++i)
                {
                    const uint32_t questId = questIds[i];
                    QuestPOIVector const* poi = nullptr;
                    if (withBlobs && sMySQLStore.getQuestProperties(questId) != nullptr)
                        poi = sQuestMgr.getQuestPOIMap(questId);

                    packet << int32_t(questId);
                    packet << int32_t(poi != nullptr ? poi->size() : 0);
                    if (poi == nullptr)
                        continue;

                    for (const auto& blob : *poi)
                    {
                        packet << int32_t(blob.PoiId);
                        packet << int32_t(blob.ObjectiveIndex);
                        packet << int32_t(blob.ObjectiveIndex >= 0 ? static_cast<int32_t>((questId << 4) | static_cast<uint32_t>(blob.ObjectiveIndex)) : 0);
                        packet << int32_t(0);               // quest object
                        packet << int32_t(blob.MapId);
                        packet << int32_t(blob.MapAreaId);
                        packet << int32_t(blob.FloorId);
                        packet << int32_t(blob.Unk3);       // priority
                        packet << int32_t(blob.Unk4);       // flags
                        packet << int32_t(0);               // world effect
                        packet << int32_t(0);               // player condition
                        packet << int32_t(0);               // spawn tracking
                        packet << int32_t(blob.points.size());
                        for (const auto& point : blob.points)
                            packet << int32_t(point.x) << int32_t(point.y);
                        if (m_protocol.isLegion())
                        {
                            packet.writeBit(false);         // always allow merging blobs
                            packet.flushBits();
                        }
                    }
                }
                return true;
            }

            if (!m_protocol.isMop())
            {
                packet << questCount;
                for (auto questId : questIds)
                {
                    QuestProperties const* q = sMySQLStore.getQuestProperties(questId);
                    if (q != nullptr)
                    {
                        QuestPOIVector const* POI = sQuestMgr.getQuestPOIMap(questId);

                        if (POI != nullptr)
                        {
                            packet << uint32_t(questId);
                            packet << uint32_t(POI->size());

                            for (QuestPOIVector::const_iterator iterator = POI->begin(); iterator != POI->end(); ++iterator)
                            {
                                packet << uint32_t(iterator->PoiId);
                                packet << int32_t(iterator->ObjectiveIndex);
                                packet << uint32_t(iterator->MapId);
                                packet << uint32_t(iterator->MapAreaId);
                                packet << uint32_t(iterator->FloorId);
                                packet << uint32_t(iterator->Unk3);
                                packet << uint32_t(iterator->Unk4);
                                packet << uint32_t(iterator->points.size());

                                for (std::vector< QuestPOIPoint >::const_iterator itr2 = iterator->points.begin(); itr2 != iterator->points.end(); ++itr2)
                                {
                                    packet << int32_t(itr2->x);
                                    packet << int32_t(itr2->y);
                                }
                            }

                        }
                        else
                        {
                            packet << uint32_t(questId);
                            packet << uint32_t(0);
                        }

                    }
                    else
                    {
                        packet << uint32_t(questId);
                        packet << uint32_t(0);
                    }
                }

                return true;
            }
            else
            {
                ByteBuffer buffer;

                packet.writeBits(questCount, 20);
                for (auto questId : questIds)
                {
                    QuestProperties const* q = sMySQLStore.getQuestProperties(questId);
                    if (q != nullptr)
                    {
                        QuestPOIVector const* POI = sQuestMgr.getQuestPOIMap(questId);

                        if (POI != nullptr)
                        {
                            packet.writeBits(POI->size(), 18); 

                            for (QuestPOIVector::const_iterator iterator = POI->begin(); iterator != POI->end(); ++iterator)
                            {
                                packet.writeBits(iterator->points.size(), 21); 
                                buffer << uint32_t(iterator->FloorId);

                                for (std::vector< QuestPOIPoint >::const_iterator itr2 = iterator->points.begin(); itr2 != iterator->points.end(); ++itr2)
                                {
                                    buffer << int32_t(itr2->x);
                                    buffer << int32_t(itr2->y);
                                }

                                buffer << int32_t(iterator->ObjectiveIndex);
                                buffer << uint32_t(iterator->PoiId);
                                buffer << uint32_t(0);      // unk
                                buffer << uint32_t(0);      // unk
                                
                                buffer << uint32_t(iterator->MapId);
                                buffer << uint32_t(iterator->points.size());
                                buffer << uint32_t(iterator->MapAreaId);
                                buffer << uint32_t(0);      // unk
                                buffer << uint32_t(iterator->Unk4);
                                buffer << uint32_t(iterator->Unk3);
                            }

                            buffer << uint32_t(questId);
                            buffer << uint32_t(POI->size());

                        }
                        else
                        {
                            buffer << uint32_t(questId);
                            buffer << uint32_t(0);

                            packet.writeBits(0, 18);
                        }

                    }
                    else
                    {
                        buffer << uint32_t(questId);
                        buffer << uint32_t(0);

                        packet.writeBits(0, 18);
                    }

                    buffer << uint32_t(questCount);
                    packet.flushBits();
                    packet.append(buffer);
                }

                return true;
            }
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
