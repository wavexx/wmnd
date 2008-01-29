/*
 * wmnd: main program module, shared header
 * Copyright(c) 2001-2008 by wave++ "Yuri D'Elia" <wavexx@users.sf.net>
 * Distributed under GNU GPL (v2 or above) WITHOUT ANY WARRANTY.
 */

#ifndef WMND_H
#define WMND_H

#include "bits.h" /* portable bit functions */

/* picky time headers, for both time_t and timeval */
#ifdef TIME_WITH_SYS_TIME
#include <time.h>
#include <sys/time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

/* version number is now defined into config.h */
#include "config.h"
#define WMND_VERSION PACKAGE_VERSION

#define MAXBUF 256

#ifndef MIN
#define MIN(a, b) (((a) < (b))? (a): (b))
#endif
#ifndef MAX
#define MAX(a, b) (((a) < (b))? (b): (a))
#endif


struct Devices
{
  struct Devices* next;     /* ptr to next structure */
  char* name;               /* device name from /proc/net/dev */
  void* drvdata;            /* driver data used by the driver */
  int devnum;		    /* overall device number */
  int drvnum;               /* driver number */
  int online;               /* 0 for online, 1 for offline */
  time_t devstart;          /* device activity start, 0 for unavaible */

  unsigned long avg[4];     /* average sampling - last samples */
  unsigned long avgBuf[4];  /* average sampling - buffers */

  unsigned long his[59][4];
  unsigned long ib_stat_last;
  unsigned long ob_stat_last;
  unsigned long ip_stat_last;
  unsigned long op_stat_last;
  unsigned long ib_max_his; /* maximum input bytes in history */
  unsigned long ob_max_his; /* maximum output bytes in history */
  unsigned long ip_max_his; /* maximum input packets in history */
  unsigned long op_max_his; /* maximum output packets in history */
};

struct var
{
  struct var* v_next; /* ptr to next structure */
  char* v_name;       /* option name */
  char* v_value;      /* option value */
};

typedef struct {
  unsigned int nr_devices;              /* number of devices in list */
  _bits_type flags;                     /* big bit-mapped flags mask */
  unsigned int wavemode;                /* type of wave graph */
  unsigned int nWavemodes;              /* numbers of wave graph */
  struct Devices* curdev;               /* current device */
  void                                  /* scale function */
  (*scale)(unsigned char sign, unsigned long value, char* buf);
  unsigned int refresh;	                /* speed of the refresh */
  unsigned int avgSteps;                /* number of steps to average */
  unsigned int avgRSteps;               /* number of remaining steps */
  float smooth;                         /* smoothing factor */
  unsigned long scroll;	                /* speed of the graph scrolling */
  unsigned long maxScale;               /* fixed max scale */
} DevTable;

typedef struct
{
  unsigned long txColor;
  unsigned long rxColor;
  unsigned long mdColor;
} stdCols;

typedef struct
{
  Display* d;
  int xfd;
  int screen;
  Window root;
  Window win;
  Window iconwin;
  Pixmap pixmap;
  Pixmap mask;
  GC gc;
  int width;
  int height;
  int x;
  int y;
  int button;
  int update;
  stdCols stdColors; /* standard allocated colors */
} Dockapp;

/* I don't like this very much.. */
extern Dockapp dockapp;
extern DevTable wmnd;

typedef struct
{
  int enable;
  int x;
  int y;
  int width;
  int height;
} MRegion;

/* store all configuration in one place */
#define RUN_ONLINE      3  /* current device is online */
#define LED_POWER       4  /* next 3 are for different leds */
#define LED_RX          5
#define LED_TX          6
#define CFG_SHOWTIME    15
#define CFG_MAXSCREEN   16 /* 1: yes 0: no, max history */
#define CFG_SHORTNAME   17 /* 1: use 0: no */
#define CFG_MODE        18 /* 1: bytes 0: packets */
#define CFG_SHOWMAX     19 /* 1: yes 0: no */

/* region definitions */
#define REG_NOREG	(-1u)
#define REG_DEV		0
#define REG_RT_PB	1
#define REG_MAIN	2
#define REG_SCALE_RX	3
#define REG_SCALE_TX	4
#define REG_SCRIPT	5

/* bit ops on wmnd.flags */
#define bit_set(n) onbit(&wmnd.flags, n)
#define bit_off(n) offbit(&wmnd.flags, n)
#define bit_tgl(n) invbit(&wmnd.flags, n)
#define bit_get(n) getbit(&wmnd.flags, n)

#endif
