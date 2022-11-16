/*
 * Copyright (C) 2013 The Common CLI viewer interface Project
 * Copyright (C) 2013-2020 READERA LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>

#include "ore_log.h"
#include "StProtocol.h"
#include "StQueue.h"
#include "StBridge.h"

constexpr static bool LOG = false;

void StBridge::renice() {
    const char *env = getenv("ST_NICE_LEVEL");
    if (env != nullptr) {
        int level = 0;
        if (strcmp(env, "Lower") == 0) {
            level = -10;
        } else if (strcmp(env, "Lowest") == 0) {
            level = -20;
        }
        if (level != 0) {
            errno = 0;
            nice(level);
            if (!errno) {
                LI("StBridge: Process nice level has been successfully changed to %s", env);
            } else {
                LE("StBridge: Process nice level cannot be changed: %d", errno);
            }
            return;
        }
    }
    //LD("StBridge: Process nice level should not be changed");
}

int StBridge::main(int argc, char *argv[]) {
    if (argc < 3) {
        LE("StBridge: No command line arguments");
        return 1;
    }
    renice();
    ResponseQueue out(argv[2], O_WRONLY, lctx);
    //LD("StBridge: Sending ready notification...");
    out.sendReadyNotification();
    LD("StBridge fifos: I[%s], O[%s]", argv[1], argv[2]);
    RequestQueue in(argv[1], O_RDONLY, lctx);
    CmdRequest request;
    CmdResponse response;
    bool run = true;
    while (run) {
        LDD(LOG, "StBridge: Waiting for request...");
        int res = in.readRequest(request);
        if (res == 0) {
            LE("StBridge: No data received");
            return -1;
        }
        LDD(LOG, "StBridge: Processing request...");
        process(request, response);
        LDD(LOG, "StBridge: Sending response...");
        out.writeResponse(response);
        run = response.cmd != CMD_RES_QUIT;
        request.reset();
        response.reset();
    }
    LI("StBridge: Exit");
    return 0;
}
