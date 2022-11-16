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
 * Developers: ReadEra Team (2013-2020), Playful Curiosity (2013-2020).
 */

#include <android/log.h>
#include "openreadera.h"
#include "openreadera_version.h"

bool OreBuildDebug() {
#if !defined(NDEBUG) || defined(DEBUG) || defined(_DEBUG) || defined(OREDEBUG)
    return true;
#else
    return false;
#endif
}

#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)

std::string OreVersion(std::string base_version)
{
#ifdef ORE_BUILD_TYPE
    base_version += "+";
    base_version += STRINGIZE_VALUE_OF(ORE_BUILD_TYPE);
#endif
    if (OreBuildDebug()) {
        base_version += "+DEBUG";
    }
    return base_version;
}

void OreStart(const char* name) {
    std::string defines;
#ifdef NDEBUG
    defines += " NDEBUG";
#endif
#ifdef DEBUG
    defines += " DEBUG";
#endif
#ifdef _DEBUG
    defines += " _DEBUG";
#endif
#ifdef OREDEBUG
    defines += " OREDEBUG";
#endif
    if (!defines.empty()) {
        defines = ". Defines:" + defines;
    }
    __android_log_print(ANDROID_LOG_INFO, ORE_LOG_TAG,
                        "Start %s v%s%s",
                        name,
                        OreVersion(OPENREADERA_BASE_VERSION).c_str(),
                        defines.c_str());
}

void OreVerResporse(const char* base_version, CmdResponse& response)
{
    response.cmd = CMD_RES_VERSION;
    response.addIpcString(OreVersion(base_version).c_str(), true);
}

bool OreIsSmartDirectArchive(uint32_t direct_archive)
{
    return direct_archive == DIRECT_ARCHIVE_SMART;
}

bool OreIsNormalDirectArchive(uint32_t direct_archive)
{
    return direct_archive > 0 && direct_archive != DIRECT_ARCHIVE_SMART;
}