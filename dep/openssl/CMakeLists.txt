# Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>

find_package(OpenSSL 1.0 REQUIRED COMPONENTS Crypto SSL)

add_library(openssl INTERFACE)

target_link_libraries(openssl
    INTERFACE
    OpenSSL::Crypto
    OpenSSL::SSL)
