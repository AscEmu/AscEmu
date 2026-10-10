/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Cryptography/WowCrypt.hpp"
#if AE_WORLD_PROFILE_BFA || AE_WORLD_PROFILE_SHADOWLANDS || AE_WORLD_PROFILE_DRAGONFLIGHT
#include "Cryptography/AesGcmCrypt.hpp"
#endif
#include "Network/WorldPacket.hpp"
#include "Network/Network.hpp"
#include "ClientProtocol.hpp"
#include "Threading/ThreadSafeQueue.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class SocketHandler;
class WorldSession;

class SERVER_DECL WorldSocket : public Socket
{
public:
    WorldSocket(SOCKET fd);
    ~WorldSocket();

    //////////////////////////////////////////////////////////////////////////////////////////
    // virtual functions (Socket)
    void onRead() override;
    void onConnect() override;
    void onDisconnect() override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // helper for protocol
    void setClientProtocol(WoW::ClientProtocol protocol);
    WoW::ClientProtocol getClientProtocol();
    void setCurrentVersionAsProtocol();
    void setClientProtocolByBuild(uint32_t build);


    //////////////////////////////////////////////////////////////////////////////////////////
    // packet sending SERVER->CLIENT
    void outPacket(uint32_t opcode, size_t len, const void* data);
    uint8_t _outPacket(uint32_t opcode, size_t len, const void* data);
    void updateQueuedPackets();

    void sendPacket(WorldPacket* packet);

    void sendUpdateQueuePosition(uint32_t Position);
    void sendAuthenticated(std::unique_ptr<WorldSession> sessionHolder);
    void sendClientConnectionPacket();

    template <typename TPacket>
    std::unique_ptr<WorldPacket> buildPacket(TPacket& managedPacket)
    {
        managedPacket.setClientProtocol(m_protocol);
        return managedPacket.serialise();
    }

    template <typename TPacket>
    void sendManagedPacket(TPacket& managedPacket)
    {
        auto packet = buildPacket(managedPacket);
        sendPacket(packet.get());
    }

protected:
    void sendAuthChallengePacket();
    void sendVerifyConnectPacket();

    // Version-specific socket adapters. Legacy uses the no-op stub; Battle.net
    // profiles provide their world transport from src/version.
    bool initializeVersionedConnection();
    bool processVersionedRead();
    bool sendVersionedPacket(WorldPacket* packet);

    //////////////////////////////////////////////////////////////////////////////////////////
    // packet receiving CLIENT->SERVER (after onRead from Socket class)
public:
    bool processHeader();
    void dispatchPacket(std::unique_ptr<WorldPacket> packet);

    template <typename TPacket>
    bool parsePacket(WorldPacket& packet, TPacket& managedPacket)
    {
        managedPacket.setClientProtocol(m_protocol);
        return managedPacket.deserialise(packet);
    }

protected:
    void handleAuthSession(std::unique_ptr<WorldPacket> recvPacket);
    void handlePing(std::unique_ptr<WorldPacket> recvPacket);
    void handleMsgVerifyConnection(std::unique_ptr<WorldPacket> recvPacket);

    // creates the session of an authenticated account, shared by all login paths
    void completeAuthentication(uint32_t accountId, const std::string& accountName, std::string gmFlags, uint8_t accountFlags, const std::string& lang, uint32_t muted);


    //////////////////////////////////////////////////////////////////////////////////////////
    // used by LogonCommClient
public:
    void informationRetreiveCallback(WorldPacket& recvData, uint32_t requestid);
    bool isAuthenticated{false};
    bool m_protocolSetByLogonComm{ true };

    //////////////////////////////////////////////////////////////////////////////////////////
    // member helpers
public:
    inline void setSession(WorldSession* session) { m_session = session; }
    inline WorldSession* getSession() { return m_session; }


private:
    uint32_t m_opcode{0};
    uint32_t m_remaining{0};
    uint32_t m_size{0};

    WoW::ClientProtocol m_protocol{};

    uint32_t m_socketSeed{0};
    uint32_t m_clientSeed{0};
    WowCrypt m_crypt;

    std::string m_accountName;
    ByteBuffer m_addonInfoBuffer;
    uint8_t m_authDigest[20]{};
    uint32_t m_clientBuild{0};
    
    uint32_t m_requestId{0};
    bool m_handshakeReceived{false};

    ThreadSafeQueue<std::unique_ptr<WorldPacket>> m_queue;
    bool m_queued{false};

    uint32_t m_latency{0};
    bool m_nagleEanbled{false};

    WorldSession* m_session{nullptr};

#if AE_WORLD_PROFILE_FOREVER
#include "version/Forever/World/WorldSocketForever.inc"
#elif AE_WORLD_PROFILE_WOD || AE_WORLD_PROFILE_LEGION
#include "version/Shared/World/WorldSocketRc4.inc"
#elif AE_WORLD_PROFILE_BFA || AE_WORLD_PROFILE_SHADOWLANDS || AE_WORLD_PROFILE_DRAGONFLIGHT
#include "version/Shared/World/WorldSocketAes.inc"
#endif
};
