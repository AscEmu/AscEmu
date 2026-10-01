/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

struct ssl_ctx_st;
using SSL_CTX = ssl_ctx_st;

namespace AscEmu::Battlenet
{
    class BNetTlsContext
    {
    public:
        static BNetTlsContext& getInstance();

        BNetTlsContext(BNetTlsContext&&) = delete;
        BNetTlsContext(const BNetTlsContext&) = delete;
        BNetTlsContext& operator=(BNetTlsContext&&) = delete;
        BNetTlsContext& operator=(const BNetTlsContext&) = delete;

        bool initialize();
        void finalize();

        [[nodiscard]] SSL_CTX* getContext() const { return m_context; }

    private:
        BNetTlsContext() = default;
        ~BNetTlsContext() = default;

        SSL_CTX* m_context = nullptr;
    };
}
