/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Cryptography/WowCrypt.hpp"
#include "Network/WorldPacket.hpp"
#include "Network/Network.hpp"
#include "ClientProtocol.hpp"
#include "Threading/ThreadSafeQueue.hpp"

#include <string>

class SocketHandler;
class WorldSession;

class WorldSocket : public Socket
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
    // helper for protocol setup
    void setClientProtocol(WoW::ClientProtocol protocol);
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
};
