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

#include "EraMobiBridge.h"
// Should go as last include to not trigger rebuild of other files on changes
#include "openreadera_version.h"

void EraMobiBridge::processQuit(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_QUIT;
}

EraMobiBridge::EraMobiBridge() : StBridge("EraMobiBridge") { }

EraMobiBridge::~EraMobiBridge() { }

void EraMobiBridge::process(CmdRequest& request, CmdResponse& response)
{
    response.reset();
    request.print(lctx);
    switch (request.cmd)
    {
        case CMD_REQ_CONVERT:
            processConvert(request, response);
            break;
        case CMD_REQ_CRE_METADATA:
            processMeta(request, response);
            break;
        case CMD_REQ_QUIT:
            processQuit(request, response);
            break;
        case CMD_REQ_VERSION:
            OreVerResporse(OPENREADERA_BASE_VERSION, response);
            break;
        default:
            LE("Unknown request: %d", request.cmd);
            response.result = RES_UNKNOWN_CMD;
            break;
    }
    response.print(lctx);
}
