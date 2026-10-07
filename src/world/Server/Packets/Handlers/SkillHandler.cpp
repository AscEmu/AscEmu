/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/Packets/ManagedPacket.h"
#include "Server/Packets/CmsgUnlearnSkill.h"
#include "Server/Packets/CmsgLearnTalent.h"
#include "Server/Packets/CmsgLearnTalentMultiple.h"
#include "Server/Packets/CmsgSetPrimaryTalentTree.h"
#include "Server/Packets/CmsgTraitsCommitConfig.h"
#include "Objects/Units/Players/Player.hpp"

using namespace AscEmu::Packets;


void WorldSession::handleUnlearnSkillOpcode(WorldPacket& recvPacket)
{
    CmsgUnlearnSkill srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    _player->removeSkillLine(static_cast<uint16_t>(srlPacket.skillLineId));
}

void WorldSession::handleLearnTalentOpcode(WorldPacket& recvPacket)
{
    CmsgLearnTalent srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (!srlPacket.talentIds.empty())
    {
        for (const auto talentId : srlPacket.talentIds)
            _player->learnTalent(talentId, 0);
    }
    else
    {
        _player->learnTalent(srlPacket.talentId, srlPacket.requestedRank);
    }

    _player->sendTalentsInfo();
}

void WorldSession::handleUnlearnTalents(WorldPacket& /*recvPacket*/)
{
    const uint32_t resetPrice = _player->calcTalentResetCost(_player->getTalentResetsCount());
    if (!_player->hasEnoughCoinage(resetPrice))
        return;

    _player->setTalentResetsCount(_player->getTalentResetsCount() + 1);
    _player->modCoinage(-static_cast<int32_t>(resetPrice));
    _player->resetTalents();
}


void WorldSession::handleLearnMultipleTalentsOpcode([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING < Cata
#if VERSION_STRING > TBC
    CmsgLearnTalentMultiple srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debug("Recieved CMSG_LEARN_TALENTS_MULTIPLE");

    for (auto learnTalent : srlPacket.multipleTalents)
        _player->learnTalent(learnTalent.talentId, learnTalent.talentRank);

    _player->sendTalentsInfo();
#endif
#endif
}

void WorldSession::handleLearnPreviewTalentsOpcode([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    int32_t current_tab;
    uint32_t talent_count;
    uint32_t talent_id;
    uint32_t talent_rank;
    //if currentTab -1 player has already the spec.
    recvPacket >> current_tab;
    recvPacket >> talent_count;

    for (uint32_t i = 0; i < talent_count; ++i)
    {
        recvPacket >> talent_id;
        recvPacket >> talent_rank;

        _player->learnTalent(talent_id, talent_rank);
    }

    _player->sendTalentsInfo();
#endif
}

void WorldSession::handleSetPrimaryTalentTreeOpcode([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING == Mop
    CmsgSetPrimaryTalentTree srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    _player->setPrimaryTalentSpecialization(srlPacket.specializationTabId);
#elif defined(AE_FOREVER)
// Forever currently reuses the shared specialization-selection packet parser.
    CmsgSetPrimaryTalentTree srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    _player->setPrimaryTalentSpecialization(srlPacket.specializationTabId);
#endif
}

void WorldSession::handleTraitsCommitConfigOpcode(WorldPacket& recvPacket)
{
    CmsgTraitsCommitConfig packet;
    if (!parsePacket(recvPacket, packet))
        return;

    if (!_player->getTraitManager().commitConfigUpdate(packet.config, packet.config.savedConfigId, packet.config.savedLocalIdentifier))
    {
        sLogger.debugOpcode("[ForeverDebug][Traits] commit rejected id={} type={} entries={}; failure response is not implemented for Forever", packet.config.id, static_cast<int32_t>(packet.config.type), packet.config.entries.size());
        return;
    }

    _player->saveToDB(false);
}

void WorldSession::handleCloseTraitSystemInteractionOpcode(WorldPacket& recvPacket)
{
    recvPacket.rfinish();
}

void WorldSession::handleTraitsTalentTestUnlearnSpellsOpcode(WorldPacket& recvPacket)
{
    // [UNKNOWN] Forever payload layout is not verified. Consume the packet without
    // applying changes until a capture proves the structure.
    sLogger.debugOpcode("[ForeverDebug][Traits] CMSG_TRAITS_TALENT_TEST_UNLEARN_SPELLS payload={} byte(s)", recvPacket.remaining());
    recvPacket.rfinish();
}
