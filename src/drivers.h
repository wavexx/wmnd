/*
 * drivers: drivers definitions
 * Copyright(c) 2001-2008 by wave++ "Yuri D'Elia" <wavexx@users.sf.net>
 * Distributed under GNU GPL (v2 or above) WITHOUT ANY WARRANTY.
 */

#ifndef DRIVERS_H
#define DRIVERS_H

#include "config.h"
#include "wmnd.h"

/*
 * Note to driver implementors:
 *
 * - p_list_devs is called first to list all the *supported* interfaces.
 *   Loopback/testing interfaces must be skipped unless specified.
 *   If a device name is specified, only this device must be listed.
 *   The driver "should" support comma-separated list of interfaces as the
 *   "devname" argument, but this is subject to change.
 *   The number of allocated interfaces is returned.
 *
 * - p_init_dev is called for each allocated interface.
 *
 * - p_updt_stats is polled to gather interface statistics.
 *   The function must return the interface status.
 *
 * - p_term_dev is called during shutdown for each interface.
 *
 * - p_term_drv is called during shutdown once.
 */

typedef int (*p_list_devs) (const char* devname, struct Devices* list);
typedef int (*p_init_dev) (struct Devices* dev);
typedef int (*p_updt_stats) (struct Devices* dev, unsigned long* ip,
    unsigned long* op, unsigned long* ib, unsigned long* ob);
typedef void (*p_term_dev) (struct Devices* dev);
typedef void (*p_term_drv) ();

/* drivers structure list */
struct drivers_struct
{
  /* base params */
  char* driver_name;
  p_list_devs list_devices;
  p_init_dev init_device;
  p_updt_stats get_stats;
  p_term_dev terminate_device;
  p_term_drv terminate_driver;

  /* extra data */
  int status;
};

/* external definition */
extern struct drivers_struct drivers_table[];

#endif
