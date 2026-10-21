// BfA (8.3.7), Shadowlands (9.2.7), Dragonflight (10.2.7), The War Within (11.2.7) and Midnight (12.1.5) extension of WorldSocket: the V2 world transport.
// Every packet travels behind a 16 byte header (size and AES-GCM tag), the encryption starts after
// SMSG_ENTER_ENCRYPTED_MODE was acknowledged. Application opcodes are resolved and dispatched through
// the central opcode table like on the classic path. The values of the client come from
// version/<profile>/World/WorldProfile.hpp.

protected:
    // Transport.
    bool sendAesInitializer();
    bool processAesInitializer();
    bool processAesPacket();
    bool writeAesPacket(uint32_t rawOpcode, const uint8_t* payload, uint32_t payloadSize);
    bool flushAesOutput();

    // Authentication with the pending Battle.net session.
    void handleAesAuthSession(WorldPacket& packet);
    bool sendAesEnterEncryptedMode();
    void handleAesEnterEncryptedModeAck();
    void completeAesAuthentication();

    // Second connection of a session: the client opens it after SMSG_CONNECT_TO and sends the
    // movement, query and interaction opcodes through it.
    void handleAesAuthContinuedSession(WorldPacket& packet);
    void attachAesInstanceConnection();

public:
    bool sendConnectTo(uint64_t key, uint32_t serial);
    bool isInstanceConnection() const { return m_aesInstanceConnection; }

private:
    enum class AesWorldState : uint8_t
    {
        Disabled,
        AwaitClientInitializer,
        AwaitAuthSession,
        AwaitEncryptionAck,
        Authenticated
    };

    AesWorldState m_aesWorldState{AesWorldState::Disabled};

    // header of the packet that is being received: size with the opcode and the tag
    bool m_aesHeaderReady{false};
    uint32_t m_aesPacketSize{0};
    AesGcmCrypt::Tag m_aesPacketTag{};

    AesGcmCrypt m_aesCrypt;

    // the challenge and the encryption key of the profile: 16 bytes up to 10.x, 32 bytes from 11.x on
    std::array<uint8_t, 32> m_aesServerChallenge{};
    std::array<uint8_t, 40> m_aesSessionKey{};
    std::array<uint8_t, 32> m_aesEncryptKey{};

    bool m_aesInstanceConnection{false};
    uint64_t m_aesConnectToKey{0};

    // account of the verified session until the encryption is enabled
    uint32_t m_aesGameAccountId{0};
    std::string m_aesGameAccountName;
    // the session the logonserver announced, its account data completes the authentication
    PendingWorldSession m_aesSession;

    // frames that did not fit into the send buffer, kept in order because of the packet counters
    std::vector<uint8_t> m_aesPendingOutput;

    // raw deflate stream of the compressed packets, one per connection
    std::shared_ptr<void> m_aesCompressionStream;
