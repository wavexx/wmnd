/*
 * bits: platform indipendent version operating on chars
 * Copyright(c) 2001 by wave++ "Yuri D'Elia" <wavexx@users.sf.net>
 * Distributed under GNU GPL (v2 or above) WITHOUT ANY WARRANTY.
 */

#ifndef BITS_H
#define BITS_H

#include "config.h"


/* a typedef for bits operation container */
typedef long int _bits_type;


/* core functions */
static inline _bits_type
getmask(const int pos)
{
  _bits_type rtval = 1;
  return (rtval << pos);
}

static inline void
onbit(_bits_type* obj, const int pos)
{
  *obj |= getmask(pos);
}

static inline void
offbit(_bits_type* obj, const int pos)
{
  *obj &= ~getmask(pos);
}

static inline int
getbit(const _bits_type* obj, const int pos)
{
  _bits_type tmpobj = *obj;
  tmpobj &= getmask(pos);
  return (tmpobj? 1: 0);
}

static inline void
invbit(_bits_type* obj, const int pos)
{
  *obj ^= getmask(pos);
}

#endif
