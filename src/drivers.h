/*
 * wmnd - window maker network devices - drivers.h
 * drivers definitions - interface
 * Copyright(c) 2001-2006 by wave++ "Yuri D'Elia" <wavexx@users.sf.net>
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
 * - p_init_drv is called for each allocated interface.
 *
 * - p_updt_stats is polled to gather interface statistics.
 *   The function must return the interface status.
 *
 * - p_term_drv is called during shutdown for each interface.
 */

typedef int (*p_list_devs) (const char* devname, struct Devices* list);
typedef int (*p_init_drv) (struct Devices* dev);
typedef int (*p_updt_stats) (struct Devices* dev, unsigned long* ip,
    unsigned long* op, unsigned long* ib, unsigned long* ob);
typedef void (*p_term_drv) (struct Devices* dev);

/* drivers structure list */
struct drivers_struct
{
  char* driver_name;
  p_list_devs list_devices;
  p_init_drv init_driver;
  p_updt_stats get_stats;
  p_term_drv terminate_driver;
};

/* external definition */
extern struct drivers_struct drivers_table[];

#endif
