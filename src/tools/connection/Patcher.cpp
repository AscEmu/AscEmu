/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Patcher.hpp"
#include "Helper.hpp"

#include <fstream>
#include <stdexcept>

namespace cp
{
    static std::vector<uint8_t> readAllBytes(const std::filesystem::path& _path)
    {
        std::ifstream in(_path, std::ios::binary);
        if (!in)
            throw std::runtime_error("Failed to open file for reading");

        in.seekg(0, std::ios::end);
        const auto size = static_cast<size_t>(in.tellg());
        in.seekg(0, std::ios::beg);

        std::vector<uint8_t> buf(size);
        if (size > 0 && !in.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(size)))
            throw std::runtime_error("Failed to read file");

        return buf;
    }

    static void writeAllBytes(const std::filesystem::path& _path, std::span<const uint8_t> _bytes)
    {
        std::ofstream out(_path, std::ios::binary | std::ios::trunc);
        if (!out)
            throw std::runtime_error("Failed to open file for writing");

        if (!_bytes.empty() && !out.write(reinterpret_cast<const char*>(_bytes.data()), static_cast<std::streamsize>(_bytes.size())))
            throw std::runtime_error("Failed to write file");
    }

    Patcher::Patcher(std::filesystem::path _file) : m_binary(std::move(_file))
    {
        m_data = readAllBytes(m_binary);
        if (!m_data.empty())
        {
            m_type = getBinaryType(std::span<const uint8_t>(m_data.data(), m_data.size()));
            m_initialized = true;
        }
    }

    Patcher::~Patcher() noexcept
    {
        try
	       {
            if (!m_success)
                return;

            std::error_code ec;
            if (std::filesystem::exists(m_binary, ec))
                std::filesystem::remove(m_binary, ec);

            writeAllBytes(m_binary, m_data);
        }
	       catch (...)
	       {
            // Destructor is not allowed to throw
        }
    }

    std::optional<size_t> Patcher::searchOffset(std::span<const uint8_t> _pattern) const
    {
        if (_pattern.empty() || m_data.size() < _pattern.size())
            return std::nullopt;

        for (size_t i = 0; i <= m_data.size() - _pattern.size(); ++i)
        {
            size_t matches = 0;

            for (size_t j = 0; j < _pattern.size(); ++j)
            {
                const auto pj = _pattern[j];

                // wildcard
                if (pj == 0x00)
                {
                    ++matches;
                    continue;
                }
			
                if (m_data[i + j] != pj)
                    break;
				
                ++matches;
            }

            if (matches == _pattern.size())
                return i;
        }
        return std::nullopt;
    }

    void Patcher::patch(std::span<const uint8_t> _replacement, std::span<const uint8_t> _pattern)
    {
        if (!m_initialized)
            return;
		
        if (_pattern.empty())
            return;
		
        if (m_data.size() < _pattern.size())
            return;

        const auto off = searchOffset(_pattern);
        if (!off)
            return;

        if (*off + _replacement.size() > m_data.size())
            return;

        for (size_t i = 0; i < _replacement.size(); ++i)
            m_data[*off + i] = _replacement[i];
    }
} // namespace cp
