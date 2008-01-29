/*
 * drivers: drivers definitions - implementation
 * Copyright(c) 2001-2008 by wave++ "Yuri D'Elia" <wavexx@users.sf.net>
 * Distributed under GNU GPL (v2 or above) WITHOUT ANY WARRANTY.
 */

/* local headers */
#include "config.h"
#include "drivers.h"
#include "messages.h"


/* common helper functions */
struct Devices*
devices_append(struct Devices* list, struct Devices* src)
{
  list->next = src;
  src->next = NULL;
  return src;
}


#ifdef USE_SOLARIS_FPPPD
#undef drName
#define drName USE_SOLARIS_FPPPD

/*
 * this driver should work on any system that could compile and execute pppd
 * for linux/solaris.
 */

/* some needed headers */
#include <sys/stropts.h>
#ifdef HAVE_NET_PPP_DEFS_H
#include <net/ppp_defs.h>
#else
#include "ppp_defs.h"
#endif
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
  close(((struct solaris_fpppd_drvdata*)dev->drvdata)->fd);
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
    if(sscanf(devn, "ppp%d", &unit) != 1)
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

  /* clear vars (not clearing will fuck up all scales when disconnecting) */
  *ip = *op = *ib = *ob = 0;

  if(drvdata->mustconnect < sfpppd_connected)
  {
    /* trying to activate the device */
    if(solaris_fpppd_init(dev))
      return 1;
  }
  if(solaris_fpppd_strioctl(drvdata->fd, PPPIO_GETSTAT,
	 (char*)&curp, 0, sizeof(curp)) < 0)
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
  if(!drvdata->active)
    msg_drInfo(drName, "device %s can't properly detect link status", dev->name);
  drvdata->in_pkt = (kstat_named_t*)kstat_data_lookup(drvdata->ksp,"ipackets");
  drvdata->in_byte = (kstat_named_t*)kstat_data_lookup(drvdata->ksp,"rbytes");
  drvdata->out_pkt = (kstat_named_t*)kstat_data_lookup(drvdata->ksp,"opackets");
  drvdata->out_byte = (kstat_named_t*)kstat_data_lookup(drvdata->ksp,"obytes");

  return !(drvdata->in_pkt && drvdata->in_byte &&
      drvdata->out_pkt && drvdata->out_byte);
}

int
solaris_kstat_get(struct Devices* dev, unsigned long* ip, unsigned long* op,
    unsigned long* ib, unsigned long* ob)
{
  struct solaris_kstat_drvdata* drvdata =
    (struct solaris_kstat_drvdata*)dev->drvdata;
  *ip = *op = *ib = *ob = 0;

  /* a bit ugly, but use the correct type on either 32 or 64bit builds
     without having to check data_type dynamically or truncate. */
#if (SIZEOF_UNSIGNED_LONG < 8)
#define KSTAT_WORD_TYPE ui32
#else
#define KSTAT_WORD_TYPE ui64
#endif

  if(kstat_read(solaris_kstat_kc, drvdata->ksp, NULL) == -1)
    return 1;
  else
  {
    *ip = (drvdata->in_pkt->value.KSTAT_WORD_TYPE);
    *op = (drvdata->out_pkt->value.KSTAT_WORD_TYPE);
    *ib = (drvdata->in_byte->value.KSTAT_WORD_TYPE);
    *ob = (drvdata->out_byte->value.KSTAT_WORD_TYPE);
  }

  return (drvdata->active? !drvdata->active->value.KSTAT_WORD_TYPE: 0);
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

#include <math.h>

/* we need the double of PI, not defined everywhere */
#undef M_2PI
#define M_2PI (M_PI * 2)

int
testing_dummy_list(const char* devname, struct Devices* list)
{
  struct Devices* ndev;
  if(devname)
    return 0;

  ndev = (struct Devices*)malloc(sizeof(struct Devices));
  ndev->name = strdup("off");
  devices_append(list, ndev);

  msg_drInfo(drName, "detected %s", ndev->name);

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
  /* some more fun when debugging! */
  static float v = 0.;
  static unsigned long is = 0;
  static unsigned long os = 0;

  *ip = *ib = (is += (16384. * cos(v)) + 16384);
  *op = *ob = (os += (16384. * sin(v)) + 16384);
  if(v >= M_2PI) v = fmodf(v, M_2PI);
  v += 0.05;

  return 1;
}

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
const char* linux_proc_tokens = " :\t\n"; /* parse tokens */

int
linux_proc_list(const char* devname, struct Devices* list)
{
  FILE* fd;
  int dta = 0;
  char temp[MAXBUF]; /* string buffer */
  struct Devices* ndev;
  char* p;

  /* device name was specified */
  if(devname)
  {
    strncpy(temp, devname, sizeof(temp));
    for(p = strtok(temp, ","); p; p = strtok(NULL, ","))
    {
      ndev = (struct Devices*)malloc(sizeof(struct Devices));
      ndev->devstart = 0;
      ndev->name = strdup(p);

      ++dta;
      list = devices_append(list, ndev);

      msg_drInfo(drName, "forced %s", p);
    }
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
      p = strtok(temp, linux_proc_tokens);
      if(!strncmp(p, "dummy", 5) ||
	  !strncmp(p, "irda", 4) || !strncmp(p, "lo", 3))
	continue;

      dta++;
      ndev = (struct Devices*)malloc(sizeof(struct Devices));
      ndev->devstart = 0;
      ndev->name = strdup(p);
      list = devices_append(list, ndev);

      msg_drInfo(drName, "detected %s", p);
    }
    fclose(fd);
  }

  return dta;
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
  if(!fp) return 1;

  fgets(temp, MAXBUF, fp);
  fgets(temp, MAXBUF, fp);

  *ib = *ob = *ip = *op = 0;

  while(fgets(temp, MAXBUF, fp))
  {
    if(!strcmp(strtok(temp, linux_proc_tokens), dev->name))
    {
      p = strchr(temp, 0) + 1;
      sscanf(p, "%lu %lu %*s %*s %*s %*s %*s %*s %lu %lu", ib, ip, ob, op);
      active = 0;
      break;
    }
  }
  fclose(fp);

  return active;
}

#endif /* USE_LINUX_PROC */


/* FreeBSD sysctl driver */
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
  int i;
  size_t len, len2;

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

/* gather stats */
int
freebsd_sysctl_get(struct Devices*dev, unsigned long* ip,
    unsigned long* op, unsigned long* ib, unsigned long* ob)
{
  struct freebsd_sysctl_drvdata* drdata = dev->drvdata;
  int datamib[6];
  size_t len;

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
  pmID pmId[4];
  int inst;
  int ph;
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

  /* fetch the instance list from any of the four domains */
  if((dta = pmGetInDom(pmD->indom, &inst, &desc)) < 0)
  {
    msg_drInfo(drName, "unable to get instances of " PCPNS_NETINBDOM
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
      ndev = (struct Devices*)malloc(sizeof(struct Devices));
      ndev->name = strdup(*p);
      drdata = (struct irix_pcp_drvdata*)malloc(
	  sizeof(struct irix_pcp_drvdata));
      memcpy(drdata->pmId, pmId, sizeof(pmId));
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
  struct irix_pcp_drvdata* drdata =
    (struct irix_pcp_drvdata*)dev->drvdata;

  /* reset the device timestamp, can't know when a device
   * goes offline with PCP */
  dev->devstart = 0;

  /* initialize a profile for this device */
  pmDelProfile(PM_INDOM_NULL, 0, NULL);
  drdata->ph = pmDupContext();
  pmAddProfile(PM_INDOM_NULL, 1, &drdata->inst);

  return 0;
}

int
irix_pcp_get(struct Devices* dev, unsigned long* ip, unsigned long* op,
    unsigned long* ib, unsigned long* ob)
{
  /* switch to the right context */
  struct irix_pcp_drvdata* drdata =
    (struct irix_pcp_drvdata*)dev->drvdata;
  pmResult* pmR;

  pmUseContext(drdata->ph);
  if(pmFetch(4, drdata->pmId, &pmR))
    return 1;

  *ib = pmR->vset[0]->vlist->value.lval;
  *ip = pmR->vset[1]->vlist->value.lval;
  *ob = pmR->vset[2]->vlist->value.lval;
  *op = pmR->vset[3]->vlist->value.lval;

  pmFreeResult(pmR);
  return 0;
}

void
irix_pcp_term(struct Devices* dev)
{
  /* free the device profile */
  struct irix_pcp_drvdata* drdata =
    (struct irix_pcp_drvdata*)dev->drvdata;

  pmDestroyContext(drdata->ph);
  free(dev->drvdata);
}

#endif /* USE_IRIX_PCP */


/* Generic SNMP driver for net-snmp >= 5 */
#ifdef USE_GENERIC_SNMP
#undef drName
#define drName USE_GENERIC_SNMP

/* NET-SNMP API headers */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <string.h>

/* SNMP MIB Definitions (from IF-MIB) and constants */
#define SNMPMIB_HOST "localhost"
#define SNMPMIB_COMN "public"
#define SNMPMIB_NUM "ifNumber.0"
#define SNMPMIB_STATUS "ifOperStatus"
#define SNMPMIB_CHANGE "ifLastChange"
#define SNMPMIB_IFINBDEF "ifInOctets"
#define SNMPMIB_IFINPDEF "ifInUcastPkts"
#define SNMPMIB_IFOUTBDEF "ifOutOctets"
#define SNMPMIB_IFOUTPDEF "ifOutUcastPkts"

struct generic_snmp_drvdata
{
  struct snmp_session* se;
  int num;
  char* operStatName;
  oid oidOperStat[MAX_OID_LEN];
  size_t oidOperStatLen;
  char* bInName;
  oid oidBIn[MAX_OID_LEN];
  size_t oidBInLen;
  char* pInName;
  oid oidPIn[MAX_OID_LEN];
  size_t oidPInLen;
  char* bOutName;
  oid oidBOut[MAX_OID_LEN];
  size_t oidBOutLen;
  char* pOutName;
  oid oidPOut[MAX_OID_LEN];
  size_t oidPOutLen;
};

/*
 * Parse a specification string [community@]host[:interface].
 * User is responsible to free community, host and interface if != NULL
 * Returns true on parsing errors?
 */
int
generic_snmp_pss(const char* str, char** community,
    char** host, int* interface)
{
  char* pos;
  size_t len;

  if(!str)
  {
    *host = strdup(SNMPMIB_HOST);
    *community = strdup(SNMPMIB_COMN);
    *interface = 0;

    return 0;
  }

  /* community specification */
  if((pos = strchr(str, '@')))
  {
    len = pos - str;
    *community = (char*)malloc(len + 1);
    memcpy(*community, str, len);
    (*community)[len] = 0;
    str = pos + 1;
  }
  else
    *community = strdup(SNMPMIB_COMN);

  /* hostname */
  if(!(pos = strchr(str, ':')))
    pos = (char*)str + strlen(str);
  len = pos - str;
  *host = (char*)malloc(len + 1);
  memcpy(*host, str, len);
  (*host)[len] = 0;

  /* interface */
  if(*pos)
    *interface = atoi(pos + 1);
  else
    *interface = 0;

  return 0;
}

/* Create a new session and connect to an host */
struct snmp_session*
generic_snmp_session(const char* community, const char* host)
{
  struct snmp_session params;
  struct snmp_session* se;

  snmp_sess_init(&params);
  params.version = SNMP_VERSION_1;
  params.peername = strdup(host);
  params.community = (u_char*)strdup(community);
  params.community_len = strlen(community);

  /* try connecting to host */
  SOCK_STARTUP;
  se = snmp_open(&params);
  if(!se)
  {
    msg_drInfo(drName, "unable to communicate to %s@%s", community, host);
    return NULL;
  }

  free(params.peername);
  free(params.community);

  return se;
}

char*
generic_snmp_comp(const char* name, const int num)
{
  char* buf = (char*)malloc(strlen(name) + 5);
  sprintf(buf, "%s.%d", name, num);
  return buf;
}

char*
generic_snmp_getNodeDesc(struct snmp_session* se, const char* node, int dev)
{
  struct snmp_pdu* pdu;
  struct snmp_pdu* res;
  oid OID[MAX_OID_LEN];
  size_t OID_len = MAX_OID_LEN;
  char* name = generic_snmp_comp(node, dev);

  pdu = snmp_pdu_create(SNMP_MSG_GET);
  get_node(name, OID, &OID_len);
  free(name);
  snmp_add_null_var(pdu, OID, OID_len);
  if(snmp_synch_response(se, pdu, &res) == STAT_SUCCESS &&
      res->errstat == SNMP_ERR_NOERROR)
  {
    name = (char*)malloc(res->variables->val_len + 1);
    memcpy(name, res->variables->val.string, res->variables->val_len);
    name[res->variables->val_len] = 0;

    snmp_free_pdu(res);
  }
  else
    return NULL;

  return name;
}

char*
generic_snmp_getDesc(struct snmp_session* se, int dev)
{
  char* desc;

  return ((desc = generic_snmp_getNodeDesc(se, "ifName", dev))? desc:
      generic_snmp_getNodeDesc(se, "ifDescr", dev));
}

struct Devices*
generic_snmp_preInit(struct Devices* list, struct snmp_session* se, int dev)
{
  struct Devices* ndev;
  struct generic_snmp_drvdata* drdata;

  ndev = (struct Devices*)malloc(sizeof(struct Devices));
  if(!(ndev->name = generic_snmp_getDesc(se, dev)))
    return NULL;

  msg_drInfo(drName, "detected %s(%d)", ndev->name, dev);
  drdata =(struct generic_snmp_drvdata*)
    malloc(sizeof(struct generic_snmp_drvdata));
  drdata->num = dev;
  drdata->se = se;
  ndev->drvdata = (void*)drdata;

  return devices_append(list, ndev);
}

int
generic_snmp_list(const char* devname, struct Devices* list)
{
  char* com;
  char* host;
  int interf;
  struct snmp_session* se;
  int rad;
  int dta;
  struct Devices* ndev;
  oid OID[MAX_OID_LEN];
  size_t OIDLen = MAX_OID_LEN;
  struct snmp_pdu* pdu;
  struct snmp_pdu* res;

  /* initialize the snmp library */
  init_snmp(msg_prgName);

  /* parse the device name */
  generic_snmp_pss(devname, &com, &host, &interf);
  se = generic_snmp_session(com, host);
  if(!se)
    return 0;

  if(interf)
  {
    /* the inferface is specified, build a struture and return immediately */
    ndev = generic_snmp_preInit(list, se, interf);
    if(ndev)
      rad = 1;
    else
    {
      msg_drInfo(drName, "unable to resolve %d", interf);
      snmp_close(se);
      rad = 0;
    }
  }
  else
  {
    /* query for number of interfaces */
    get_node(SNMPMIB_NUM, OID, &OIDLen);
    pdu = snmp_pdu_create(SNMP_MSG_GET);
    snmp_add_null_var(pdu, OID, OIDLen);

    if(snmp_synch_response(se, pdu, &res) == STAT_SUCCESS &&
	res->errstat == SNMP_ERR_NOERROR)
      dta = *res->variables->val.integer;
    else
    {
      dta = 0;
      msg_drInfo(drName, "unable to determine the number of interfaces");
    }

    /* close the temporary session */
    snmp_close(se);

    /* intialize devices */
    for(rad = 0; rad != dta;)
    {
      se = generic_snmp_session(com, host);
      ndev = generic_snmp_preInit(list, se, rad + 1);
      if(!ndev)
      {
	snmp_close(se);
	continue;
      }

      list = ndev;
      ++rad;
    }
  }

  if(com)
    free(com);
  if(host)
    free(host);

  return rad;
}

int
generic_snmp_init(struct Devices* dev)
{
  struct generic_snmp_drvdata* drdata =
    (struct generic_snmp_drvdata*)dev->drvdata;

  /* TODO: SNMP 'should' be able to tell the last change through ifLastChange */
  dev->devstart = 0;

  drdata->operStatName = generic_snmp_comp(SNMPMIB_STATUS, drdata->num);
  drdata->oidOperStatLen = MAX_OID_LEN;
  get_node(drdata->operStatName, drdata->oidOperStat, &drdata->oidOperStatLen);
  drdata->bInName = generic_snmp_comp(SNMPMIB_IFINBDEF, drdata->num);
  drdata->oidBInLen = MAX_OID_LEN;
  get_node(drdata->bInName, drdata->oidBIn, &drdata->oidBInLen);
  drdata->pInName = generic_snmp_comp(SNMPMIB_IFINPDEF, drdata->num);
  drdata->oidPInLen = MAX_OID_LEN;
  get_node(drdata->pInName, drdata->oidPIn, &drdata->oidPInLen);
  drdata->bOutName = generic_snmp_comp(SNMPMIB_IFOUTBDEF, drdata->num);
  drdata->oidBOutLen = MAX_OID_LEN;
  get_node(drdata->bOutName, drdata->oidBOut, &drdata->oidBOutLen);
  drdata->pOutName = generic_snmp_comp(SNMPMIB_IFOUTPDEF, drdata->num);
  drdata->oidPOutLen = MAX_OID_LEN;
  get_node(drdata->pOutName, drdata->oidPOut, &drdata->oidPOutLen);

  return 0;
}

int
generic_snmp_get(struct Devices* dev, unsigned long* ip, unsigned long* op,
    unsigned long* ib, unsigned long* ob)
{
  /* switch to the right context */
  struct generic_snmp_drvdata* drdata =
    (struct generic_snmp_drvdata*)dev->drvdata;

  struct snmp_pdu* pdu;
  struct snmp_pdu* res;
  struct variable_list* var;
  int stat;

  pdu = snmp_pdu_create(SNMP_MSG_GET);
  snmp_add_null_var(pdu, drdata->oidOperStat, drdata->oidOperStatLen);
  snmp_add_null_var(pdu, drdata->oidBIn, drdata->oidBInLen);
  snmp_add_null_var(pdu, drdata->oidPIn, drdata->oidPInLen);
  snmp_add_null_var(pdu, drdata->oidBOut, drdata->oidBOutLen);
  snmp_add_null_var(pdu, drdata->oidPOut, drdata->oidPOutLen);

  if(snmp_synch_response(drdata->se, pdu, &res) == STAT_SUCCESS &&
      res->errstat == SNMP_ERR_NOERROR)
  {
    var = res->variables;
    stat = (*var->val.integer != 1);
    var = var->next_variable;
    *ib = *var->val.integer;
    var = var->next_variable;
    *ip = *var->val.integer;
    var = var->next_variable;
    *ob = *var->val.integer;
    var = var->next_variable;
    *op = *var->val.integer;
    snmp_free_pdu(res);
  }
  else
    stat = 1;

  return stat;
}

void
generic_snmp_term(struct Devices* dev)
{
  /* free the device profile */
  struct generic_snmp_drvdata* drdata =
    (struct generic_snmp_drvdata*)dev->drvdata;

  snmp_close(drdata->se);
  free(drdata->operStatName);
  free(drdata->bInName);
  free(drdata->pInName);
  free(drdata->bOutName);
  free(drdata->pOutName);
  free(dev->drvdata);
}

#endif /* USE_GENERIC_SNMP */

/* NetBSD ioctl driver */
#ifdef USE_NETBSD_IOCTL
#undef drName
#define drName USE_NETBSD_IOCTL

/* system headers */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>

int s;
struct ifdatareq ifdreq;

/* device listing */
int
netbsd_ioctl_list(const char *devname, struct Devices *list)
{
  struct Devices *ndev;
  struct ifaddrs *ifap, *ifa;
  unsigned int ifn;

  if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    msg_drInfo(drName, "unable to create socket");
    return 0;
  }

  if(getifaddrs(&ifap) != 0)
  {
    msg_drInfo(drName, "failed to perform getifaddrs");
    return 0;
  }

  ifn = 0;
  for(ifa = ifap; ifa; ifa = ifa->ifa_next)
  {
    if(ifa->ifa_addr->sa_family == AF_LINK)
    {
      if((devname && !strcmp(devname, ifa->ifa_name)) || !devname)
      {
	ndev = NULL;
	ndev = malloc(sizeof(struct Devices));
	if(!ndev)
	  continue;
	ndev->devstart = 0;
	ndev->name = strdup(ifa->ifa_name);
	ndev->next = NULL;
	list->next = ndev;
	list = ndev;

	msg_drInfo(drName, "detected %s", ndev->name);
	ifn++;
      }
    }
  }
  freeifaddrs(ifap);

  return ifn;
}

/* gather stats */
int
netbsd_ioctl_get(struct Devices *dev, unsigned long *ip,
    unsigned long *op, unsigned long *ib, unsigned long *ob)
{
  strncpy(ifdreq.ifdr_name, dev->name, IFNAMSIZ - 1);
  if(ioctl(s, SIOCGIFDATA, &ifdreq) < 0)
    return 1;

  *ip = ifdreq.ifdr_data.ifi_ipackets;
  *ib = ifdreq.ifdr_data.ifi_ibytes;
  *op = ifdreq.ifdr_data.ifi_opackets;
  *ob = ifdreq.ifdr_data.ifi_obytes;

  return 0;
}

void
netbsd_ioctl_unlist()
{
  close(s);
}

#endif /* USE_NETBSD_IOCTL */

/* define the drivers list */
struct drivers_struct drivers_table[] =
{
#ifdef USE_FREEBSD_SYSCTL
  {USE_FREEBSD_SYSCTL, freebsd_sysctl_list, NULL,
    freebsd_sysctl_get, freebsd_sysctl_term, NULL},
#endif
#ifdef USE_NETBSD_IOCTL
  {USE_NETBSD_IOCTL, netbsd_ioctl_list, NULL,
    netbsd_ioctl_get, NULL, netbsd_ioctl_unlist},
#endif
#ifdef USE_LINUX_PROC
  {USE_LINUX_PROC, linux_proc_list, NULL,
    linux_proc_get, NULL, NULL},
#endif
#ifdef USE_SOLARIS_FPPPD
  {USE_SOLARIS_FPPPD, solaris_fpppd_list, solaris_fpppd_init,
    solaris_fpppd_get, solaris_fpppd_term, NULL},
#endif
#ifdef USE_SOLARIS_KSTAT
  {USE_SOLARIS_KSTAT, solaris_kstat_list, solaris_kstat_init,
    solaris_kstat_get, solaris_kstat_term, NULL},
#endif
#ifdef USE_IRIX_PCP
  {USE_IRIX_PCP, irix_pcp_list, irix_pcp_init,
    irix_pcp_get, irix_pcp_term, NULL},
#endif
#ifdef USE_GENERIC_SNMP
  {USE_GENERIC_SNMP, generic_snmp_list, generic_snmp_init,
    generic_snmp_get, generic_snmp_term, NULL},
#endif
#ifdef USE_TESTING_DUMMY
  {USE_TESTING_DUMMY, testing_dummy_list, testing_dummy_init,
    testing_dummy_get, NULL, NULL},
#endif
  {NULL, NULL, NULL, NULL, NULL, NULL}
};
