//C-  -*- C++ -*-
//C- -------------------------------------------------------------------
//C- DjVuLibre-3.5
//C- Copyright (c) 2002  Leon Bottou and Yann Le Cun.
//C- Copyright (c) 2001  AT&T
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, either Version 2 of the license,
//C- or (at your option) any later version. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C-
//C- This program is distributed in the hope that it will be useful,
//C- but WITHOUT ANY WARRANTY; without even the implied warranty of
//C- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//C- GNU General Public License for more details.
//C- 
//C- DjVuLibre-3.5 is derived from the DjVu(r) Reference Library from
//C- Lizardtech Software.  Lizardtech Software has authorized us to
//C- replace the original DjVu(r) Reference Library notice by the following
//C- text (see doc/lizard2002.djvu and doc/lizardtech2007.djvu):
//C-
//C-  ------------------------------------------------------------------
//C- | DjVu (r) Reference Library (v. 3.5)
//C- | Copyright (c) 1999-2001 LizardTech, Inc. All Rights Reserved.
//C- | The DjVu Reference Library is protected by U.S. Pat. No.
//C- | 6,058,214 and patents pending.
//C- |
//C- | This software is subject to, and may be distributed under, the
//C- | GNU General Public License, either Version 2 of the license,
//C- | or (at your option) any later version. The license should have
//C- | accompanied the software or you may obtain a copy of the license
//C- | from the Free Software Foundation at http://www.fsf.org .
//C- |
//C- | The computer code originally released by LizardTech under this
//C- | license and unmodified by other parties is deemed "the LIZARDTECH
//C- | ORIGINAL CODE."  Subject to any third party intellectual property
//C- | claims, LizardTech grants recipient a worldwide, royalty-free, 
//C- | non-exclusive license to make, use, sell, or otherwise dispose of 
//C- | the LIZARDTECH ORIGINAL CODE or of programs derived from the 
//C- | LIZARDTECH ORIGINAL CODE in compliance with the terms of the GNU 
//C- | General Public License.   This grant only confers the right to 
//C- | infringe patent claims underlying the LIZARDTECH ORIGINAL CODE to 
//C- | the extent such infringement is reasonably necessary to enable 
//C- | recipient to make, have made, practice, sell, or otherwise dispose 
//C- | of the LIZARDTECH ORIGINAL CODE (or portions thereof) and not to 
//C- | any greater extent that may be necessary to utilize further 
//C- | modifications or combinations.
//C- |
//C- | The LIZARDTECH ORIGINAL CODE is provided "AS IS" WITHOUT WARRANTY
//C- | OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//C- | TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
//C- | MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
//C- +------------------------------------------------------------------

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

// This file defines machine independent classes
// for running and synchronizing threads.
// - Author: Leon Bottou, 01/1998

// From: Leon Bottou, 1/31/2002
// Almost unchanged by Lizardtech.
// GSafeFlags should go because it not as safe as it claims.

#include "GThreads.h"
#include "GException.h"
#include "DjVuMessageLite.h"
#include "ore_log.h"

#include <cstddef>
#include <cstdlib>
#include <cstdio>

// ----------------------------------------
// Consistency check

#ifdef USE_EXCEPTION_EMULATION
# if defined(POSIXTHREADS)
#  warning "Compiler must support thread safe exceptions"
# endif
#endif

#ifndef _DEBUG
# if defined(DEBUG)
#  define _DEBUG /* */
# elif DEBUGLVL >= 1
#  define _DEBUG /* */
# endif
#endif

#ifdef HAVE_NAMESPACES
namespace DJVU {
# ifdef NOT_DEFINED // Just to fool emacs c++ mode
}
#endif
#endif

// ----------------------------------------
// POSIXTHREADS IMPLEMENTATION
// ----------------------------------------

#if POSIXTHREADS

#if defined(CMA_INCLUDE)
# define DCETHREADS 1
# define pthread_key_create pthread_keycreate
#else
# define pthread_mutexattr_default  NULL
# define pthread_condattr_default   NULL
#endif

static pthread_t pthread_null; // portable zero initialization!

void *
GThread::start(void *arg)
{
  GThread *gt = (GThread*)arg;
#if DCETHREADS
# ifdef CANCEL_ON
  pthread_setcancel(CANCEL_ON);
  pthread_setasynccancel(CANCEL_ON);
# endif
#else // !DCETHREADS
# ifdef PTHREAD_CANCEL_ENABLE
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
# endif
# ifdef PTHREAD_CANCEL_ASYNCHRONOUS
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);
# endif
#endif
  // Catch exceptions
#ifdef __EXCEPTIONS
  try
    {
#endif
      G_TRY
        {
          (gt->xentry)(gt->xarg);
        }
      G_CATCH(ex)
        {
          ex.perror();
          DjVuMessageLite::perror( ERR_MSG("GThreads.uncaught") );
          abort();
        }
      G_ENDCATCH;
#ifdef __EXCEPTIONS
    }
  catch(...)
    {
      DjVuMessageLite::perror( ERR_MSG("GThreads.unrecognized") );
      abort();
    }
#endif
  return 0;
}

// GThread

GThread::GThread(int stacksize) :
  hthr(pthread_null), xentry(0), xarg(0)
{
}

GThread::~GThread()
{
  hthr = pthread_null;
}

int
GThread::create(void (*entry)(void*), void *arg)
{
  if (xentry || xarg)
    return -1;
  xentry = entry;
  xarg = arg;
#if DCETHREADS
  int ret = pthread_create(&hthr, pthread_attr_default, GThread::start, (void*)this);
  if (ret >= 0)
    pthread_detach(hthr);
#else
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  int ret = pthread_create(&hthr, &attr, start, (void*)this);
  pthread_attr_destroy(&attr);
#endif
  return ret;
}

void
GThread::terminate()
{
#ifndef __ANDROID__
  if (xentry || xarg)
    pthread_cancel(hthr);
#endif
}

int
GThread::yield()
{
#if DCETHREADS
  pthread_yield();
#else
  // should use sched_yield() when available.
  static struct timeval timeout = { 0, 0 };
  ::select(0, 0,0,0, &timeout);
#endif
  return 0;
}

void*
GThread::current()
{
  pthread_t self = pthread_self();
#if defined(pthread_getunique_np)
  return (void*) pthread_getunique_np( & self );
#elif defined(cma_thread_get_unique)
  return (void*) cma_thread_get_unique( & self );
#else
  return (void*) self;
#endif
}

// -- GMonitor

GMonitor::GMonitor() : tag(nullptr), ok(0), count(1), locker(pthread_null) {
    // none of this should be necessary ... in theory.
#ifdef PTHREAD_MUTEX_INITIALIZER
    static pthread_mutex_t tmutex = PTHREAD_MUTEX_INITIALIZER;
    memcpy(&mutex, &tmutex, sizeof(mutex));
#endif
#ifdef PTHREAD_COND_INITIALIZER
    static pthread_cond_t tcond = PTHREAD_COND_INITIALIZER;
    memcpy(&cond, &tcond, sizeof(cond));
#endif
    // standard
    pthread_mutex_init(&mutex, pthread_mutexattr_default);
    pthread_cond_init(&cond, pthread_condattr_default);
    locker = pthread_self();
    ok = 1;
}

GMonitor::GMonitor(const char *tag) : GMonitor::GMonitor() {
#ifdef OREDEBUG
    //this->tag = strdup(tag);
#endif
}

GMonitor::~GMonitor() {
#ifdef OREDEBUG
    if (tag) {
        free(tag);
    }
#endif
    ok = 0;
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}


void GMonitor::enter() {
    pthread_t self = pthread_self();
    if (count > 0 || !pthread_equal(locker, self)) {
        if (ok) {
#ifdef OREDEBUG
            if (tag) {
                LV("GMonitor:%s:enter %d GO", tag, count);
            }
#endif
            pthread_mutex_lock(&mutex);
        }
        locker = self;
        count = 1;
    }
    count -= 1;
}

void GMonitor::leave() {
    pthread_t self = pthread_self();
    if (ok && (count > 0 || !pthread_equal(locker, self)))
        G_THROW(ERR_MSG("GThreads.not_acq_broad"));
#ifdef OREDEBUG
    if (tag) {
        LV("GMonitor:%s:leave %d GO", tag, count);
    }
#endif
    count += 1;
    if (count > 0) {
        count = 1;
        locker = pthread_null;
        if (ok)
            pthread_mutex_unlock(&mutex);
    }
}

void GMonitor::signal() {
    if (ok) {
#ifdef OREDEBUG
        if (tag) {
            LV("GMonitor:%s:signal %d GO", tag, count);
        }
#endif
        pthread_t self = pthread_self();
        if (count > 0 || !pthread_equal(locker, self))
            G_THROW(ERR_MSG("GThreads.not_acq_signal"));
        pthread_cond_signal(&cond);
    }
}

void GMonitor::broadcast() {
    if (ok) {
#ifdef OREDEBUG
        if (tag) {
            LV("GMonitor:%s:broadcast %d GO", tag, count);
        }
#endif
        pthread_t self = pthread_self();
        if (count > 0 || !pthread_equal(locker, self))
            G_THROW(ERR_MSG("GThreads.not_acq_broad"));
        pthread_cond_broadcast(&cond);
    }
}

void GMonitor::wait() {
    // Check
    pthread_t self = pthread_self();
    if (count > 0 || !pthread_equal(locker, self)) {
        G_THROW(ERR_MSG("GThreads.not_acq_wait"));
    }
    // Wait
    if (ok) {
        // Release
        int sav_count = count;
        count = 1;
        // Wait
#ifdef OREDEBUG
        if (tag) {
            LV("GMonitor:%s:wait pthread_cond_wait, %d GO", tag, sav_count);
        }
#endif
        pthread_cond_wait(&cond, &mutex);
#ifdef OREDEBUG
        if (tag) {
            LV("GMonitor:%s:wait pthread_cond_wait, %d OK", tag, sav_count);
        }
#endif
        // Re-acquire
        count = sav_count;
        locker = self;
    }
}

void GMonitor::wait(unsigned long timeout) {
    // Check
    pthread_t self = pthread_self();
    if (count > 0 || !pthread_equal(locker, self)) {
        G_THROW(ERR_MSG("GThreads.not_acq_wait"));
    }
    // Wait
    if (ok) {
        // Release
        int sav_count = count;
        count = 1;
        // Wait
        struct timeval abstv;
        struct timespec absts;
        gettimeofday(&abstv, NULL); // grrr
        absts.tv_sec = abstv.tv_sec + timeout / 1000;
        absts.tv_nsec = abstv.tv_usec * 1000 + (timeout % 1000) * 1000000;
        if (absts.tv_nsec > 1000000000) {
            absts.tv_nsec -= 1000000000;
            absts.tv_sec += 1;
        }
#ifdef OREDEBUG
        if (tag) {
            LV("GMonitor:%s:wait pthread_cond_timedwait, %d GO", tag, sav_count);
        }
#endif
        pthread_cond_timedwait(&cond, &mutex, &absts);
#ifdef OREDEBUG
        if (tag) {
            LV("GMonitor:%s:wait pthread_cond_timedwait, %d OK", tag, sav_count);
        }
#endif
        // Re-acquire
        count = sav_count;
        locker = self;
    }
}

#endif

// ----------------------------------------
// GSAFEFLAGS 
// ----------------------------------------

GSafeFlags &
GSafeFlags::operator=(long xflags)
{
   enter();
   if (flags!=xflags)
   {
      flags=xflags;
      broadcast();
   }
   leave();
   return *this;
}

GSafeFlags::operator long(void) const
{
   long f;
   ((GSafeFlags *) this)->enter();
   f=flags;
   ((GSafeFlags *) this)->leave();
   return f;
}

bool
GSafeFlags::test_and_modify(long set_mask, long clr_mask,
			    long set_mask1, long clr_mask1)
{
   enter();
   if ((flags & set_mask)==set_mask &&
       (~flags & clr_mask)==clr_mask)
   {
      long new_flags=flags;
      new_flags|=set_mask1;
      new_flags&=~clr_mask1;
      if (new_flags!=flags)
      {
	 flags=new_flags;
	 broadcast();
      }
      leave();
      return true;
   }
   leave();
   return false;
}

#ifdef HAVE_NAMESPACES
}
# ifndef NOT_USING_DJVU_NAMESPACE
using namespace DJVU;
# endif
#endif
