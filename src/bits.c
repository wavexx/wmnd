/* 
 * bits operators - platform indipendent version operating on chars
 * small source file, by Wave++ <wavexx@users.sourceforge.net>
 */

#include "bits.h"

INLINE __bytearray
getmask(const int pos)
{
  __bytearray rtval = 1;

  if (!pos)
    return 1;

  return (rtval << pos);
}

INLINE void
onbit(__bytearray* obj, const int pos)
{
  *obj |= getmask(pos);
}

INLINE void
offbit(__bytearray* obj, const int pos)
{
  *obj &= ~getmask(pos);
}

INLINE int
getbit(const __bytearray* obj, const int pos)
{
  __bytearray tmpobj = *obj;
  tmpobj &= getmask(pos);
  return tmpobj ? 1 : 0;
}

INLINE void
invbit(__bytearray* obj, const int pos)
{
  *obj ^= getmask(pos);
}

