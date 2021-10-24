# Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>

set(OPENSSL_EXPECTED_VERSION 1.1.0)

find_package(OpenSSL REQUIRED COMPONENTS Crypto SSL)

add_library(openssl INTERFACE)

target_link_libraries(openssl
  INTERFACE
    OpenSSL::Crypto
    OpenSSL::SSL)
