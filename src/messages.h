/* messages - generalized information reporting */

#include <stdarg.h>

/*
 * usually I hate using globals, but in that case it would
 * bloat all over the code without some namespace
 *
 * set this to the program name (argv[0]) when program starts
 */
extern char* msg_prgName;

/*
 * msg_messages is a bitfielf of messages to show
 */
#define MSG_FINFO 0x01
#define MSG_FERR 0x02
#define MSG_FDRINFO 0x04
#define MSG_FALL 0xFF

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

/* general debug trace, links to msg_err */
#ifndef NDEBUG
#define msg_dbg(fmt, args...) msg_err("%s:%d: " fmt, __FILE__, __LINE__, ##args)
#else
#define msg_dbg(...)
#endif

