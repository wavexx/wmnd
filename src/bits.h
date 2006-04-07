/* 
 * bits operators - platform indipendent version operating on chars
 * small header file, by Wave++ <wavexx@users.sourceforge.net>
 *
 * Originally an hpp extended file. Distributed under GNU GPL
 */

#ifndef BITS_H
#define BITS_H

#include <config.h>

/* a typedef for bits operation container */
typedef long int __bytearray;

/* core functions */
INLINE __bytearray getmask(const int pos);
INLINE void onbit(__bytearray* obj, const int pos);
INLINE void offbit(__bytearray* obj, const int pos);
INLINE int getbit(const __bytearray* obj, const int pos);
INLINE void invbit(__bytearray* obj, const int pos);

#endif

