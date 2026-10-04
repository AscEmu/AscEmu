// WoD (6.2.4) and Legion (7.3.5) extension of WorldSocket.
// Both clients share this world transport, the differences come from
// version/<profile>/World/WorldProfile.hpp. Application opcodes are resolved and dispatched
// through the central opcode table like on the classic path.

protected:
    // Transport.
    bool sendRc4Initializer();
    bool processRc4Initializer();
    bool processRc4Packet();
    bool writeRc4Packet(uint16_t rawOpcode, const uint8_t* payload, uint32_t payloadSize);
    bool flushRc4Output();

    // Authentication with the pending Battle.net session.
    void handleRc4AuthSession(WorldPacket& packet);
    void handleRc4EnableEncryptionAck();
    void completeRc4Authentication();

    // Second connection of a session: the client opens it after SMSG_CONNECT_TO and sends the
    // movement, query and interaction opcodes through it.
    void handleRc4AuthContinuedSession(WorldPacket& packet);

public:
    bool sendConnectTo(uint64_t key, uint32_t serial);
    bool isInstanceConnection() const { return m_rc4InstanceConnection; }

private:
    enum class Rc4WorldState : uint8_t
    {
        Disabled,
        AwaitClientInitializer,
        AwaitAuthSession,
        AwaitEncryptionAck,
        Authenticated
    };

    Rc4WorldState m_rc4WorldState{Rc4WorldState::Disabled};

    // clients with a setup header switch to the 6 byte header after CMSG_AUTH_SESSION
    bool m_rc4LargeClientHeader{false};
    bool m_rc4HeaderReady{false};
    uint16_t m_rc4PacketOpcode{0};
    uint32_t m_rc4PacketSize{0};

    std::array<uint8_t, 16> m_rc4ServerChallenge{};
    std::array<uint8_t, 40> m_rc4SessionKey{};

    // seeds of the auth challenge: encryption and decryption of a second connection
    std::array<uint8_t, 32> m_rc4ChallengeSeeds{};
    bool m_rc4InstanceConnection{false};
    uint64_t m_rc4ConnectToKey{0};

    // account of the verified session until the encryption is enabled
    uint32_t m_rc4GameAccountId{0};
    std::string m_rc4GameAccountName;

    // frames that did not fit into the send buffer, kept in order because of the stream cipher
    std::vector<uint8_t> m_rc4PendingOutput;

    // raw deflate stream of the compressed packets, one per connection
    std::shared_ptr<void> m_rc4CompressionStream;
