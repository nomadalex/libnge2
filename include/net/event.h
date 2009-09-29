/*
 * Copyright (c) 2000-2004 Niels Provos <provos@citi.umich.edu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _EVENT_H_
#define _EVENT_H_
#ifdef HAVE_NET_CONFIG_H
#include "config.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <sys/timeb.h>
#include <time.h>
	
#define EVLIST_TIMEOUT	0x01
#define EVLIST_INSERTED	0x02
#define EVLIST_SIGNAL	0x04
#define EVLIST_ACTIVE	0x08
#define EVLIST_INTERNAL	0x10
#define EVLIST_INIT	0x80

/* EVLIST_X_ Private space: 0x1000-0xf000 */
#define EVLIST_ALL	(0xf000 | 0x9f)

#define EV_TIMEOUT	0x01
#define EV_READ		0x02
#define EV_WRITE	0x04
#define EV_SIGNAL	0x08
#define EV_PERSIST	0x10	/* Persistant event */

/* Fix so that ppl dont have to run with <sys/queue.h> */
#ifndef TAILQ_ENTRY
#define _EVENT_DEFINED_TQENTRY
#define TAILQ_ENTRY(type)						\
struct {								\
	struct type *tqe_next;	/* next element */			\
	struct type **tqe_prev;	/* address of previous next element */	\
}
#endif /* !TAILQ_ENTRY */
#ifndef RB_ENTRY
#define _EVENT_DEFINED_RBENTRY
#define RB_ENTRY(type)							\
struct {								\
	struct type *rbe_left;		/* left element */		\
	struct type *rbe_right;		/* right element */		\
	struct type *rbe_parent;	/* parent element */		\
	int rbe_color;			/* node color */		\
}
#endif /* !RB_ENTRY */
#define MAX_TIMER 65536
static int _timesocket[MAX_TIMER]={-1};
static int _timerid = 0;;
static int i = 0;


struct event_base;

#ifdef WIN32
#ifndef _LIB
struct timeval {
	long	tv_sec;		/* seconds */
	long	tv_usec;	/* and microseconds */
};
#endif
#endif

struct event {
	TAILQ_ENTRY (event) ev_next;
	TAILQ_ENTRY (event) ev_active_next;
	TAILQ_ENTRY (event) ev_signal_next;
	RB_ENTRY (event) ev_timeout_node;

	struct event_base *ev_base;
	int ev_fd;
	short ev_events;
	short ev_ncalls;
	short *ev_pncalls;	/* Allows deletes in callback */
	
	struct timeval ev_timeout;

	int ev_pri;		/* smaller numbers are higher priority */

	void (*ev_callback)(int, short, void *arg);
	void *ev_arg;

	int ev_res;		/* result passed to event callback */
	int ev_flags;
};

#define EVENT_SIGNAL(ev)	(int)(ev)->ev_fd
#define EVENT_FD(ev)		(int)(ev)->ev_fd



#ifdef _EVENT_DEFINED_TQENTRY
#undef TAILQ_ENTRY
struct event_list;
struct evkeyvalq;
#undef _EVENT_DEFINED_TQENTRY
#else
TAILQ_HEAD (event_list, event);
TAILQ_HEAD (evkeyvalq, evkeyval);
#endif /* _EVENT_DEFINED_TQENTRY */
#ifdef _EVENT_DEFINED_RBENTRY
#undef RB_ENTRY
#undef _EVENT_DEFINED_RBENTRY
#endif /* _EVENT_DEFINED_RBENTRY */

struct eventop {
	char *name;
	void *(*init)(struct event_base *);
	int (*add)(void *, struct event *);
	int (*del)(void *, struct event *);
	int (*recalc)(struct event_base *, void *, int);
	int (*dispatch)(struct event_base *, void *, struct timeval *);
	void (*dealloc)(struct event_base *, void *);
};

void *event_init(void);
int event_dispatch(void);
int event_base_dispatch(struct event_base *);
void event_base_free(struct event_base *);

#define _EVENT_LOG_DEBUG 0
#define _EVENT_LOG_MSG   1
#define _EVENT_LOG_WARN  2
#define _EVENT_LOG_ERR   3
typedef void (*event_log_cb)(int severity, const char *msg);
void event_set_log_callback(event_log_cb cb);

/* Associate a different event base with an event */
int event_base_set(struct event_base *, struct event *);

#define EVLOOP_ONCE	0x01
#define EVLOOP_NONBLOCK	0x02
int event_loop(int);
int event_base_loop(struct event_base *, int);
int event_loopexit(struct timeval *);	/* Causes the loop to exit */
int event_base_loopexit(struct event_base *, struct timeval *);

#define evtimer_add(ev, tv)		event_add(ev, tv)
#define evtimer_set(ev, cb, arg) 	    do{ \
											for(i=0;i<MAX_TIMER;i++) \
											if(_timesocket[i]!=-1) {_timerid = i;break;}\
											_timesocket[_timerid]= socket(PF_INET,SOCK_STREAM,0); \
											event_set(ev,_timesocket[_timerid], EV_READ, cb, arg);}while(0) 								
#define evtimer_del(ev)			do{event_del(ev);\
										closesocket(_timesocket[_timerid]);\
										_timesocket[_timerid]= -1;}while(0)	
#define evtimer_pending(ev, tv)		event_pending(ev, EV_TIMEOUT, tv)
#define evtimer_initialized(ev)		((ev)->ev_flags & EVLIST_INIT)

#define timeout_add(ev, tv)		event_add(ev, tv)
#define timeout_set(ev, cb, arg)	do{ \
											for(i=0;i<MAX_TIMER;i++) \
											if(_timesocket[i]!=-1) {_timerid = i;break;}\
											_timesocket[_timerid]= socket(PF_INET,SOCK_STREAM,0); \
											event_set(ev,_timesocket[_timerid], EV_READ, cb, arg);}while(0) 
#define timeout_del(ev) 			do{event_del(ev);\
										closesocket(_timesocket[_timerid]);\
										_timesocket[_timerid]= -1;}while(0)		
								
#define timeout_pending(ev, tv)		event_pending(ev, EV_TIMEOUT, tv)
#define timeout_initialized(ev)		((ev)->ev_flags & EVLIST_INIT)



void event_set(struct event *, int, short, void (*)(int, short, void *), void *);
int event_once(int, short, void (*)(int, short, void *), void *, struct timeval *);
int event_base_once(struct event_base *, int, short, void (*)(int, short, void *), void *, struct timeval *);

int event_add(struct event *, struct timeval *);
int event_del(struct event *);
void event_active(struct event *, int, short);

int event_pending(struct event *, short, struct timeval *);

#ifdef WIN32
#define event_initialized(ev)		((ev)->ev_flags & EVLIST_INIT && (ev)->ev_fd != (int)INVALID_HANDLE_VALUE)
#else
#define event_initialized(ev)		((ev)->ev_flags & EVLIST_INIT)
#endif

/* Some simple debugging functions */
const char *event_get_version(void);
const char *event_get_method(void);

/* These functions deal with event priorities */

int	event_priority_init(int);
int	event_base_priority_init(struct event_base *, int);
int	event_priority_set(struct event *, int);

#ifdef __cplusplus
}
#endif

#endif /* _EVENT_H_ */
