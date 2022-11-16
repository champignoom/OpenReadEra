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

#ifndef _ORE_LOG_H_
#define _ORE_LOG_H_

#include <android/log.h>

#define ORE_LOG_TAG "openreadera"

#ifdef OREDEBUG
#define LV(...)  __android_log_print(ANDROID_LOG_VERBOSE, ORE_LOG_TAG, __VA_ARGS__)
#define LD(...)  __android_log_print(ANDROID_LOG_DEBUG, ORE_LOG_TAG, __VA_ARGS__)
#define LDD(ENABLED, args...) { if (ENABLED) {__android_log_print(ANDROID_LOG_DEBUG, ORE_LOG_TAG, args); } }
#define LI(...)  __android_log_print(ANDROID_LOG_INFO, ORE_LOG_TAG, __VA_ARGS__)
#define LW(...)  __android_log_print(ANDROID_LOG_WARN, ORE_LOG_TAG, __VA_ARGS__)
#define LE(...)  __android_log_print(ANDROID_LOG_ERROR, ORE_LOG_TAG, __VA_ARGS__)
#else
#define LV(...)
#define LD(...)
#define LDD(ENABLED, args...)
#define LI(...)
#define LW(...)
#define LE(...)
#endif

#endif // _ORE_LOG_H_
