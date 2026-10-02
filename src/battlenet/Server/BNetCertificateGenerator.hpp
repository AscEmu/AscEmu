/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <string>

namespace AscEmu::Battlenet
{
    // Creates the TLS identity of this server: a private root certificate and the server certificate signed by it.
    // The certificate file gets the chain (server certificate first, root last) and the key file the private key
    // of the server certificate, both as PEM. Clients trust the server after they were patched with the certificate file.
    bool generateServerCertificate(const std::string& certificateFile, const std::string& privateKeyFile);
}
