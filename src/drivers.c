/*
 * wmnd - window maker network devices - drivers.c
 *
 * drivers definitions
 * $Id$
 */

/* local headers */
#include "config.h"
#include "drivers.h"
#include "messages.h"

/*
 * _list: allocate new elements in the list and assign a name for each device
 * _init: callled for each device allocated by *_list. Prepare device status
 * _getstats: obtain infos about the device
 * _term: eventually deallocate drvdata and close fds
 */

#ifdef USE_SOLARIS_FPPPD
#undef drName
#define drName USE_SOLARIS_FPPPD

/*
 * this driver should work on any system that could compile and execute pppd
 * for linux/solaris. Had require some hacking and has some drawbacks but
 * finally works. always detect at least one device since you can't detect the
 * number of futher connections. The _get function also required a bit of hack
 * to manage correctly the status of the device without messing the graph
 */

// some needed headers
#include <sys/stropts.h>
#include "ppp_defs.h"
#include "pppio.h"
#include <errno.h>

/* on some systems may be /dev/streams/ppp */
const char* solaris_fpppd_device = "/dev/ppp";

/* connection status (needed for dialup devices) */
enum solaris_fpppd_connstatus
{
  sfpppd_untested,
  sfpppd_failed,
  sfpppd_opened,
  sfpppd_connected
};

/* driver data in Devices, used for coding style */
struct solaris_fpppd_drvdata
{
  int fd;
  int unit;
  enum solaris_fpppd_connstatus mustconnect;
};

/* strioclt is a support function, not previously declared */
int
solaris_fpppd_strioctl(int fd, int cmd, char* ptr, int ilen, int olen)
{
  struct strioctl str;

  str.ic_cmd = cmd;
  str.ic_timout = 0;
  str.ic_len = ilen;
  str.ic_dp = ptr;
  if(ioctl(fd, I_STR, &str) == -1)
    return -1;
  if(str.ic_len != olen)
    msg_drInfo(drName, "strioctl expected %d bytes, got %d for cmd %x",
        olen, str.ic_len, cmd);
  return 0;
}

int
solaris_fpppd_init(struct Devices* dev)
{
  struct solaris_fpppd_drvdata* drvdata = dev->drvdata;

  /* opening fd */
  if(drvdata->mustconnect < sfpppd_opened)
  {
    dev->devstart = 0;
    if((drvdata->fd = open(solaris_fpppd_device, O_RDONLY)) < 0)
    {
      drvdata->mustconnect = sfpppd_failed;
      return 1;
    }
    else
    if(drvdata->mustconnect < sfpppd_opened)
    {
      if(drvdata->mustconnect == sfpppd_untested)
      {
        /* supposing the device is down */
        drvdata->mustconnect = sfpppd_opened;
        return 0;
      }
      drvdata->mustconnect = sfpppd_opened;
    }
  }

  /* attaching to fd device's signals */
  if(drvdata->mustconnect < sfpppd_connected)
  {
    if(solaris_fpppd_strioctl(drvdata->fd, PPPIO_ATTACH,
        (char*)&drvdata->unit, sizeof(int), 0) < 0)
    {
      drvdata->mustconnect = sfpppd_opened;
      dev->devstart = 0;
      return 1;
    }
    else
    {
      drvdata->mustconnect = sfpppd_connected;
      time(&dev->devstart);
    }
  }

  return 0;
}

void
solaris_fpppd_term(struct Devices* dev)
{
  close(((struct solaris_fpppd_drvdata*) dev->drvdata)->fd);
  free(dev->drvdata);
}

int
solaris_fpppd_list(const char* devname, struct Devices* list)
{
  int unit;
  char* devn = NULL;
  struct Devices* ndev;
  struct solaris_fpppd_drvdata* ndata;
  struct Devices* ptr;
  int dta;
  
  /* device name testing */
  if(devname)
    devn = strdup(devname);
  
  if(devn) /* device specified */	
  {
    if (sscanf(devn, "ppp%d", &unit) != 1)
    {
      msg_drInfo(drName, "invalid specified interface '%s'", devn);
      free(devn);
      return 0;
    }
    dta = 1; /* allocate only one device, using unit */
  }
  else
  {
    /* check all devices from 0 until error */
    ndev = malloc(sizeof(struct Devices));
    ndev->name = NULL;
    unit = 0;
    while(1)
    {
      ndata = malloc(sizeof(struct solaris_fpppd_drvdata));
      ndata->mustconnect = sfpppd_failed;
      ndata->unit = unit;
      ndev->drvdata = ndata;

      dta = solaris_fpppd_init(ndev);
      solaris_fpppd_term(ndev);

      if(dta)
        /* device initialization failed */
        break; 

      msg_drInfo(drName, "detected ppp%d",unit);
      unit++;
    }
    free(ndev);

    if(!unit)
      return 0;
      
    dta = unit;
    unit = 0;
  }
  
  /* allocate new dta devices structures */
  ptr = list;
  while(dta--)
  {
    ndata = malloc(sizeof(struct solaris_fpppd_drvdata));
    ndata->mustconnect = sfpppd_untested;
    ndata->unit = unit;

    ndev = malloc(sizeof(struct Devices));
    ptr->next = ndev;
    ptr = ndev;

    ndev->next = NULL;
    ndev->drvdata = ndata;

    ndev->name = malloc(10);
    sprintf(ndev->name,"ppp%d",unit);

    unit++;
  }

  return (devn)?free(devn),1:unit;
}

int
solaris_fpppd_get(struct Devices* dev, unsigned long* ip,
    unsigned long* op, unsigned long* ib, unsigned long* ob)
{
  static struct ppp_stats curp;
  struct solaris_fpppd_drvdata* drvdata = dev->drvdata;

  /* clear vars (not clearing will fuck up all scales when disconnecting */
  *ip = *op = *ib = *ob = 0;

  if(drvdata->mustconnect < sfpppd_connected)
  {
    /* trying to activate the device */
    if (solaris_fpppd_init(dev))
      return 1;
  }
  if(solaris_fpppd_strioctl(drvdata->fd, PPPIO_GETSTAT, (char* )&curp,
      0, sizeof(curp)) < 0)
  {
    /* the connection has probably shutted down */
    drvdata->mustconnect = sfpppd_failed;
    close(drvdata->fd);
    return 1;
  }

  *ip = curp.p.ppp_ipackets;
  *ib = curp.p.ppp_ibytes;
  *op = curp.p.ppp_opackets;
  *ob = curp.p.ppp_obytes;

  return 0;
}

#endif /* USE_SOLARIS_FPPPD */


#ifdef USE_SOLARIS_KSTAT
#undef drName
#define drName USE_SOLARIS_KSTAT

/* basic kstat header */
#include <kstat.h>

static kstat_ctl_t* solaris_kstat_kc = NULL;

/* driver data in Devices, used for coding style */
struct solaris_kstat_drvdata
{
  kstat_t* ksp;
  kstat_named_t* active;
  kstat_named_t* in_pkt;
  kstat_named_t* in_byte;
  kstat_named_t* out_pkt;
  kstat_named_t* out_byte;
};

int
solaris_kstat_list(const char* devname, struct Devices* list)
{
  struct Devices* ndev;
  struct solaris_kstat_drvdata* ndata;
  struct Devices* ptr;
  kstat_t* ksp;
  int dta = 0;

  /* opening /dev/kstat */
  solaris_kstat_kc = kstat_open();
  if(!solaris_kstat_kc)
  {
    msg_drInfo(drName, "unable to open /dev/kstat");
    return 0;
  }

  /* check all devices of class net */
  ptr = list;
  for(ksp = solaris_kstat_kc->kc_chain; ksp; ksp = ksp->ks_next)
  {
    if(!strcmp(ksp->ks_class, "net"))
    {
      kstat_read(solaris_kstat_kc, ksp, NULL);
      if((!devname || (devname && !strcmp(devname,ksp->ks_name))) &&
          kstat_data_lookup(ksp, "link_up") &&
          kstat_data_lookup(ksp, "ipackets") &&
          kstat_data_lookup(ksp, "opackets") &&
          kstat_data_lookup(ksp, "rbytes") &&
          kstat_data_lookup(ksp, "obytes"))
      {
        /* device is suitable */
        ndev = (struct Devices*)malloc(sizeof(struct Devices));
        ndev->devstart = 0;
        ndev->name = strdup(ksp->ks_name);
        ndata = (struct solaris_kstat_drvdata*)
          malloc(sizeof(struct solaris_kstat_drvdata));
        ndata->ksp = ksp;
        ptr->next = ndev;
        ndev->next = NULL;
        ndev->drvdata = ndata;
        ptr = ndev;

        msg_drInfo(drName, "detected %s",ndev->name);
        dta++;
      }
    }
  }

  return dta;
}

int
solaris_kstat_init(struct Devices* dev)
{
  struct solaris_kstat_drvdata* drvdata =
    (struct solaris_kstat_drvdata*)dev->drvdata;

  drvdata->active = (kstat_named_t*)kstat_data_lookup(drvdata->ksp,"link_up");
  drvdata->in_pkt = (kstat_named_t*)kstat_data_lookup(drvdata->ksp,"ipackets");
  drvdata->in_byte = (kstat_named_t*)kstat_data_lookup(drvdata->ksp,"rbytes");
  drvdata->out_pkt = (kstat_named_t*)kstat_data_lookup(drvdata->ksp,"opackets");
  drvdata->out_byte = (kstat_named_t*)kstat_data_lookup(drvdata->ksp,"obytes");

  return !(drvdata->active && drvdata->in_pkt && drvdata->in_byte &&
      drvdata->out_pkt && drvdata->out_byte);
}

int
solaris_kstat_get(struct Devices* dev, unsigned long* ip, unsigned long* op,
    unsigned long* ib, unsigned long* ob)
{
  struct solaris_kstat_drvdata* drvdata =
    (struct solaris_kstat_drvdata*)dev->drvdata;
  *ip = *op = *ib = *ob = 0;

  if(kstat_read(solaris_kstat_kc, drvdata->ksp, NULL) == -1)
    return 1;
  else
  {
    *ip = (drvdata->in_pkt->value.ui32);
    *op = (drvdata->out_pkt->value.ui32);
    *ib = (drvdata->in_byte->value.ui32);
    *ob = (drvdata->out_byte->value.ui32);
  }

  return !drvdata->active->value.ui32;
}

void
solaris_kstat_term(struct Devices* dev)
{
  free(dev->drvdata);
}

#endif /* USE_SOLARIS_KSTAT */


#ifdef USE_TESTING_DUMMY
#undef drName
#define drName USE_TESTING_DUMMY

#ifdef USE_SINE_TESTING_DUMMY
#include <math.h>

/* we need the double of PI, not defined everywhere */
#undef M_2PI
#define M_2PI M_PI * 2

#endif

int
testing_dummy_list(const char* devname, struct Devices* list)
{
  char* devn;
  struct Devices* ndev;

  if(!devname)
    devn = strdup("off");
  else
    devn = strdup(devname);

  ndev = (struct Devices*)malloc(sizeof(struct Devices));
  list->next = ndev;

  ndev->next = NULL;
  ndev->name = devn;

  msg_drInfo(drName, "detected %s", devn);

  return 1; /* usually returns the number of devices */
}

int
testing_dummy_init(struct Devices* dev)
{
  dev->devstart = 0;
  return 0;
}

int
testing_dummy_get(struct Devices* dev, unsigned long* ip, unsigned long* op,
    unsigned long* ib, unsigned long* ob)
{
#ifdef USE_SINE_TESTING_DUMMY
  /* some more fun when debugging! */
  static float v = 0.;
  static unsigned long is = 0;
  static unsigned long os = 0;

  *ip = *ib = (is += (16384. * cos(v)) + 16384);
  *op = *ob = (os += (16384. * sin(v)) + 16384);
  v += 0.05;

  if(v >= M_PI * 2)
    v = 0;
#else
  *ip = *op = *ib = *ob = 0;
#endif
  return 1;
}

void
testing_dummy_term(struct Devices* dev)
{}

#endif /* USE_TESTING_DUMMY */


#ifdef USE_LINUX_PROC
#undef drName
#define drName USE_LINUX_PROC

/*
 * this driver is badly optimized, re-reading each time the file consumes
 * many cpu clocks, only for finding the device name, however we cannot use
 * an offset so it's the only way to get some kind of statistics. Definitively
 * the kstat driver seems the best way to gather statistics.
 */

const char* linux_proc_netDevice = "/proc/net/dev";

int
linux_proc_list(const char* devname, struct Devices* list)
{
  FILE* fd;
  int dta = 0;
  char temp[MAXBUF]; /* string buffer */
  char* tokens = " :\t\n"; /* parse tokens */
  struct Devices* ndev;
  char* p;

  /* device name was specified */
  if(devname)
  {
    dta = 1;
    ndev = (struct Devices*)malloc(sizeof(struct Devices));
    ndev->devstart = 0;
    ndev->name = strdup(devname);
    ndev->next = ndev->drvdata = NULL;
    list->next = ndev;

    msg_drInfo(drName, "forced %s", p);
  }
  else
  {
    /* fetch all devices */
    fd = fopen(linux_proc_netDevice, "r");
    if(!fd) return 0;

    /* Skip the first 2 lines */
    fgets(temp, MAXBUF, fd);
    fgets(temp, MAXBUF, fd);

    while(fgets(temp, MAXBUF, fd))
    {
      /* grab all active devices, adding them as we go */
      p = strtok(temp, tokens);
      if(!strncmp(p, "dummy", 5) ||
          !strncmp(p, "irda", 4) || !strncmp(p, "lo", 3))
        continue;

      dta++;
      ndev = (struct Devices*)malloc(sizeof(struct Devices));
      ndev->devstart = 0;
      ndev->name = strdup(p);
      ndev->next = ndev->drvdata = NULL;
      list->next = ndev;
      list = ndev;

      msg_drInfo(drName, "detected %s", p);
    }
    fclose(fd);
  }

  return dta;
}

int
linux_proc_init(struct Devices* dev)
{
  /* linux proc doesn't need single device initialization */
  return 0;
}

int
linux_proc_get(struct Devices* dev, unsigned long* ip, unsigned long* op,
    unsigned long* ib, unsigned long* ob)
{
  FILE* fp;
  char temp[MAXBUF];
  char* p;
  int active = 1;

  /* read statistics from network device's list */
  fp = fopen(linux_proc_netDevice, "r");
  fgets(temp, MAXBUF, fp);
  fgets(temp, MAXBUF, fp);

  *ib = *ob = *ip = *op = 0;

  while(fgets(temp, MAXBUF, fp))
    if(strstr(temp, dev->name))
    {
      p = strchr(temp, ':');
      ++p;
      sscanf(p, "%lu %lu %*s %*s %*s %*s %*s %*s %lu %lu", ib, ip, ob, op);
      active = 0;
      break;
    }
  fclose(fp);

  return active;
}

void
linux_proc_term(struct Devices* dev)
{}

#endif /* USE_LINUX_PROC */


/* FreeBDS sysctl driver */
#ifdef USE_FREEBSD_SYSCTL
#undef drName
#define drName USE_FREEBSD_SYSCTL

/* system headers */
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_mib.h>
#include <sys/errno.h>

/* we need a structure to hold the device id and the driver data */
struct freebsd_sysctl_drvdata
{
  struct ifmibdata* data; /* device data */
  int id; /* device id */
};

/* device listing */
int
freebsd_sysctl_list(const char* devname, struct Devices* list)
{
  struct Devices* ndev;
  struct Devices* ptr;
  struct ifmibdata tempndata;
  struct freebsd_sysctl_drvdata* drdata;
  int numifaces, numrfaces = 0;
  int mib[5], datamib[6];
  int i, len, len2;
  
  mib[0] = CTL_NET;
  mib[1] = PF_LINK;
  mib[2] = NETLINK_GENERIC;
  mib[3] = IFMIB_SYSTEM;
  mib[4] = IFMIB_IFCOUNT;
         
  datamib[0] = CTL_NET;
  datamib[1] = PF_LINK;
  datamib[2] = NETLINK_GENERIC;
  datamib[3] = IFMIB_IFDATA;
  datamib[4] = 1; 
  datamib[5] = IFDATA_GENERAL;

  len = sizeof(numifaces);
  len2 = sizeof(struct ifmibdata);

  if(sysctl(mib, 5, &numifaces, &len, NULL, 0) < 0)
  {
    msg_drInfo(drName, "failed to perform sysctl");
    return 0;
  }

  ptr = list;
  for(i = 1; i <= numifaces; i++)
  {
    datamib[4] = i;
    if(sysctl(datamib, 6, &tempndata, &len2, NULL, 0) < 0)
    {
      msg_drInfo(drName, "failed to get device(%d) data", i);
      break;
    }

    if((devname && !strcmp(devname, tempndata.ifmd_name)) || (!devname &&
        strcmp(tempndata.ifmd_name, "lo")))
    {
      ndev = malloc(sizeof(struct Devices));
      ndev->devstart = 0;
      ndev->name = strdup(tempndata.ifmd_name);
      drdata = malloc(sizeof(struct freebsd_sysctl_drvdata));
      drdata->data = malloc(sizeof(struct ifmibdata));
      memcpy(drdata->data, &tempndata, sizeof(struct ifmibdata));
      drdata->id = i;
      ptr->next = ndev;
      ndev->next = NULL;
      ndev->drvdata = drdata;
      ptr = ndev;

      msg_drInfo(drName, "detected %s", ndev->name);

      /* increment the number of really avaible interfaces */
      ++numrfaces;
    }
  }

  return numrfaces;
}

/* per-interface initialization */
int
freebsd_sysctl_init(struct Devices* dev)
{
  /* FreeBSD doesn't need any device init */
  return 0;
}

/* gather stats */
int
freebsd_sysctl_get(struct Devices*dev, unsigned long* ip,
    unsigned long* op, unsigned long* ib, unsigned long* ob)
{
  struct freebsd_sysctl_drvdata* drdata = dev->drvdata;
  int datamib[6];
  int len;

  *ip = *op = *ib = *ob = 0;

  datamib[0] = CTL_NET;
  datamib[1] = PF_LINK;
  datamib[2] = NETLINK_GENERIC;
  datamib[3] = IFMIB_IFDATA;
  datamib[4] = drdata->id;
  datamib[5] = IFDATA_GENERAL;

  len = sizeof(struct ifmibdata); 

  if(sysctl(datamib, 6, drdata->data, &len, NULL, 0) < 0)
    return 1;

  *ip = drdata->data->ifmd_data.ifi_ipackets;
  *op = drdata->data->ifmd_data.ifi_opackets;
  *ib = drdata->data->ifmd_data.ifi_ibytes;
  *ob = drdata->data->ifmd_data.ifi_obytes;

  return 0;
}

/* per-interface deallocation */
void
freebsd_sysctl_term(struct Devices* dev)
{
  struct freebsd_sysctl_drvdata* drdata = dev->drvdata;
  free(drdata->data);
  free(drdata);
}

#endif /* USE_FREEBSD_SYSCTL */


/* IRIX's Performance Co-Pilot (PMCAPI 2.x) */
#ifdef USE_IRIX_PCP
#undef drName
#define drName USE_IRIX_PCP

/* PCP API headers */
#include <pcp/pmapi.h>

/* PCP NameSpace definitions */
#define PCPNS_NETINBDOM "network.interface.in.bytes"
#define PCPNS_NETINPDOM "network.interface.in.packets"
#define PCPNS_NETOUTBDOM "network.interface.out.bytes"
#define PCPNS_NETOUTPDOM "network.interface.out.packets"

struct irix_pcp_drvdata
{
  pmInDom indom;
  int inst;
};

/* resolve a PCP domain/desc */
int
irix_pcp_resDom(char* dom, pmID* pmId, pmDesc* pmD)
{
  int r;
  if((r = pmLookupName(1, &dom, pmId)) < 0)
  {
    msg_drInfo(drName, "unable to lookup %s: %s", dom, pmErrStr(r));
    return -1;
  }

  if((r = pmLookupDesc(*pmId, pmD)) < 0)
  {
    msg_drInfo(drName, "unable to get descriptions about %s: %s",
        dom, pmErrStr(r));
    return -1;
  }

  return 0;
}

int
irix_pcp_list(const char* devname, struct Devices* list)
{
  pmID pmId[4];
  pmDesc pmD[4];
  int* inst;
  char** desc;

  int dta = 0;
  int rad = 0;
  struct Devices* ndev;
  int i;
  int* t;
  char** p;
  struct irix_pcp_drvdata* drdata;


  /* create a connection to pcpd */
  if((dta = pmNewContext(PM_CONTEXT_HOST, "localhost")) < 0)
  {
    msg_drInfo(drName, pmErrStr(dta));
    return 0;
  }

  /* resolve doms/descs */
  if(irix_pcp_resDom(PCPNS_NETINBDOM, &pmId[0], &pmD[0]) ||
      irix_pcp_resDom(PCPNS_NETINPDOM, &pmId[1], &pmD[1]) ||
      irix_pcp_resDom(PCPNS_NETOUTBDOM, &pmId[2], &pmD[2]) ||
      irix_pcp_resDom(PCPNS_NETOUTPDOM, &pmId[3], &pmD[3]))
    return 0;

  /* fetch the hinstance list from any of the four domains */
  if((dta = pmGetInDom(pmD->indom, &inst, &desc)) < 0)
  {
    msg_drInfo(drName, "unable to get hinstances of " PCPNS_NETINBDOM
        ": %s", pmErrStr(dta));
    return 0;
  }

  /* traverse the list */
  p = desc;
  t = inst;
  for(i = 0; i < dta; ++i)
  {
    if((devname && !strcmp(devname, *p)) ||
        (!devname && strcmp(*p, "lo0")))
    {
      msg_drInfo(drName, "detected %s(%d)", *p, *t);
      ndev = malloc(sizeof(struct Devices));
      ndev->name = strdup(*p);
      drdata = malloc(sizeof(struct irix_pcp_drvdata));
      /*drdata->indom = pmD.indom;*/
      drdata->inst = *t;
      ndev->drvdata = (void*)drdata;

      /* append the new device */
      list->next = ndev;
      list = ndev;
      ndev->next = NULL;
      ++rad;
    }

    ++p;
    ++t;
  }
  free(desc);
  free(inst);

  return rad;
}

int
irix_pcp_init(struct Devices* dev)
{
  dev->devstart = 0;
  return 0;
}

int
irix_pcp_get(struct Devices* dev, unsigned long* ip, unsigned long* op,
    unsigned long* ib, unsigned long* ob)
{
  *ip = *op = *ib = *ob = 0;
  return 0;
}

void
irix_pcp_term(struct Devices* dev)
{
  free(dev->drvdata);
}

#endif /* USE_IRIX_PCP */


/* define the drivers list */
struct drivers_struct drivers_table[] =
{
#ifdef USE_FREEBSD_SYSCTL
  {USE_FREEBSD_SYSCTL, freebsd_sysctl_list, freebsd_sysctl_init,
    freebsd_sysctl_get, freebsd_sysctl_term},
#endif
#ifdef USE_LINUX_PROC
  {USE_LINUX_PROC, linux_proc_list, linux_proc_init, linux_proc_get,
    linux_proc_term},
#endif
#ifdef USE_SOLARIS_FPPPD
  {USE_SOLARIS_FPPPD, solaris_fpppd_list, solaris_fpppd_init,
    solaris_fpppd_get, solaris_fpppd_term},
#endif
#ifdef USE_SOLARIS_KSTAT
  {USE_SOLARIS_KSTAT, solaris_kstat_list, solaris_kstat_init,
    solaris_kstat_get, solaris_kstat_term},
#endif
#ifdef USE_IRIX_PCP
  {USE_IRIX_PCP, irix_pcp_list, irix_pcp_init, irix_pcp_get, irix_pcp_term},
#endif
#ifdef USE_TESTING_DUMMY
  {USE_TESTING_DUMMY, testing_dummy_list, testing_dummy_init,
    testing_dummy_get, testing_dummy_term},
#endif
  {NULL, NULL, NULL, NULL, NULL}
};

