/*
 * display.c - window maker network devices
 *
 * common drawing functions definitions
 */

// local headers
#include "display.h"
#include "wmnd.h"

// system headers
#include <math.h>
#include <X11/Xlib.h>

#ifdef USE_DRW_TRADITIONAL
/* Traditional mode */
void
drwTraditional(unsigned long* hist, unsigned mIn, unsigned mOut,
    unsigned size, int bpp, unsigned long long rx_max,
    unsigned long long tx_max)
{
  unsigned int k;
  for(k = 0; k < 58; k++) {
    unsigned int txlev, rxlev, nolev;
    rxlev = hist[mIn] / bpp;
    txlev = hist[mOut] / bpp;
    nolev = size - (rxlev + txlev);
    copy_xpm_area(66, 0, 1, txlev, k + 3, 53 - (rxlev + txlev));
    copy_xpm_area(65, 0, 1, nolev, k + 3, 53 - size);
    copy_xpm_area(67, 0, 1, rxlev, k + 3, 53 - rxlev);
    hist += 4;
  }
  copy_xpm_area(70, 1, 58, 1, 3, 53 - size);
}
#endif

#ifdef USE_DRW_MGRAPH
/* MGraph mode */
void
drwMGraph(unsigned long* hist, unsigned mIn, unsigned mOut,
    unsigned size, int bpp, unsigned long long rx_max,
    unsigned long long tx_max)
{
  /* max scale */
  unsigned int m = MAX(tx_max, rx_max);
  unsigned int dfac, k;
  
  if(m > size)
  {
    dfac = m / size;
    if(m % size > 0)
      ++dfac;
  }
  else
    dfac = 1;

  /* process the whole history */
  for(k = 0; k < 58; ++k)
  {
    unsigned int txlev, rxlev;
    rxlev = hist[mIn] / dfac;
    txlev = hist[mOut] / dfac;

    // clear the top area
    copy_xpm_area(65, 0, 1, size - MAX(rxlev, txlev), k + 3, 53 - size);

    // draw the tx/rx bars
    if(rxlev > txlev)
    {
      // rx is major
      copy_xpm_area(67, 0, 1, rxlev - txlev, k + 3, 53 - rxlev);
      copy_xpm_area(66, 0, 1, txlev, k + 3, 53 - txlev);
    }
    else
    {
      // tx is major
      copy_xpm_area(66, 0, 1, txlev - rxlev, k + 3, 53 - txlev);
      copy_xpm_area(67, 0, 1, rxlev, k + 3, 53 - rxlev);
    }

    // advance
    hist += 4;
  }
  copy_xpm_area(70, 1, 58, 1, 3, 53 - size);
}
#endif

#ifdef USE_DRW_WAVEFORM
/* Waveform mode */
void
drwWaveform(unsigned long* hist, unsigned mIn, unsigned mOut,
    unsigned size, int bpp, unsigned long long rx_max,
    unsigned long long tx_max)
{
  unsigned int k;
  for(k = 0; k < 58; k++) {
    unsigned int txlev, rxlev, center;
    rxlev = hist[mIn] / bpp / 2;	/* we need them like this */
    txlev = hist[mOut] / bpp / 2;	/* we need it like this */
    center = 53 - size / 2;
    copy_xpm_area(65, 0, 1, size, k + 3, 53 - size);
    copy_xpm_area(66, 0, 1, txlev, k + 3,
        (center - rxlev) - txlev);
    copy_xpm_area(66, 0, 1, txlev, k + 3, (center + rxlev));
    copy_xpm_area(67, 0, 1, rxlev * 2, k + 3, center - rxlev);
    hist += 4;
  }
  copy_xpm_area(70, 1, 58, 1, 3, 53 - size);
  copy_xpm_area(70, 0, 58, 1, 3, 53 - size / 2);
}
#endif

#ifdef USE_DRW_WMWAVE
/* Reverse wmnet mode */
void
drwWmwave(unsigned long* hist, unsigned mIn, unsigned mOut,
    unsigned size, int bpp, unsigned long long rx_max,
    unsigned long long tx_max)
{
  unsigned int k, msize, dfac, mlev;

  msize = (unsigned)((double)size / 2);
  mlev = MAX(tx_max, rx_max);
  if(mlev > msize)
  {
    dfac = mlev / msize;
    if (mlev % msize > 0)
      dfac++;
  }
  else
    dfac = 1;

  for(k = 0; k < 58; k++)
  {
    unsigned int txlev, rxlev, center;
    txlev = hist[mOut] / dfac;
    rxlev = hist[mIn] / dfac;
    center = 53 - msize;

    copy_xpm_area(65, 0, 1, size, k + 3, 53 - size);
    copy_xpm_area(66, 0, 1, txlev, k + 3, center - txlev);
    copy_xpm_area(67, 0, 1, rxlev, k + 3, center);

    hist += 4;
  }
  copy_xpm_area(70, 1, 58, 1, 3, 53 - size);
  copy_xpm_area(70, 0, 58, 1, 3, 53 - msize);
}
#endif

#ifdef USE_DRW_WMNET
/* Wmnet like modeness */
void
drwWmnet(unsigned long* hist, unsigned mIn, unsigned mOut, unsigned size,
    int bpp, unsigned long long rx_max, unsigned long long tx_max)
{
  unsigned int k;
  for(k = 0; k < 58; k++)
  {
    unsigned int txlev, rxlev, nolev;
    txlev = hist[mOut] / bpp;
    rxlev = hist[mIn] / bpp;
    nolev = size - (rxlev + txlev);
    copy_xpm_area(66, 0, 1, txlev, k + 3, 53 - size);
    copy_xpm_area(65, 0, 1, nolev, k + 3, (53 - size) + txlev);
    copy_xpm_area(67, 0, 1, rxlev, k + 3, 53 - rxlev);
    hist += 4;
  }
  copy_xpm_area(70, 1, 58, 1, 3, 53 - size);
}
#endif

#ifdef USE_DRW_SEPGRAPHS
/* Separated graphs mode */
void
drwSepgraphs(unsigned long* hist, unsigned mIn, unsigned mOut,
    unsigned size, int bpp, unsigned long long rx_max,
    unsigned long long tx_max)
{
  unsigned int k, msize, Txdfac, Rxdfac;

  msize = (unsigned)((double)size / 2);
  if(tx_max > msize)
  {
    Txdfac = tx_max / msize;
    if (tx_max % msize > 0)
      Txdfac++;
  }
  else
    Txdfac = 1;
  if(rx_max > msize)
  {
    Rxdfac = rx_max / msize;
    if (rx_max % msize > 0)
      Rxdfac++;
  }
  else
    Rxdfac = 1;

  for(k = 0; k < 58; k++)
  {
    unsigned int txlev, rxlev, center;
    txlev = hist[mOut] / Txdfac;
    rxlev = hist[mIn] / Rxdfac;
    center = 53 - msize;

    copy_xpm_area(65, 0, 1, size, k + 3, 53 - size);
    copy_xpm_area(66, 0, 1, txlev, k + 3, center - txlev);
    copy_xpm_area(67, 0, 1, rxlev, k + 3, 53 - rxlev);

    hist += 4;
  }
  copy_xpm_area(70, 1, 58, 1, 3, 53 - size);
  copy_xpm_area(70, 0, 58, 1, 3, 53 - msize);
}
#endif

#ifdef USE_DRW_TWISTED
/* Twisted mode */
void
drwTwisted(unsigned long* hist, unsigned mIn, unsigned mOut,
    unsigned size, int bpp, unsigned long long rx_max,
    unsigned long long tx_max)
{
  unsigned int k, stpos, itn, dfac, mlev;

  stpos = 53 - size;
  itn = 53 - stpos;

  mlev = MAX(tx_max, rx_max);
  if (mlev > 29)
  {
    dfac = mlev / 29;
    if (mlev % 29 > 0)
      dfac++;
  }
  else
    dfac = 1;

  hist += 4 * (58-itn);
  for(k = 0; k < itn; k++)
  {
    unsigned int txlev, rxlev, center;
    txlev = hist[mOut] / dfac;
    rxlev = hist[mIn] / dfac;
    center = 29;

    /* Tx on the right, flowing up */
    copy_xpm_area(70, 3, 29, 1, 32, stpos + k);
    if (txlev)
      copy_xpm_area(70, 4, txlev, 1, 32 + (29-txlev)/2, stpos + k)
    else
      copy_xpm_area(70, 0, 1, 1, 46, stpos + k);

    /* Rx on the left, flowing down */
    copy_xpm_area(70, 3, center, 1, 3, 52 - k);
    if (rxlev)
      copy_xpm_area(70, 5, rxlev, 1, 3 + (center-rxlev)/2, 52 - k)
    else
      copy_xpm_area(70, 0, 1, 1, 17, 52 - k);

    hist += 4;
  }
  copy_xpm_area(70, 1, 58, 1, 3, 53 - size);
}
#endif

#if defined(USE_DRW_CHARTS) || defined(USE_DRW_NEEDLE)
/* func utils */
void
drwGetMeds(const unsigned long* hist, const unsigned mIn,
    const unsigned mOut, unsigned long* tx_med,
    unsigned long* rx_med)
{
  unsigned int k;
  *tx_med = *rx_med = 0;
  for(k = 0; k < 58; k++)
  {
    *tx_med += hist[mOut];
    *rx_med += hist[mIn];
    hist += 4;
  }
  *tx_med /= 58;
  *rx_med /= 58;
}
#endif

#ifdef USE_DRW_CHARTS
/* Charts mode */
void drwCharts(unsigned long* hist, unsigned mIn, unsigned mOut, unsigned size,
    int bpp, unsigned long long rx_max, unsigned long long tx_max)
{
  unsigned int k, j, stpos, spc;
  unsigned long tx_med = 0, rx_med = 0, x_max;

  stpos = 53 - size;

  drwGetMeds(hist, mIn, mOut, &tx_med, &rx_med);

  hist +=  4 * 57;
  spc = (size - 34) / 3;
  x_max = MAX(rx_max, tx_max);
  if (!x_max)
    x_max = 1;

  /* clear zones */
  copy_xpm_area(70, 3, 58, 1, 3, stpos + 1);
  copy_xpm_area(70, 1, 58, 1, 3, 53 - size);
  copy_xpm_area(70, 3, 58, 1, 3, 52);
  for(k = 0; k < 3; k++)
    for(j = 0; j < spc; j++)
      copy_xpm_area(70, 3, 58, 1, 3, stpos + 10 + (8*k) + (k*spc) + j);

  /* tx current */
  copy_xpm_area(70, 8, 58, 8, 3, stpos + 2);
  copy_xpm_area(70, 17, (int)((double)54*hist[mOut]/x_max), 6, 5, stpos + 3);

  /* tx average */
  copy_xpm_area(70, 8, 58, 8, 3, stpos + 10 + spc);
  copy_xpm_area(70, 17, (int)((double)54*tx_med/x_max), 6, 5, stpos + 11 + spc);

  /* rx current */
  copy_xpm_area(70, 8, 58, 8, 3, stpos + 18 + spc*2);
  copy_xpm_area(70, 17, (int)((double)54*hist[mIn]/x_max), 6, 5,
      stpos + 19 + spc * 2);

  /* rx average */
  copy_xpm_area(70, 8, 58, 8, 3, stpos + 26 + spc*3);
  copy_xpm_area(70, 17, (int)((double)54*rx_med/x_max), 6, 5,
      stpos + 27 + spc * 3);
}
#endif

#ifdef USE_DRW_NEEDLE
// simple utility function for rotating points
void
drwRotPoint(const unsigned len, const double rad,
    unsigned* nX, unsigned* nY)
{
  *nX = (unsigned)((double)len * sin(rad));
  *nY = (unsigned)((double)len * cos(rad));
}

/* needle mode */
void drwNeedle(unsigned long* hist, unsigned mIn, unsigned mOut, unsigned size,
    int bpp, unsigned long long rx_max, unsigned long long tx_max)
{
  unsigned int k, stpos, offset, nX, nY;
  unsigned long tx_med = 0, rx_med = 0, x_max;
  double angle;

  static int gcinit = 0;
  static GC gcs[3];

  /* gc initialization */
  if (!gcinit)
  {
    XGCValues gcval;

    gcval.foreground = dockapp.stdColors.txColor;
    gcval.graphics_exposures = False;
    gcs[0] = XCreateGC(dockapp.d, dockapp.pixmap, GCForeground |
        GCGraphicsExposures, &gcval);
    gcval.foreground = dockapp.stdColors.rxColor;
    gcs[1] = XCreateGC(dockapp.d, dockapp.pixmap, GCForeground |
        GCGraphicsExposures, &gcval);
    gcval.foreground = dockapp.stdColors.mdColor;
    gcs[2] = XCreateGC(dockapp.d, dockapp.pixmap, GCForeground |
        GCGraphicsExposures, &gcval);
    gcinit = 1;
  }

  stpos = 53 - size;
  offset = (size - 34) / 2 + 1;
  
  /* background pixmap */
  copy_xpm_area(129, 0, 58, 34, 3, stpos + offset);
  
  /* clean up */
  copy_xpm_area(70, 1, 58, 1, 3, 53 - size);
  for(k = 1; k < offset; k++)
  {
    copy_xpm_area(70, 3, 58, 1, 3, stpos + k);
    copy_xpm_area(70, 3, 58, 1, 3, 53 - k);
  }

  drwGetMeds(hist, mIn, mOut, &tx_med, &rx_med);
  x_max = MAX(rx_max, tx_max);
  if (!x_max)
    x_max=1;

  /* tx needle */
  angle = 1.57 * tx_med / x_max;
  drwRotPoint(13, angle, &nX, &nY);

  XDrawLine(dockapp.d, dockapp.pixmap, gcs[0], 5, stpos + offset + 2,
      5 + nX, stpos + offset + 2 + nY);

  /* rx needle */
  angle = 1.57 * rx_med / x_max;
  drwRotPoint(13, angle, &nX, &nY);

  XDrawLine(dockapp.d, dockapp.pixmap, gcs[1], 5, stpos + offset + 31,
      5 + nX, stpos + offset + 31 - nY);

  /* central needle */
  if (!(tx_max+rx_max))
    tx_max=1;

  angle = 0.65*(rx_med+tx_med)/(rx_max+tx_max)+1.25;
  drwRotPoint(44, angle, &nX, &nY);

  XDrawLine(dockapp.d, dockapp.pixmap, gcs[2], 14, stpos + offset + 17,
      14 + nX, stpos + offset + 17 + nY);
}
#endif

#ifdef USE_DRW_LINES
/* lines mode */
void drwLines(unsigned long* hist, unsigned mIn, unsigned mOut, unsigned size,
    int bpp, unsigned long long rx_max, unsigned long long tx_max)
{
  unsigned int m = MAX(tx_max, rx_max);
  unsigned int dfac, k, oTxlev, oRxlev;

  static int gcinit = 0;
  static GC gcs[2];

  /* gc initialization */
  if (!gcinit)
  {
    XGCValues gcval;

    gcval.foreground = dockapp.stdColors.txColor;
    gcval.graphics_exposures = False;
    gcs[0] = XCreateGC(dockapp.d, dockapp.pixmap, GCForeground |
        GCGraphicsExposures, &gcval);
    gcval.foreground = dockapp.stdColors.rxColor;
    gcs[1] = XCreateGC(dockapp.d, dockapp.pixmap, GCForeground |
        GCGraphicsExposures, &gcval);
    gcinit = 1;
  }

  /* XDrawLine is a bit piggy sometimes, reduce avaible size by two */
  if((m + 2) > size)
  {
    dfac = m / (size - 2);
    if(m % (size - 2) > 0)
      ++dfac;
  }
  else
    dfac = 1;

  /* fake old values */
  oRxlev = hist[mIn] / dfac;
  oTxlev = hist[mOut] / dfac;

  for(k = 0; k < 58; k++)
  {
    unsigned int txlev, rxlev;

    rxlev = hist[mIn] / dfac;
    txlev = hist[mOut] / dfac;

    /* clear the area */
    copy_xpm_area(65, 0, 1, size, k + 3, 53 - size);

    /* tx and rx */
    XDrawLine(dockapp.d, dockapp.pixmap, gcs[0],
        k + 3, 52 - oTxlev, k + 3, 52 - txlev);
    XDrawLine(dockapp.d, dockapp.pixmap, gcs[1],
        k + 3, 52 - oRxlev, k + 3, 52 - rxlev);

    /* advance */
    hist += 4;
    oTxlev = txlev;
    oRxlev = rxlev;
  }

  copy_xpm_area(70, 1, 58, 1, 3, 53 - size);
}
#endif

/* function's structure list */
struct drwStruct drwFuncs[] = 
{
#ifdef USE_DRW_TRADITIONAL
  { "traditional", drwTraditional },
#endif
#ifdef USE_DRW_MGRAPH
  { "mgraph",      drwMGraph      },
#endif
#ifdef USE_DRW_WAVEFORM
  { "waveform",    drwWaveform    },
#endif
#ifdef USE_DRW_WMWAVE
  { "wmwave",      drwWmwave      },
#endif
#ifdef USE_DRW_WMNET
  { "wmnet",       drwWmnet       },
#endif
#ifdef USE_DRW_SEPGRAPHS
  { "sepgraphs",   drwSepgraphs   },
#endif
#ifdef USE_DRW_TWISTED
  { "twisted",     drwTwisted     },
#endif
#ifdef USE_DRW_CHARTS
  { "charts",      drwCharts      },
#endif
#ifdef USE_DRW_NEEDLE
  { "needle",      drwNeedle      },
#endif
#ifdef USE_DRW_LINES
  { "lines",       drwLines       },
#endif
  { NULL,          NULL,          }
};

