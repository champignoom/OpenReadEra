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

#include "StProtocol.h"
#include "ore_log.h"

constexpr static bool LOG = false;

CmdData::CmdData()
{
    type = TYPE_NONE;
    value.value32 = 0;
    owned_external = true;
    external_array = nullptr;
    nextData = nullptr;
}

CmdData::~CmdData()
{
    freeArray();
    type = TYPE_NONE;
    value.value32 = 0;
    external_array = nullptr;
    if (nextData != nullptr)
    {
        delete nextData;
        nextData = nullptr;
    }
}

uint8_t* CmdData::newByteArray(int n)
{
    freeArray();
    type = TYPE_ARRAY_POINTER;
    value.value32 = n;
    owned_external = true;
    external_array = (uint8_t*) malloc(value.value32);
    return external_array;
}

void CmdData::freeArray()
{
    if (owned_external && external_array != nullptr)
    {
        free(external_array);
    }
    type = TYPE_NONE;
    value.value32 = 0;
    external_array = nullptr;
    owned_external = true;
}

CmdData* CmdData::setInt(uint32_t val)
{
    freeArray();
    type = TYPE_FIX_INT;
    value.value32 = val;
    return this;
}

CmdData* CmdData::setWords(uint16_t val0, uint16_t val1)
{
    freeArray();
    type = TYPE_FIX_WORDS;
    value.value16[0] = val0;
    value.value16[1] = val1;
    return this;
}

CmdData* CmdData::setFloat(float val)
{
    freeArray();
    type = TYPE_FIX_FLOAT;
    value.valuef = val;
    return this;
}

CmdData* CmdData::setByteArray(int n, uint8_t* ptr, bool owned)
{
    freeArray();
    type = TYPE_ARRAY_POINTER;
    value.value32 = n;
    owned_external = owned;
    if (owned)
    {
        external_array = (uint8_t*) calloc(1, value.value32);
        memcpy(external_array, ptr, value.value32);
    }
    else
    {
        external_array = ptr;
    }
    return this;
}

CmdData* CmdData::setIntArray(int n, int* ptr, bool owned)
{
    freeArray();
    type = TYPE_ARRAY_POINTER;
    value.value32 = n * sizeof(int);
    owned_external = owned;
    if (owned)
    {
        external_array = (uint8_t*) calloc(1, value.value32);
        memcpy(external_array, ptr, value.value32);
    }
    else
    {
        external_array = (uint8_t*) ptr;
    }
    return this;
}

CmdData* CmdData::setFloatArray(int n, float* ptr, bool owned)
{
    freeArray();
    type = TYPE_ARRAY_POINTER;
    value.value32 = n * sizeof(float);
    owned_external = owned;
    if (owned)
    {
        external_array = (uint8_t*) calloc(1, value.value32);
        memcpy(external_array, ptr, value.value32);
    }
    else
    {
        external_array = (uint8_t*) ptr;
    }
    return this;
}

CmdData* CmdData::setIpcString(const char* data, bool owned)
{
    freeArray();
    type = TYPE_ARRAY_POINTER;
    owned_external = owned;
    if (data != nullptr)
    {
        value.value32 = strlen(data) + 1;
        external_array = owned ? (uint8_t*) strdup(data) : (uint8_t*) data;
    }
    else
    {
        value.value32 = 0;
        external_array = nullptr;
    }
    return this;
}

void CmdData::print(const char* lctx)
{
    LDD(LOG, "CmdData: Data: %s, %p %u %08x %p",
            lctx, this, this->type, this->value.value32, this->external_array);
}

CmdDataList::CmdDataList()
{
    dataCount = 0;
    first = last = nullptr;
}

CmdDataList& CmdDataList::addData(CmdData* data)
{
    if (data != nullptr)
    {
        if (last == nullptr)
        {
            first = last = data;
        }
        else
        {
            last->nextData = data;
            last = data;
        }
        dataCount++;
    }
    return *this;
}

CmdDataList& CmdDataList::addInt(uint32_t val)
{
    return addData((new CmdData())->setInt(val));
}

CmdDataList& CmdDataList::addWords(uint16_t val0, uint16_t val1)
{
    return addData((new CmdData())->setWords(val0, val1));
}

CmdDataList& CmdDataList::addFloat(float val)
{
    return addData((new CmdData())->setFloat(val));
}

CmdDataList& CmdDataList::addByteArray(int n, uint8_t* ptr, bool owned)
{
    return addData((new CmdData())->setByteArray(n, ptr, owned));
}

CmdDataList& CmdDataList::addIntArray(int n, int* ptr, bool owned)
{
    return addData((new CmdData())->setIntArray(n, ptr, owned));
}
CmdDataList& CmdDataList::addFloatArray(int n, float* ptr, bool owned)
{
    return addData((new CmdData())->setFloatArray(n, ptr, owned));
}

CmdDataList& CmdDataList::addIpcString(const char* data, bool owned)
{
    return addData((new CmdData())->setIpcString(data, owned));
}

CmdRequest::CmdRequest()
{
    cmd = CMD_UNKNOWN;
}
CmdRequest::CmdRequest(uint8_t c)
{
    cmd = c;
}
CmdRequest::~CmdRequest()
{
    reset();
}

void CmdRequest::reset()
{
    if (first != nullptr)
    {
        delete first;
    }
    cmd = CMD_UNKNOWN;
    dataCount = 0;
    first = last = nullptr;
}

void CmdRequest::print(const char* lctx)
{
    LDD(LOG, "CmdData: Request: %s %u", lctx, this->cmd);
    CmdData* data;
    for (data = this->first; data != nullptr; data = data->nextData)
    {
        data->print(lctx);
    }
}

CmdResponse::CmdResponse()
{
    cmd = CMD_UNKNOWN;
    result = RES_OK;
}

CmdResponse::CmdResponse(uint8_t c)
{
    cmd = c;
    result = RES_OK;
}

CmdResponse::~CmdResponse()
{
    reset();
}

void CmdResponse::reset()
{
    if (first != nullptr)
    {
        delete first;
    }
    cmd = CMD_UNKNOWN;
    result = RES_OK;
    first = last = nullptr;
}

void CmdResponse::print(const char* lctx)
{
    LDD(LOG, "CmdData: Response: %s %u %u", lctx, this->cmd, this->result);
    CmdData* data;
    for (data = this->first; data != nullptr; data = data->nextData)
    {
        data->print(lctx);
    }
}

CmdDataIterator::CmdDataIterator(CmdData* d)
{
    this->count = 0;
    this->data = d;
    this->errors = 0;
}
CmdDataIterator::~CmdDataIterator()
{
    this->count = 0;
    this->data = nullptr;
    this->errors = 0;
}

bool CmdDataIterator::hasNext()
{
    return this->data != nullptr;
}

bool CmdDataIterator::isValid()
{
    return this->errors == 0;
}

bool CmdDataIterator::isValid(int index)
{
    return (this->errors & (1 << index)) != 0;
}

int CmdDataIterator::getCount()
{
    return count;
}

uint32_t CmdDataIterator::getErrors()
{
    return errors;
}

CmdDataIterator& CmdDataIterator::getWords(uint16_t* v0, uint16_t* v1)
{
    *v0 = *v1 = 0;
    if (this->data == nullptr)
    {
        this->errors |= 1 << count;
    }
    else if (this->data->type != TYPE_FIX_WORDS)
    {
        this->errors |= 1 << count;
    }
    else
    {
        *v0 = this->data->value.value16[0];
        *v1 = this->data->value.value16[1];
    }

    count++;
    this->data = this->data != nullptr ? this->data->nextData : nullptr;
    return *this;

}

CmdDataIterator& CmdDataIterator::getInt(uint32_t* v0)
{
    *v0 = 0;
    if (this->data == nullptr)
    {
        this->errors |= 1 << count;
    }
    else if (this->data->type != TYPE_FIX_INT)
    {
        this->errors |= 1 << count;
    }
    else
    {
        *v0 = this->data->value.value32;
    }

    count++;
    this->data = this->data != nullptr ? this->data->nextData : nullptr;
    return *this;
}

CmdDataIterator& CmdDataIterator::getFloat(float* v0)
{
    *v0 = 0;
    if (this->data == nullptr)
    {
        this->errors |= 1 << count;
    }
    else if (this->data->type != TYPE_FIX_FLOAT)
    {
        this->errors |= 1 << count;
    }
    else
    {
        *v0 = this->data->value.valuef;
    }

    count++;
    this->data = this->data != nullptr ? this->data->nextData : nullptr;
    return *this;
}

CmdDataIterator& CmdDataIterator::optionalByteArray(uint8_t** buffer, uint32_t* len)
{
    *buffer = nullptr;
    if (len) {
    *len = 0;
    }
    if (this->data == nullptr)
    {
        this->errors |= 1 << count;
    }
    else if (this->data->type != TYPE_ARRAY_POINTER)
    {
        this->errors |= 1 << count;
    }
    else
    {
    	if (len) {
        *len = this->data->value.value32;
		}
        *buffer = this->data->external_array;
    }

    count++;
    this->data = this->data != nullptr ? this->data->nextData : nullptr;
    return *this;
}

CmdDataIterator& CmdDataIterator::getByteArray(uint8_t** buffer)
{
    *buffer = nullptr;
    if (this->data == nullptr)
    {
        this->errors |= 1 << count;
    }
    else if (this->data->type != TYPE_ARRAY_POINTER)
    {
        this->errors |= 1 << count;
    }
    else if (this->data->value.value32 == 0 || this->data->external_array == nullptr)
    {
        this->errors |= 1 << count;
    }
    else
    {
        *buffer = this->data->external_array;
    }

    count++;
    this->data = this->data != nullptr ? this->data->nextData : nullptr;
    return *this;
}

CmdDataIterator& CmdDataIterator::getIntArray(uint32_t** buffer, int elements)
{
    *buffer = nullptr;
    if (this->data == nullptr)
    {
        this->errors |= 1 << count;
    }
    else if (this->data->type != TYPE_ARRAY_POINTER)
    {
        this->errors |= 1 << count;
    }
    else if (this->data->value.value32 != elements * sizeof(uint32_t))
    {
        this->errors |= 1 << count;
    }
    else
    {
        *buffer = (uint32_t*) this->data->external_array;
    }

    count++;
    this->data = this->data != nullptr ? this->data->nextData : nullptr;
    return *this;
}

CmdDataIterator& CmdDataIterator::getFloatArray(float** buffer, int elements)
{
    *buffer = nullptr;
    if (this->data == nullptr)
    {
        this->errors |= 1 << count;
    }
    else if (this->data->type != TYPE_ARRAY_POINTER)
    {
        this->errors |= 1 << count;
    }
    else if (this->data->value.value32 != elements * sizeof(float))
    {
        this->errors |= 1 << count;
    }
    else
    {
        *buffer = (float*) this->data->external_array;
    }

    count++;
    this->data = this->data != nullptr ? this->data->nextData : nullptr;
    return *this;
}

void CmdDataIterator::print(const char* lctx)
{
    LDD(LOG, "CmdData: Iterator: %s %p %u %08x", lctx, this->data, this->count, this->errors);
}

