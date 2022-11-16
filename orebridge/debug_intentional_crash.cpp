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

#include "debug_intentional_crash.h"
#include <unistd.h>
#include <cstdio>

static volatile bool true_var = true;

void debug_generate_sigsegv_segv_maperr()
{
#ifdef OREDEBUG
    *(int*) 0 = 0;
#endif
}

void debug_generate_busyloop()
{
#ifdef OREDEBUG
    while (true_var) {
        sleep(1);
    }
#endif
}

void debug_proxy_call_complex_two()
{
#ifdef OREDEBUG
    printf("debug_proxy_call_complex_two");
    debug_generate_sigsegv_segv_maperr();
#endif
}

static void debug_proxy_call_oneline_static()
{
#ifdef OREDEBUG
    debug_proxy_call_complex_two();
#endif
}

void debug_proxy_call_oneline()
{
#ifdef OREDEBUG
    debug_proxy_call_oneline_static();
#endif
}

void debug_generate_long_backtrace()
{
#ifdef OREDEBUG
    printf("debug_generate_long_backtrace");
    debug_proxy_call_oneline();
#endif
}