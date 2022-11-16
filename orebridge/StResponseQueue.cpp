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
#include "debug_intentional_crash.h"

constexpr static bool LOG = false;

ResponseQueue::ResponseQueue(const char* fname, int mode, const char* lctx)
    : Queue(fname, mode, lctx)
{
}

void ResponseQueue::sendReadyNotification()
{
    CmdResponse response(CMD_NOTIF_READY);
    writeResponse(response);
}

void ResponseQueue::writeResponse(CmdResponse& response)
{
    LDD(LOG, "ResponseQueue: Waiting for write lock");
    pthread_mutex_lock(&writelock);
#ifdef DEBUG_INTENTIONAL_CRASH
    if (response.cmd == CMD_RES_OPEN) {
        debug_generate_long_backtrace();
    }
#endif
    uint8_t cmd = response.cmd | (response.first != NULL ? CMD_MASK_HAS_DATA : 0);

    LDD(LOG, "ResponseQueue: Writing response cmd: %02x", cmd);
    write(fp, &(cmd), sizeof(cmd));

    LDD(LOG, "ResponseQueue: Writing response result: %d", response.result);
    write(fp, &(response.result), sizeof(response.result));

    CmdData* data = response.first;
    while (data != nullptr)
    {
        writeData(data);
        data = data->nextData;
    }

    LDD(LOG, "ResponseQueue: Flush response");
    fdatasync(fp);

    pthread_mutex_unlock(&writelock);
}

int ResponseQueue::readResponse(CmdResponse& response)
{
    LDD(LOG, "ResponseQueue: Waiting for read lock");
    pthread_mutex_lock(&readlock);

    LDD(LOG, "ResponseQueue: Reading response cmd...");
    uint8_t cmd = 0;
    int res = readByte(&cmd);
    if (res == 0)
    {
        pthread_mutex_unlock(&readlock);
#ifdef DEBUG_INTENTIONAL_CRASH
        debug_generate_long_backtrace();
#endif
        return 0;
    }
    response.cmd = cmd & CMD_MASK_CMD;
    uint8_t hasData = cmd & CMD_MASK_HAS_DATA;
    LDD(LOG, "ResponseQueue: Response cmd: %d, has data: %d", response.cmd, hasData);

    LDD(LOG, "ResponseQueue: Reading response result...");
    res = readByte(&(response.result));
    if (res == 0)
    {
        pthread_mutex_unlock(&readlock);
        return 0;
    }
    LDD(LOG, "ResponseQueue: Response result: %d", response.result);

    CmdData* data = response.first;
    while (hasData)
    {
        if (data == nullptr)
        {
            data = new CmdData();
            response.addData(data);
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

bool ResponseQueue::readResponseValid(CmdResponse& response, int cmd)
{
    return readResponse(response) > 0 && response.cmd == cmd && response.result == RES_OK;
}

void ResponseQueue::mutexLock()
{
    pthread_mutex_lock(&readlock);
}

void ResponseQueue::mutexUnlock()
{
    pthread_mutex_unlock(&readlock);
}

int ResponseQueue::readResponseHeader(CmdResponse& response, uint8_t& has_next)
{
    LDD(LOG, "ResponseQueue: Reading response cmd");
    uint8_t cmd = 0;
    int res = readByte(&cmd);
    if (res == 0) {
        return 0;
    }
    response.cmd = static_cast<uint8_t>(cmd & CMD_MASK_CMD);
    has_next = static_cast<uint8_t>(cmd & CMD_MASK_HAS_DATA);
    LDD(LOG, "ResponseQueue: Reading response result");
    res = readByte(&(response.result));
    if (res == 0) {
        has_next = 0;
    }
    LDD(LOG, "ResponseQueue: Response result: %d", response.result);
    return res;
}

bool ResponseQueue::readResponseHeader(int expected_cmd, uint8_t& has_next)
{
    CmdResponse response;
    if (readResponseHeader(response, has_next) > 0) {
        if (response.cmd == expected_cmd && response.result == RES_OK) {
            return true;
        }
    }
    return false;
}

void ResponseQueue::resetData(CmdData* data)
{
    data->type = TYPE_NONE;
    data->value.value32 = 0;
    data->owned_external = true;
    data->external_array = nullptr;
    data->nextData = nullptr;
}

void ResponseQueue::discardResponse(uint8_t has_next)
{
    if (!has_next) {
        return;
    }
    auto* data = new CmdData();
    while (has_next) {
        if (readData(data, has_next) == 0) {
            break;
        }
        resetData(data);
    }
    delete data;
}

bool ResponseQueue::readDataWrap(CmdData* data, uint8_t type, uint8_t& has_next, bool require_next)
{
    resetData(data);
    if (readData(data, has_next) == 0) {
        return false;
    }
    if (data->type != type) {
        discardResponse(has_next);
        return false;
    }
    if (data->type == TYPE_ARRAY_POINTER) {
        if (data->value.value32 == 0 || data->external_array == nullptr) {
            discardResponse(has_next);
            return false;
        }
    }
    if (require_next && !has_next) {
        discardResponse(has_next);
        return false;
    }
    return true;
}