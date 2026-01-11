# Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>

add_library(openssl INTERFACE)

target_link_libraries(openssl
  INTERFACE
    OpenSSL::Crypto
    OpenSSL::SSL
)
