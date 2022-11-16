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
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "ore_log.h"
#include "StProtocol.h"
#include "StQueue.h"

constexpr static bool LOG = false;

RequestQueue::RequestQueue(const char* fname, int mode, const char* lctx)
    : Queue(fname, mode, lctx)
{
}

void RequestQueue::writeRequest(CmdRequest& request)
{
    LDD(LOG, "RequestQueue: Waiting for write lock");
    pthread_mutex_lock(&writelock);

    uint8_t cmd = request.cmd | (request.first != NULL ? CMD_MASK_HAS_DATA : 0);

    LDD(LOG, "RequestQueue: Writing request cmd: %02x", cmd);
    write(fp, &(cmd), sizeof(cmd));

    CmdData* data = request.first;
    while (data != nullptr)
    {
        writeData(data);
        data = data->nextData;
    }

    LDD(LOG, "RequestQueue: Flush request");
    fdatasync(fp);

    pthread_mutex_unlock(&writelock);
}

int RequestQueue::readRequest(CmdRequest& request)
{
    LDD(LOG, "RequestQueue: Waiting for read lock");
    pthread_mutex_lock(&readlock);

    LDD(LOG, "RequestQueue: Reading request cmd...");
    uint8_t cmd = 0;
    int res = readByte(&cmd);
    if (res == 0)
    {
        LDD(LOG, "RequestQueue: No request received");
        pthread_mutex_unlock(&readlock);
        return 0;
    }

    request.cmd = cmd & CMD_MASK_CMD;
    uint8_t hasData = cmd & CMD_MASK_HAS_DATA;
    LDD(LOG, "RequestQueue: Request cmd: %d, has data: %d", request.cmd, hasData);

    CmdData* data = request.first;
    while (hasData)
    {
        if (data == nullptr)
        {
            data = new CmdData();
            request.addData(data);
        }

        if (readData(data, hasData) == 0)
        {
            pthread_mutex_unlock(&readlock);
            return 0;
        }

        data = data->nextData;
    }

    pthread_mutex_unlock(&readlock);

    return res;
}

