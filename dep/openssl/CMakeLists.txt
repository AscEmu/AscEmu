set(OPENSSL_EXPECTED_VERSION 1.0.0)

find_package(OpenSSL REQUIRED COMPONENTS Crypto SSL)

add_library(openssl INTERFACE)

target_link_libraries(openssl
  INTERFACE
    OpenSSL::Crypto
    OpenSSL::SSL)