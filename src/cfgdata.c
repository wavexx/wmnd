/*
 * WMND configuration data definitions
 */

/* local header */
#include "cfgdata.h"

/* define null if missing */
#ifndef NULL
#define NULL 0x0
#endif

struct pair_strint psi_bool[] =
{
  {"no",0},
  {"yes",1},
  {NULL,0}
};

/*
 * we use a % because we must not use an existing name
 * If you develope a driver, don't name it with a leading %
 * the "used" variable is used to detect forced command line
 * parameters, 0: read config, 1: don't read
 */
struct pair_strstr pss_defcon[] =
{
  {"tx_color",        "#00fff2", 0},
  {"rx_color",        "#188a86", 0},
  {"md_color",        "#71e371", 0},
  {"refresh",         "50000",   0},
  {"scroll",          "10",      0},
  {"avg_steps",       "1",       0},
  {"binary_scale",    "no",      0},
  {"interface_name",  "%first",  0}, 
  {"use_long_names",  "no",      0},
  {"show_max_values", "yes",     0},
  {"use_max_history", "no",      0},
  {"display_time",    "yes",     0},
  {"wave_mode",       "wmnet",   0},
  {"driver",          "%auto",   0},
  {"driver_interface","%any",    0},
  {"quiet",           "no",      0},
  {"smooth",          "0",       0},
  {"name",            "wmnd",    0},
  {NULL,               NULL,     0}
};

