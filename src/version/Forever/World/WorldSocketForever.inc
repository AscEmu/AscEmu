// Forever-only extension of WorldSocket.
// Modern World-V2 transport stays on WorldSocket; all application opcodes are
// resolved and dispatched through AscEmu's central opcode table/registry.

public:
    uint32_t getForeverRealmId() const { return m_foreverRealmId; }
    uint32_t getForeverRegionId() const { return m_foreverRegionId; }
    uint32_t getForeverBattlegroupId() const { return m_foreverBattlegroupId; }

    bool sendForeverPacket(uint32_t internalOpcode, const uint8_t* payload, uint32_t payloadSize);
    bool beginForeverInstanceLogin(uint32_t guidLow);

protected:
    // Transport/auth.
    bool sendForeverWorldPacket(uint32_t opcode, const uint8_t* payload, uint32_t payloadSize);
    bool sendForeverAuthChallenge();
    bool processForeverAuthPacket();
    bool processForeverAuthSession(uint32_t opcode, const std::vector<uint8_t>& payload);
    bool processForeverAuthContinuedSession(uint32_t opcode, const std::vector<uint8_t>& payload);
    bool sendForeverEnterEncryptedMode();
    bool processForeverEnterEncryptedModeAck(uint32_t opcode, const std::vector<uint8_t>& payload);
    bool sendForeverPostAuthBootstrap();
    bool processForeverEncryptedPacket();
    bool encryptForeverPayload(std::vector<uint8_t>& data, std::array<uint8_t, 12>& tag);
    bool decryptForeverPayload(std::vector<uint8_t>& data, const std::array<uint8_t, 12>& tag);

    // Modern opcode dispatcher / glue coordinator.
    bool dispatchForeverOpcode(uint32_t rawOpcode, const uint8_t* payload, size_t payloadSize);
    bool processForeverGlueState(WorldPacket& packet, bool& consumed);

    // Movement handlers.
    bool handleMovementOpcodes(WorldPacket& packet);
    bool handleForeverUnknown003E002DOpcode(WorldPacket& packet);

    // Character handlers.
    bool handleForeverCharEnumOpcode(WorldPacket& packet);
    bool handleForeverCheckCharacterNameOpcode(WorldPacket& packet);
    bool handleForeverCharCreateOpcode(WorldPacket& packet);
    bool handleForeverPlayerLoginOpcode(WorldPacket& packet);
    bool handleForeverCharDeleteOpcode(WorldPacket& packet);
    bool handleForeverUndeleteCooldownOpcode(WorldPacket& packet);
    bool handleForeverCharacterListAckOpcode(WorldPacket& packet);
    bool handleForeverUpdateAccountDataOpcode(WorldPacket& packet);

    // DB2/system/glue handlers.
    bool handleForeverDbQueryBulkOpcode(WorldPacket& packet);
    bool handleForeverHotfixRequestOpcode(WorldPacket& packet);
    bool handleForeverQueryCreatureOpcode(WorldPacket& packet);
    bool handleForeverQueryGameObjectOpcode(WorldPacket& packet);
    bool handleCloseInteraction(WorldPacket& packet);
    bool handleForeverSetSelectionOpcode(WorldPacket& packet);
    bool handleForeverPingOpcode(WorldPacket& packet);
    bool handleForeverLogoutRequestOpcode(WorldPacket& packet);
    bool handleForeverLogoutCancelOpcode(WorldPacket& packet);
    bool handleForeverSocialContractOpcode(WorldPacket& packet);
    bool handleForeverSocialContractAcceptOpcode(WorldPacket& packet);
    bool handleForeverServerTimeOffsetOpcode(WorldPacket& packet);
    bool handleForeverIgnoredGlueOpcode(WorldPacket& packet);
    bool handleForeverQuickJoinOpcode(WorldPacket& packet);
    bool handleForeverLastCatalogFetchOpcode(WorldPacket& packet);

    // Character/glue services.
    bool sendForeverEmptyCharacterList();
    bool sendForeverCharacterEnumFromDatabase(bool includeCollection);
    bool handleForeverCreateCharacter(const uint8_t* payload, uint32_t payloadSize);
    bool sendForeverSecondEnumCompletion();
    bool sendForeverHotfixBootstrap();

private:
    friend class ::OpcodeHandlerRegistry;

    enum class ForeverWorldState : uint8_t
    {
        Disabled,
        AwaitClientInitializer,
        AwaitAuthSession,
        AuthSessionObserved,
        AwaitEncryptionAck,
        Encrypted
    };

    ForeverWorldState m_foreverWorldState{ForeverWorldState::Disabled};

    std::array<uint8_t, 32> m_foreverServerChallenge{};
    std::array<uint8_t, 32> m_foreverDosChallenge{};
    std::array<uint8_t, 40> m_foreverSessionKey{};
    std::array<uint8_t, 32> m_foreverEncryptionKey{};

    uint32_t m_foreverClientBuild{0};
    uint32_t m_foreverBattleNetAccountId{0};
    uint32_t m_foreverGameAccountId{0};
    std::string m_foreverGameAccountName{};
    uint32_t m_foreverPacketOpcode{0};
    uint32_t m_foreverPacketRemaining{0};
    uint64_t m_foreverRecvCounter{0};
    uint64_t m_foreverSendCounter{0};
    uint64_t m_foreverCryptoSendCounter{0};
    uint64_t m_foreverCryptoRecvCounter{0};
    uint32_t m_foreverCharacterEnumRequests{0};
    uint32_t m_foreverDbQueryBulkCount{0};
    uint32_t m_foreverRegionId{0};
    uint32_t m_foreverBattlegroupId{0};
    uint32_t m_foreverRealmId{0};
    uint32_t m_foreverSecondEnumStep{0};
    bool m_foreverSecondEnumPending{false};
    bool m_foreverSecondEnumGateSent{false};
    bool m_foreverSecondEnumSocialContractSeen{false};
    bool m_foreverBufferedEnumRequest{false};
    bool m_foreverPostDbEnumRefreshPending{false};
    bool m_foreverPostDbEnumRefreshSent{false};
    bool m_foreverPostCreateEnumRefreshPending{false};
    bool m_foreverPostCreateEnumRefreshArmed{false};
    bool m_foreverHotfixBootstrapSent{false};
    bool m_foreverContinuedSession{false};
    uint64_t m_foreverConnectToKey{0};
    uint32_t m_foreverPendingLoginGuid{0};
    WorldSession* m_foreverContinuedWorldSession{nullptr};
    bool m_foreverEncryptedHeaderReady{false};
    uint32_t m_foreverEncryptedPacketSize{0};
    std::array<uint8_t, 12> m_foreverEncryptedPacketTag{};
    std::array<uint8_t, 4> m_foreverEncryptedOpcode{};
