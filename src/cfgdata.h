/*
 * cfgdata: WMND configuration data
 * Copyright(c) 2001-2008 by wave++ "Yuri D'Elia" <wavexx@users.sf.net>
 * Distributed under GNU GPL (v2 or above) WITHOUT ANY WARRANTY.
 */

#ifndef CFGDATA_H
#define CFGDATA_H

#include "config.h"


/* configuration enums */
struct pair_strint
{
  char* strval;
  int val;
};

/* configuration booleans values */
extern struct pair_strint psi_bool[];

/* default configuration values */
struct pair_strstr
{
  char* token;
  char* value;
  int used;
};

/* configuration strings */
extern struct pair_strstr pss_defcon[];

#endif
