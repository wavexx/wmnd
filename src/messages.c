/*
 * messages: generalized information reporting - implementation
 * Copyright(c) 2001-2008 by wave++ "Yuri D'Elia" <wavexx@users.sf.net>
 * Distributed under GNU GPL (v2 or above) WITHOUT ANY WARRANTY.
 */

/* local headers */
#include <messages.h>

/* externs definitions */
char* msg_prgName = NULL;
char msg_messages = MSG_FALL;

void
msg_info(const char* fmt, ...)
{
  if(msg_messages & MSG_FINFO)
  {
    va_list params;

    printf("%s: ", msg_prgName);

    va_start(params, fmt);
    vprintf(fmt, params);
    va_end(params);

    printf("\n");
  }
}

void
msg_err(const char* fmt, ...)
{
  if(msg_messages & MSG_FERR)
  {
    va_list params;

    fprintf(stderr, "%s: ", msg_prgName);

    va_start(params, fmt);
    vfprintf(stderr, fmt, params);
    va_end(params);

    fprintf(stderr, "\n");
  }
}

void
msg_drInfo(const char* driver, const char* fmt, ...)
{
  if(msg_messages & MSG_FDRINFO)
  {
    va_list params;

    fprintf(stderr, "%s [%s]: ", msg_prgName, driver);

    va_start(params, fmt);
    vfprintf(stderr, fmt, params);
    va_end(params);

    fprintf(stderr, "\n");
  }
}
