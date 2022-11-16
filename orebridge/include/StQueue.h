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

#ifndef __STQUEUE_H__
#define __STQUEUE_H__

#include <pthread.h>

class Queue
{
protected:
    int fp;
    pthread_mutex_t readlock;
    pthread_mutex_t writelock;

protected:
    Queue(const char* fname, int mode, const char* lctx);
    ~Queue();
protected:
    int readBuffer(int size, uint8_t* buf);
    int readByte(uint8_t* buf);
    int readInt(uint32_t* buf);
    int readData(CmdData* data, uint8_t& hasNext);
    void writeData(CmdData* data);
};

class RequestQueue : Queue
{
public:
    RequestQueue(const char* fname, int mode, const char* lctx);
public:
    int readRequest(CmdRequest& request);
    void writeRequest(CmdRequest& request);
};

class ResponseQueue : Queue
{
public:
    ResponseQueue(const char* fname, int mode, const char* lctx);
public:
    void sendReadyNotification();
    int readResponse(CmdResponse& response);
    bool readResponseValid(CmdResponse& response, int cmd);
    void writeResponse(CmdResponse& response);
    int readResponseHeader(CmdResponse& response, uint8_t& has_next);
    bool readResponseHeader(int expected_cmd, uint8_t& has_next);
    void mutexLock();
    void mutexUnlock();
    void resetData(CmdData* data);
    void discardResponse(uint8_t next_data);
    bool readDataWrap(CmdData* data, uint8_t type, uint8_t& has_next, bool require_next);
};

#endif
