/*
 * messages: generalized information reporting
 * Copyright(c) 2001-2008 by wave++ "Yuri D'Elia" <wavexx@users.sf.net>
 * Distributed under GNU GPL (v2 or above) WITHOUT ANY WARRANTY.
 */

#ifndef MESSAGES_H
#define MESSAGES_H

/* local headers */
#include "config.h"

/* system headers */
#include <stdio.h>
#include <stdarg.h>

/*
 * usually I hate using globals, but in that case it would
 * bloat all over the code without some namespace
 *
 * set this to the program name (argv[0]) when program starts
 */
extern char* msg_prgName;

/*
 * msg_messages is a bitfield of messages to show
 */
#define MSG_FINFO	0x01
#define MSG_FERR	0x02
#define MSG_FDRINFO	0x04
#define MSG_FALL	0x0F
#define MSG_FDBG	0x10

extern char msg_messages;

/* general info, written as argv[0]: ...\n on stdout */
void
msg_info(const char* fmt, ...);

/* general errors, written as argv[0]: ...\n on stderr */
void
msg_err(const char* fmt, ...);

/* general driver message, written as argv[0] [driver]: ...\n on stderr */
void
msg_drInfo(const char* driver, const char* fmt, ...);

/* general debug trace */
static inline void
msg_dbg(const char* file, const int line, const char* fmt, ...)
{
#ifndef NDEBUG
  if(msg_messages & MSG_FDBG)
  {
    va_list params;

    fprintf(stderr, "%s[%s:%d]: ", msg_prgName, file, line);

    va_start(params, fmt);
    vfprintf(stderr, fmt, params);
    va_end(params);

    fprintf(stderr, "\n");
  }
#endif
}

/*
 * msg_dbg requires the filename and the current line. We can use a macro
 * to shorten the command lenght a bit (msg_dbg(__POSITION__, ...)
 */
#undef __POSITION__
#define __POSITION__ __FILE__, __LINE__

#endif
