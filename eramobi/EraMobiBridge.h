/*
 * Copyright (C) 2013-2020 READERA LLC
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Developers: ReadEra Team (2013-2020), Playful Curiosity (2013-2020),
 * Tarasus (2018-2020).
 */

#ifndef _ERAMOBI_BRIDGE_H_
#define _ERAMOBI_BRIDGE_H_

#include "openreadera.h"
#include "StBridge.h"
#include "ore_log.h"

class EraMobiBridge : public StBridge {
public:
    EraMobiBridge();

    ~EraMobiBridge();

    void process(CmdRequest& request, CmdResponse& response);

protected:

    void processMeta(CmdRequest &request, CmdResponse &response);

    void processConvert(CmdRequest &request, CmdResponse &response);

    void processQuit(CmdRequest& request, CmdResponse& response);
};

#endif //_ERAMOBI_BRIDGE_H_
