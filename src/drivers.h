/*
 * wmnd - window maker network devices - drivers.h
 *
 * drivers prototypes and includes
 */

#ifndef __drivers_h
#define __drivers_h

#include "config.h"
#include "wmnd.h"

/*
 * how a driver works:
 * first wmnd calls p_list_devs, for populating the Devices struct. If the
 * driver does not support listing, then only requested device in devname
 * must be listed. this function returns the number of devices avaible for
 * this driver and fills up the Devices struct.
 * Than the driver will be initialized using p_init_drv. The function
 * must return 0 if successfull.
 * When the driver is up&running p_updt_stats will be called periodically to
 * get device statistics. If it returns 0 the device is ok, 1 for offline.
 * wmnd will continue calling it, even if the device is offline. The driver
 * must detect by itself the device status.
 * finally, when removing a device or exiting, p_term_drv will be called.
 * Actually you can't plug devices on-the-fly so inside p_init_drv you must
 * list ALL devices, whenever they're online or not.
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

