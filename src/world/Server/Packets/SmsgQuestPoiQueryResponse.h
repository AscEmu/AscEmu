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
        size_t expectedSize() const override
        {
            if (m_protocol.isForever())
                return 8 + 8 * questCount;

            return 4 + (4 + 4) * questCount;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                // Verified with build 70124. The response starts with the result count
                // twice, followed by QuestPOIData. Each blob uses the modern fixed
                // fields, int16 XYZ points and one AlwaysAllowMergingBlobs bit.
                packet << static_cast<int32_t>(questCount);
                packet << static_cast<int32_t>(questCount);

                for (uint32_t i = 0; i < questCount; ++i)
                {
                    const uint32_t questId = i < questIds.size() ? questIds[i] : 0;
                    QuestProperties const* q = sMySQLStore.getQuestProperties(questId);
                    QuestPOIVector const* poi = q != nullptr ? sQuestMgr.getQuestPOIMap(questId) : nullptr;

                    packet << static_cast<int32_t>(questId);
                    packet << static_cast<int32_t>(poi != nullptr ? poi->size() : 0);

                    if (poi == nullptr)
                        continue;

                    for (QuestPOI const& blob : *poi)
                    {
                        int32_t questObjectiveId = static_cast<int32_t>(blob.QuestObjectiveId);
                        int32_t questObjectId = 0;

                        const auto objectives = sQuestMgr.buildForeverQuestObjectives(q, 0);
                        for (ForeverQuestObjectiveData const& objective : objectives)
                        {
                            if (objective.storageIndex != blob.ObjectiveIndex)
                                continue;

                            if (questObjectiveId == 0)
                                questObjectiveId = static_cast<int32_t>(objective.id);
                            questObjectId = objective.objectId;
                            break;
                        }

                        packet << static_cast<int32_t>(blob.PoiId);              // BlobIndex
                        packet << static_cast<int32_t>(blob.ObjectiveIndex);     // ObjectiveIndex
                        packet << questObjectiveId;                              // QuestObjectiveID
                        packet << questObjectId;                                 // QuestObjectID
                        packet << static_cast<int32_t>(blob.MapId);              // MapID
                        packet << static_cast<int32_t>(blob.MapAreaId);          // UiMapID
                        packet << static_cast<int32_t>(blob.Unk3);               // Priority
                        packet << static_cast<int32_t>(blob.Unk4);               // Flags
                        packet << int32_t(0);                                    // WorldEffectID
                        packet << static_cast<int32_t>(blob.PlayerConditionId);   // PlayerConditionID
                        packet << static_cast<int32_t>(blob.NavigationPlayerConditionId); // NavigationPlayerConditionID
                        packet << int32_t(0);                                    // SpawnTrackingID
                        packet << static_cast<int32_t>(blob.points.size());

                        for (QuestPOIPoint const& point : blob.points)
                        {
                            packet << static_cast<int16_t>(point.x);
                            packet << static_cast<int16_t>(point.y);
                            packet << static_cast<int16_t>(point.z);
                        }

                        packet.writeBit(false);                                  // AlwaysAllowMergingBlobs
                        packet.flushBits();
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
