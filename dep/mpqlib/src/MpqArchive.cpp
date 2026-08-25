/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "mpqlib/MpqArchive.hpp"
#include "mpqlib/MpqCrypto.hpp"
#include "mpqlib/MpqCompression.hpp"

#include <algorithm>
#include <cstring>
#include <type_traits>

namespace mpqlib
{
    namespace
    {
        constexpr uint32_t kMpqSignature = 0x1A51504D; // 'MPQ\x1A', little-endian
        constexpr uint32_t kHashEntryDeleted = 0xFFFFFFFE;
        constexpr uint32_t kHashEntryFree = 0xFFFFFFFF;

        template <typename T>
        bool readLE(std::ifstream& file, T& value)
        {
            static_assert(std::is_trivially_copyable_v<T>);
            file.read(reinterpret_cast<char*>(&value), sizeof(T));
            return file.good();
        }

        uint32_t ceilDiv(uint32_t a, uint32_t b)
        {
            return (a + b - 1) / b;
        }
    }

    MpqArchive::MpqArchive(std::string path) :
        m_path(std::move(path))
    {
        m_isOpen = open();
    }

    MpqArchive::~MpqArchive() = default;
    MpqArchive::MpqArchive(MpqArchive&&) noexcept = default;
    MpqArchive& MpqArchive::operator=(MpqArchive&&) noexcept = default;

    bool MpqArchive::open()
    {
        m_file.open(m_path, std::ios::binary);
        if (!m_file.is_open())
            return false;

        uint32_t signature = 0;
        uint32_t headerSize = 0;
        uint32_t archiveSize32 = 0;
        uint16_t formatVersion = 0;
        uint16_t sectorSizeShift = 0;
        uint32_t hashTablePos32 = 0;
        uint32_t blockTablePos32 = 0;

        if (!readLE(m_file, signature) || signature != kMpqSignature)
            return false;
        if (!readLE(m_file, headerSize))
            return false;
        if (!readLE(m_file, archiveSize32))
            return false;
        if (!readLE(m_file, formatVersion))
            return false;
        if (!readLE(m_file, sectorSizeShift))
            return false;
        if (!readLE(m_file, hashTablePos32))
            return false;
        if (!readLE(m_file, blockTablePos32))
            return false;
        if (!readLE(m_file, m_hashTableCount))
            return false;
        if (!readLE(m_file, m_blockTableCount))
            return false;

        uint64_t hiBlockTablePos = 0;
        uint16_t hashTablePosHi = 0;
        uint16_t blockTablePosHi = 0;

        // Header format 2 (Burning Crusade+) extends the base 32-byte header with
        // three more fields needed to address archives >4GB. Format 3/4 (Cata/Mop)
        // archives reserve a much larger header (208 bytes) for HET/BET metadata
        // we don't use, but still carry these same three fields at this position.
        if (headerSize >= 44)
        {
            if (!readLE(m_file, hiBlockTablePos))
                return false;
            if (!readLE(m_file, hashTablePosHi))
                return false;
            if (!readLE(m_file, blockTablePosHi))
                return false;
        }

        // Header format 4 additionally records the real on-disk byte size of
        // each table. Normally that equals entryCount * sizeof(entry), but
        // some archives (notably the Cache/patch-*.MPQ hotfix archives) store
        // the classic hash/block tables *compressed*, in which case these
        // fields are smaller than the natural size - our signal to decompress
        // after decrypting. Layout (from MPQ_HEADER_SIZE_V3 at offset 44):
        // ArchiveSize64(8) BetTablePos64(8) HetTablePos64(8) HashTableSize64(8) BlockTableSize64(8) ...
        uint64_t hashTableCmpSize = static_cast<uint64_t>(m_hashTableCount) * sizeof(HashEntry);
        uint64_t blockTableCmpSize = static_cast<uint64_t>(m_blockTableCount) * sizeof(BlockEntry);

        if (headerSize >= 84)
        {
            uint64_t archiveSize64 = 0, betTablePos64 = 0, hetTablePos64 = 0;
            uint64_t hashTableSize64 = 0, blockTableSize64 = 0;

            if (!readLE(m_file, archiveSize64))
                return false;
            if (!readLE(m_file, betTablePos64))
                return false;
            if (!readLE(m_file, hetTablePos64))
                return false;
            if (!readLE(m_file, hashTableSize64))
                return false;
            if (!readLE(m_file, blockTableSize64))
                return false;

            if (hashTableSize64 != 0 && hashTableSize64 <= hashTableCmpSize)
                hashTableCmpSize = hashTableSize64;
            if (blockTableSize64 != 0 && blockTableSize64 <= blockTableCmpSize)
                blockTableCmpSize = blockTableSize64;

            (void)archiveSize32;
            (void)archiveSize64;
            (void)betTablePos64;
            (void)hetTablePos64; // HET/BET parsing not implemented; see class comment.
        }

        m_sectorSize = 512u << sectorSizeShift;

        const uint64_t hashTablePos = hashTablePos32 | (static_cast<uint64_t>(hashTablePosHi) << 32);
        const uint64_t blockTablePos = blockTablePos32 | (static_cast<uint64_t>(blockTablePosHi) << 32);

        // Read (+ decrypt, + decompress if the on-disk size is smaller than
        // natural) the classic hash table.
        m_hashTable.resize(m_hashTableCount);
        if (!readMaybeCompressedTable(hashTablePos, hashTableCmpSize, m_hashTable.data(),
                m_hashTable.size() * sizeof(HashEntry), "(hash table)"))
            return false;

        // Same for the classic block table.
        m_blockTable.resize(m_blockTableCount);
        if (!readMaybeCompressedTable(blockTablePos, blockTableCmpSize, m_blockTable.data(),
                m_blockTable.size() * sizeof(BlockEntry), "(block table)"))
            return false;

        (void)archiveSize32;
        (void)hiBlockTablePos; // no file in the archives we target exceeds 4GB; kept only for header-layout completeness.
        (void)formatVersion;

        return true;
    }

    bool MpqArchive::readMaybeCompressedTable(uint64_t tablePos, uint64_t cmpSize, void* out, size_t naturalSize, const char* keyName) const
    {
        m_file.clear();
        m_file.seekg(static_cast<std::streamoff>(tablePos));

        const uint32_t key = hashString(keyName, MpqHashType::FileKey);

        if (cmpSize >= naturalSize)
        {
            // Stored uncompressed at its natural size (the overwhelmingly
            // common case).
            m_file.read(reinterpret_cast<char*>(out), static_cast<std::streamsize>(naturalSize));
            if (!m_file.good())
                return false;

            decryptBlock(out, naturalSize, key);
            return true;
        }

        // Compressed: read+decrypt the compressed blob, then run it through
        // the same generic sector-decompression codec used for file data.
        std::vector<uint8_t> compressed(cmpSize);
        m_file.read(reinterpret_cast<char*>(compressed.data()), static_cast<std::streamsize>(cmpSize));
        if (!m_file.good())
            return false;

        decryptBlock(compressed.data(), compressed.size(), key);

        const int32_t written = decompressSector(compressed.data(), static_cast<uint32_t>(compressed.size()),
            static_cast<uint8_t*>(out), static_cast<uint32_t>(naturalSize), SectorCompression::Multi);

        return written >= 0 && static_cast<size_t>(written) == naturalSize;
    }

    std::optional<MpqFileInfo> MpqArchive::findFile(std::string_view fileName) const
    {
        if (!m_isOpen || m_hashTableCount == 0)
            return std::nullopt;

        const uint32_t hash1 = hashString(fileName, MpqHashType::TableOffset) & (m_hashTableCount - 1);
        const uint32_t hash2 = hashString(fileName, MpqHashType::NameA);
        const uint32_t hash3 = hashString(fileName, MpqHashType::NameB);

        uint32_t index = hash1;
        for (uint32_t attempt = 0; attempt < m_hashTableCount; ++attempt, index = (index + 1) & (m_hashTableCount - 1))
        {
            const HashEntry& entry = m_hashTable[index];

            if (entry.blockIndex == kHashEntryFree)
                break;

            if (entry.blockIndex == kHashEntryDeleted)
                continue;

            if (entry.nameHashA == hash2 && entry.nameHashB == hash3)
            {
                if (entry.blockIndex >= m_blockTableCount)
                    return std::nullopt;

                const BlockEntry& block = m_blockTable[entry.blockIndex];
                if ((block.flags & MPQ_FILE_EXISTS) == 0)
                    return std::nullopt;

                MpqFileInfo info;
                info.blockIndex = entry.blockIndex;
                info.filePos = block.filePos;
                info.packedSize = block.packedSize;
                info.unpackedSize = block.unpackedSize;
                info.flags = block.flags;
                return info;
            }
        }

        return std::nullopt;
    }

    bool MpqArchive::readSectorOffsetTable(const MpqFileInfo& info, std::vector<uint32_t>& offsets, uint32_t& fileKey) const
    {
        fileKey = 0;

        const bool isSingleUnit = (info.flags & MPQ_FILE_SINGLE_UNIT) != 0;
        const uint32_t dataSectorCount = isSingleUnit ? 1 : ceilDiv(info.unpackedSize, m_sectorSize);
        uint32_t entryCount = dataSectorCount + 1;
        if (info.flags & MPQ_FILE_SECTOR_CRC)
            entryCount += 1;

        // Single-unit files never carry a stored offset table on disk, even
        // when compressed - there's only one sector, so {0, packedSize} is
        // already fully known from the block table entry itself.
        const bool hasStoredOffsetTable = !isSingleUnit && (info.flags & (MPQ_FILE_COMPRESS | MPQ_FILE_IMPLODE)) != 0;

        if (!hasStoredOffsetTable)
        {
            if (isSingleUnit)
            {
                offsets = { 0, info.packedSize };
                return true;
            }

            // Sector boundaries are implicit: every sector is exactly one
            // archive sector wide, except the last (which holds the remainder).
            offsets.resize(dataSectorCount + 1);
            for (uint32_t i = 0; i < dataSectorCount; ++i)
                offsets[i] = i * m_sectorSize;
            offsets[dataSectorCount] = info.unpackedSize;
            return true;
        }

        const uint32_t tableByteSize = entryCount * sizeof(uint32_t);

        offsets.resize(entryCount);
        m_file.clear();
        m_file.seekg(static_cast<std::streamoff>(info.filePos));
        m_file.read(reinterpret_cast<char*>(offsets.data()), tableByteSize);
        if (!m_file.good())
            return false;

        // Some archives mark files encrypted without setting the flag (a common
        // protection trick); detect it by checking whether the table decoded
        // to its own known byte size.
        const bool looksEncrypted = (info.flags & MPQ_FILE_ENCRYPTED) != 0 ||
            (offsets[0] != tableByteSize && offsets[0] != tableByteSize + sizeof(uint32_t));

        if (looksEncrypted)
        {
            uint32_t detectedKey = 0;
            if (!detectFileKey(offsets.data(), tableByteSize, tableByteSize, m_sectorSize, detectedKey))
                return false;

            fileKey = detectedKey;
            decryptBlock(offsets.data(), tableByteSize, detectedKey - 1);

            if (offsets[0] != tableByteSize)
                return false;
        }

        return true;
    }

    bool MpqArchive::readFile(const MpqFileInfo& info, std::vector<uint8_t>& out) const
    {
        if (!m_isOpen)
            return false;

        out.assign(info.unpackedSize, 0);

        std::vector<uint32_t> offsets;
        uint32_t fileKey = 0;
        if (!readSectorOffsetTable(info, offsets, fileKey))
            return false;

        const bool isSingleUnit = (info.flags & MPQ_FILE_SINGLE_UNIT) != 0;
        const uint32_t dataSectorCount = isSingleUnit ? 1 : ceilDiv(info.unpackedSize, m_sectorSize);
        const bool encrypted = (info.flags & MPQ_FILE_ENCRYPTED) != 0 || fileKey != 0;

        SectorCompression compression = SectorCompression::None;
        if (info.flags & MPQ_FILE_COMPRESS)
            compression = SectorCompression::Multi;
        else if (info.flags & MPQ_FILE_IMPLODE)
            compression = SectorCompression::Imploded;

        std::vector<uint8_t> packedSector;

        for (uint32_t sector = 0; sector < dataSectorCount; ++sector)
        {
            const uint32_t sectorPackedSize = offsets[sector + 1] - offsets[sector];
            const uint32_t sectorUnpackedSize = (sector + 1 == dataSectorCount)
                ? (info.unpackedSize - m_sectorSize * sector)
                : m_sectorSize;

            packedSector.resize(sectorPackedSize);
            m_file.clear();
            m_file.seekg(static_cast<std::streamoff>(info.filePos + offsets[sector]));
            m_file.read(reinterpret_cast<char*>(packedSector.data()), sectorPackedSize);
            if (!m_file.good())
                return false;

            if (encrypted)
                decryptBlock(packedSector.data(), sectorPackedSize, fileKey + sector);

            const bool sectorActuallyPacked = sectorPackedSize < sectorUnpackedSize;
            const SectorCompression effectiveCompression = sectorActuallyPacked ? compression : SectorCompression::None;

            const int32_t written = decompressSector(packedSector.data(), sectorPackedSize,
                out.data() + m_sectorSize * sector, sectorUnpackedSize, effectiveCompression);

            if (written < 0 || static_cast<uint32_t>(written) != sectorUnpackedSize)
                return false;
        }

        return true;
    }

    bool MpqArchive::readFile(std::string_view fileName, std::vector<uint8_t>& out) const
    {
        const auto info = findFile(fileName);
        if (!info)
            return false;

        return readFile(*info, out);
    }

    std::vector<std::string> MpqArchive::listFiles() const
    {
        std::vector<uint8_t> listFileData;
        if (!readFile("(listfile)", listFileData))
            return {};

        std::vector<std::string> names;
        std::string current;
        for (const uint8_t byte : listFileData)
        {
            if (byte == '\r')
                continue;

            if (byte == '\n')
            {
                if (!current.empty())
                    names.push_back(std::move(current));
                current.clear();
                continue;
            }

            current += static_cast<char>(byte);
        }
        if (!current.empty())
            names.push_back(std::move(current));

        return names;
    }
}
