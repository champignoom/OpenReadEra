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

#ifndef _DEBUG_INTENTIONAL_CRASH_
#define _DEBUG_INTENTIONAL_CRASH_

#ifdef OREDEBUG
//#define DEBUG_INTENTIONAL_CRASH
#endif

void debug_generate_sigsegv_segv_maperr();
void debug_generate_busyloop();
void debug_generate_long_backtrace();
void debug_proxy_call_complex_two();
void debug_proxy_call_oneline();

#endif //_DEBUG_INTENTIONAL_CRASH_
