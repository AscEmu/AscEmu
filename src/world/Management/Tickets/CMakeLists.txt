# Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Management/Tickets)

set(SRC_MANAGEMENT_TICKETS_FILES
    ${PATH_PREFIX}/TicketMgr.cpp
    ${PATH_PREFIX}/TicketMgr.hpp
)

source_group(Management\\Tickets FILES ${SRC_MANAGEMENT_TICKETS_FILES})
unset(PATH_PREFIX)
