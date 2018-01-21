/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifdef AE_TBC
#define REALM_LIST 16
void InformationCore::writeRealmDataTbc(AuthSocket* socket)
{
    realmLock.Acquire();

    ByteBuffer data(m_realms.size());
    data << uint8_t(0x10);
    // Size placeholder
    data << uint16_t(0);

    data << uint32_t(0);
    data << uint16_t(m_realms.size());

    for (const auto realm_pair : m_realms)
    {
        const auto realm = realm_pair.second;
        if (realm->GameBuild != socket->GetChallenge()->build)
            continue;

        const auto character_itr = realm->CharacterMap.find(socket->GetAccountID());
        const uint8_t character_count = character_itr == realm->CharacterMap.end() ? 0 : character_itr->second;

        data << uint8_t(realm->Icon);
        data << realm->Lock;
        data << uint8_t(realm->flags);
        data << realm->Name;
        data << realm->Address;
        data << realm->Population;
        data << character_count;
        data << uint8_t(realm->TimeZone);
        data << uint8_t(GetRealmIdByName(realm->Name));

        if (realm->flags & REALM_FLAG_SPECIFYBUILD)
        {
            data << socket->GetChallenge()->version1;
            data << socket->GetChallenge()->version2;
            data << socket->GetChallenge()->version3;
            data << socket->GetChallenge()->build;
        }
    }

    data << uint16_t(0x17);
    *(uint16_t*)&data.contents()[1] = uint16_t(data.size() - 3);
    realmLock.Release();

    // Send to the socket.
    socket->Send(static_cast<const uint8*>(data.contents()), uint32(data.size()));

    std::list< LogonCommServerSocket* > server_sockets;

    serverSocketLock.Acquire();
    {
        if (m_serverSockets.empty())
        {
            serverSocketLock.Release();
            return;
        }

        for (const auto server_socket : m_serverSockets)
        {
            server_sockets.push_back(server_socket);
        }
    }
    serverSocketLock.Release();

    for (const auto server_socket : server_sockets)
    {
        server_socket->RefreshRealmsPop();
    }
}
#endif
