/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// client 3.3.3, 2010/03/20
enum LoginErrorCode
{
    E_RESPONSE_SUCCESS                                       = 0x00,
    E_RESPONSE_FAILURE                                       = 0x01,
    E_RESPONSE_CANCELLED                                     = 0x02,
    E_RESPONSE_DISCONNECTED                                  = 0x03,
    E_RESPONSE_FAILED_TO_CONNECT                             = 0x04,
    E_RESPONSE_CONNECTED                                     = 0x05,
    E_RESPONSE_VERSION_MISMATCH                              = 0x06,

    E_CSTATUS_CONNECTING                                     = 0x07,
    E_CSTATUS_NEGOTIATING_SECURITY                           = 0x08,
    E_CSTATUS_NEGOTIATION_COMPLETE                           = 0x09,
    E_CSTATUS_NEGOTIATION_FAILED                             = 0x0A,
    E_CSTATUS_AUTHENTICATING                                 = 0x0B,

    E_AUTH_OK                                                = 0x0C,
    E_AUTH_FAILED                                            = 0x0D,
    E_AUTH_REJECT                                            = 0x0E,
    E_AUTH_BAD_SERVER_PROOF                                  = 0x0F,
    E_AUTH_UNAVAILABLE                                       = 0x10,
    E_AUTH_SYSTEM_ERROR                                      = 0x11,
    E_AUTH_BILLING_ERROR                                     = 0x12,
    E_AUTH_BILLING_EXPIRED                                   = 0x13,
    E_AUTH_VERSION_MISMATCH                                  = 0x14,
    E_AUTH_UNKNOWN_ACCOUNT                                   = 0x15,
    E_AUTH_INCORRECT_PASSWORD                                = 0x16,
    E_AUTH_SESSION_EXPIRED                                   = 0x17,
    E_AUTH_SERVER_SHUTTING_DOWN                              = 0x18,
    E_AUTH_ALREADY_LOGGING_IN                                = 0x19,
    E_AUTH_LOGIN_SERVER_NOT_FOUND                            = 0x1A,
    E_AUTH_WAIT_QUEUE                                        = 0x1B,
    E_AUTH_BANNED                                            = 0x1C,
    E_AUTH_ALREADY_ONLINE                                    = 0x1D,
    E_AUTH_NO_TIME                                           = 0x1E,
    E_AUTH_DB_BUSY                                           = 0x1F,
    E_AUTH_SUSPENDED                                         = 0x20,
    E_AUTH_PARENTAL_CONTROL                                  = 0x21,
    E_AUTH_LOCKED_ENFORCED                                   = 0x22,

    E_REALM_LIST_IN_PROGRESS                                 = 0x23,
    E_REALM_LIST_SUCCESS                                     = 0x24,
    E_REALM_LIST_FAILED                                      = 0x25,
    E_REALM_LIST_INVALID                                     = 0x26,
    E_REALM_LIST_REALM_NOT_FOUND                             = 0x27,

    E_ACCOUNT_CREATE_IN_PROGRESS                             = 0x28,
    E_ACCOUNT_CREATE_SUCCESS                                 = 0x29,
    E_ACCOUNT_CREATE_FAILED                                  = 0x2A,

    E_CHAR_LIST_RETRIEVING                                   = 0x2B,
    E_CHAR_LIST_RETRIEVED                                    = 0x2C,
    E_CHAR_LIST_FAILED                                       = 0x2D,
};

