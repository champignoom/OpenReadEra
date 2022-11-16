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
#include <cerrno>
#include <cstring>

#include "ore_log.h"
#include "StProtocol.h"
#include "StQueue.h"

constexpr static bool LOG = false;

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

#define BUF_SIZE_1 1024
#define BUF_SIZE_2 (2*16384)
#define BUF_SIZE_3 (1*65536)

Queue::Queue(const char* fname, int mode, const char* lctx)
{
    fp = open(fname, mode);
    pthread_mutex_init(&readlock, nullptr);
    pthread_mutex_init(&writelock, nullptr);
}

Queue::~Queue()
{
    close(fp);

    pthread_mutex_destroy(&readlock);
    pthread_mutex_destroy(&writelock);
}

int Queue::readBuffer(int size, uint8_t* buf)
{
    int count = 0;

    int bufSize = size < BUF_SIZE_1 ? BUF_SIZE_1 : size < BUF_SIZE_2 ? BUF_SIZE_2 : BUF_SIZE_3;

    while (count < size)
    {
        ssize_t r = read(fp, buf + count, MIN(bufSize, size - count));
        if (r == 0)
        {
            LDD(LOG, "Queue: EOF");
            return 0;
        }
        else if (r == -1)
        {
            LDD(LOG, "Queue: IO error: %s", strerror(errno));
            return 0;
        }
        count += r;
    }
    return count;
}

int Queue::readByte(uint8_t* buf)
{
    ssize_t res = read(fp, buf, sizeof(uint8_t));
    if (res == 0)
    {
        LDD(LOG, "Queue: EOF");
        return 0;
    }
    else if (res == -1)
    {
        LDD(LOG, "Queue: IO error: %s", strerror(errno));
        return 0;
    }
    return res;
}

int Queue::readInt(uint32_t* buf)
{
    ssize_t res = read(fp, buf, sizeof(uint32_t));
    if (res == 0)
    {
        LDD(LOG, "Queue: EOF");
        return 0;
    }
    else if (res == -1)
    {
        LDD(LOG, "Queue: IO error: %s", strerror(errno));
        return 0;
    }
    return res;
}

int Queue::readData(CmdData* data, uint8_t& hasNext)
{
    LDD(LOG, "Queue: Reading data type...");
    uint8_t type = 0;
    int res = readByte(&(type));
    if (res == 0)
    {
        LDD(LOG, "Queue: No data type received");
        return 0;
    }

    hasNext = type & TYPE_MASK_HAS_NEXT;
    uint8_t dtype = type & TYPE_MASK_TYPE;

    LDD(LOG, "Queue: Data type: %d, has next data: %d", dtype, hasNext);

    if ((data->type == TYPE_ARRAY_POINTER) && data->type != dtype)
    {
        LE("Received type %d not equal to expected %d", dtype, data->type);
        data->freeArray();
    }
    data->type = dtype;

    LDD(LOG, "Queue: Reading data...");
    uint32_t old = data->value.value32;
    res = readInt(&(data->value.value32));
    if (res == 0)
    {
        LDD(LOG, "Queue: No data received");
        return 0;
    }
    LDD(LOG, "Queue: Data: %08x", data->value.value32);

    if ((data->type == TYPE_ARRAY_POINTER) && data->value.value32 > 0)
    {
        if (old < data->value.value32 && data->external_array != NULL)
        {
            LE("Received length %d not equal to expected %d", data->value.value32, old);
            data->freeArray();
        }

        LDD(LOG, "Queue: Reading external data...");
        if (data->external_array == NULL)
        {
            data->external_array = (uint8_t*) malloc(data->value.value32);
        }
        res = readBuffer(data->value.value32, data->external_array);
        if (res == 0)
        {
            LDD(LOG, "Queue: No external data received");
            data->freeArray();
            return 0;
        }
        else
        {
            LDD(LOG, "Queue: Data received");
        }
    }
    return res;
}

void Queue::writeData(CmdData* data)
{
    uint8_t type = data->type | (data->nextData != NULL ? TYPE_MASK_HAS_NEXT : 0);
    uint32_t val = data->value.value32;

    LDD(LOG, "Queue: Writing data type: %02x", type);
    write(fp, &(type), sizeof(type));

    LDD(LOG, "Queue: Writing data: %08x", val);
    write(fp, &(val), sizeof(val));
    if (data->type == TYPE_ARRAY_POINTER)
    {
        if (val > 0 && data->external_array != NULL)
        {
            LDD(LOG, "Queue: Writing external data: %d", val);

            uint32_t bufSize = val < BUF_SIZE_1 ? BUF_SIZE_1 : val < BUF_SIZE_2 ? BUF_SIZE_2 : BUF_SIZE_3;
            uint32_t count = 0;

            while(count < val) {
                ssize_t r = write(fp, data->external_array + count, MIN(bufSize, val - count));
                if (r == -1)
                {
                    LDD(LOG, "Queue: IO error: %s", strerror(errno));
                    return;
                }
                count += r;
            }
        }
        else
        {
            LDD(LOG, "Queue: No external data");
        }
    }
}
