/*
 * wmnd.c
 * by Reed Lai
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * $Id$
 */

#include "wmnd.h"
#include "cfgdata.h"
#include "display.h"
#include "misc.h"
#include "master.xpm"
#include "drivers.h"
#include "messages.h"

struct var *vars;        /* config file option structure */
struct Devices *devices; /* devices to monitor */
DevTable wmnd;           /* interesting information about devices */
Dockapp dockapp;         /* dockapp structure */
MRegion mr[32];          /* mouse regions */

/* GUI */
static void redraw_window(void);
unsigned long get_color(char *name);
void new_window(char *name, int width, int height, int argc, char** argv);

/* config file read/write */
void conf_read(char *filename);
void conf_write(char *filename);
void assign(char *name, char *value);
void vdel(char *name);
struct var *lookup(char *name);
char *value(char *name);
char *vcopy(char *str);

/* utils for command line */
int strval_fe(const struct pair_strint *data, const char *val);
int defcon_lk(const char *token);
void defcon_touch(char *token, char *val);

/* support */
int add_mr(int index, int x, int y, int width, int height);
int check_mr(int x, int y);
void beat_event(void);
void click_event(unsigned int region, unsigned int button);
static void led_control(const unsigned char led, const unsigned char mode);
void scale(char *rx_buf, char *tx_buf, unsigned long rx, unsigned long tx);
void metric_scale(unsigned char sign, unsigned long value, char *buf);
void binary_scale(unsigned char sign, unsigned long value, char *buf);
void draw_string(const char *buf, unsigned int x, unsigned int y);
void smooth(unsigned long* stat, const unsigned long last, const float smooth);

/* device statistics */
void draw_interface(void);
void draw_rate(unsigned long rx, unsigned long tx);
void draw_max(unsigned long rx, unsigned long tx);
void draw_stats(struct Devices *ptr);

/* driver functions */
int devices_init(const char *driver, const char *interface);
void devices_select(const char *interface);
void devices_getstat(struct Devices *device, unsigned long *ip,
    unsigned long *op, unsigned long *ib, unsigned long *ob);
void devices_destroy();
void devices_restart(int sig);

/* useless shit */
void usage(void);
void printversion(void);

int
add_mr(int index, int x, int y, int width, int height)
{
  mr[index].enable = 1;
  mr[index].x = x;
  mr[index].y = y;
  mr[index].width = width;
  mr[index].height = height;
  return 0;
}

int
check_mr(int x, int y)
{
  register int i;
  register int found = 0;

  for(i = 0; i < 32 && !found; i++)
  {
    if(mr[i].enable && x >= mr[i].x &&
        x <= mr[i].x + mr[i].width &&
        y >= mr[i].y && y <= mr[i].y + mr[i].height)
      found = 1;
  }
  if(!found)
    return -1;
  return (i - 1);
}

static void
redraw_window(void)
{
  if(dockapp.update)
  {
    msg_dbg(__POSITION__, "redrawing window");
    XCopyArea(dockapp.d, dockapp.pixmap, dockapp.iconwin, dockapp.gc,
        0, 0, dockapp.width, dockapp.height, 0, 0);
    XCopyArea(dockapp.d, dockapp.pixmap, dockapp.win, dockapp.gc,
        0, 0, dockapp.width, dockapp.height, 0, 0);
    dockapp.update = 0;
  }
}

unsigned long
get_color(char *name)
{
  XColor color;
  XWindowAttributes attr;

  color.pixel = 0;

  XGetWindowAttributes(dockapp.d, DefaultRootWindow(dockapp.d), &attr);
  XParseColor(dockapp.d, DefaultColormap(dockapp.d,
      DefaultScreen(dockapp.d)), name, &color);
  color.flags = DoRed | DoGreen | DoBlue;
  XAllocColor(dockapp.d, attr.colormap, &color);
  msg_dbg(__POSITION__, "pixel: %08lx", color.pixel);
  return color.pixel;
}

void
new_window(char *name, int width, int height, int argc, char** argv)
{
  XpmAttributes attr;
  XpmColorSymbol cols[3] =
  {
    {"rx_color", NULL, 0},
    {"tx_color", NULL, 0},
    {"md_color", NULL, 0}
  };
  Pixel fg, bg;
  XGCValues gcval;
  XSizeHints sizehints;
  XClassHint classhint;
  XWMHints wmhints;

  dockapp.width = width;
  dockapp.height = height;
  dockapp.screen = DefaultScreen(dockapp.d);
  dockapp.root = DefaultRootWindow(dockapp.d);

  sizehints.flags = USSize | USPosition;
  sizehints.x = dockapp.x ? dockapp.x : 0;
  sizehints.y = dockapp.y ? dockapp.y : 0;
  sizehints.width = width;
  sizehints.height = height;

  fg = BlackPixel(dockapp.d, dockapp.screen);
  bg = WhitePixel(dockapp.d, dockapp.screen);

  dockapp.win = XCreateSimpleWindow(dockapp.d, dockapp.root,
      sizehints.x, sizehints.y, sizehints.width, sizehints.height, 1, fg, bg);
  dockapp.iconwin = XCreateSimpleWindow(dockapp.d, dockapp.win,
      sizehints.x, sizehints.y, sizehints.width, sizehints.height, 1, fg, bg);

  XSetWMNormalHints(dockapp.d, dockapp.win, &sizehints);
  classhint.res_name = name;
  classhint.res_class = name;
  XSetClassHint(dockapp.d, dockapp.win, &classhint);

  XSelectInput(dockapp.d, dockapp.win,
      ExposureMask | ButtonPressMask | ButtonReleaseMask |
      StructureNotifyMask);
  XSelectInput(dockapp.d, dockapp.iconwin,
      ExposureMask | ButtonPressMask | ButtonReleaseMask |
      StructureNotifyMask);

  XStoreName(dockapp.d, dockapp.win, name);
  XSetIconName(dockapp.d, dockapp.win, name);

  gcval.foreground = fg;
  gcval.background = bg;
  gcval.graphics_exposures = False;

  dockapp.gc = XCreateGC(dockapp.d, dockapp.win,
      GCForeground | GCBackground | GCGraphicsExposures, &gcval);

  cols[0].pixel = get_color(value("rx_color"));
  cols[1].pixel = get_color(value("tx_color"));
  cols[2].pixel = get_color(value("md_color"));
  dockapp.stdColors.txColor = cols[1].pixel;
  dockapp.stdColors.rxColor = cols[0].pixel;
  dockapp.stdColors.mdColor = cols[2].pixel;

  attr.exactColors = 0;
  attr.alloc_close_colors = 1;
  attr.closeness = 1L << 15;
  attr.colorsymbols = cols;
  attr.numsymbols = 3;
  attr.valuemask =
    (XpmColorSymbols | XpmExactColors | XpmAllocCloseColors | XpmCloseness);
  if(XpmCreatePixmapFromData
      (dockapp.d, dockapp.win, master_xpm, &dockapp.pixmap,
       &dockapp.mask, &attr) != XpmSuccess)
  {
    msg_err("not enough colors!");
    exit(-1);
  }
  XShapeCombineMask(dockapp.d, dockapp.win, ShapeBounding, 0, 0,
      dockapp.mask, ShapeSet);
  XShapeCombineMask(dockapp.d, dockapp.iconwin, ShapeBounding, 0, 0,
      dockapp.mask, ShapeSet);

  wmhints.initial_state = WithdrawnState;
  wmhints.flags = StateHint;
  wmhints.icon_window = dockapp.iconwin;
  wmhints.icon_x = sizehints.x;
  wmhints.icon_y = sizehints.y;
  wmhints.window_group = dockapp.win;
  wmhints.flags =
    StateHint | IconWindowHint | IconPositionHint | WindowGroupHint;
  XSetWMHints(dockapp.d, dockapp.win, &wmhints);

  XSetCommand(dockapp.d, dockapp.win, argv, argc);
  XSetCommand(dockapp.d, dockapp.iconwin, argv, argc);

  XMapWindow(dockapp.d, dockapp.win);
  dockapp.xfd = ConnectionNumber(dockapp.d);
}

/* support function - chop cr/lf off the end of a string */
void
chomp(char *buffer)
{
  int i = strlen(buffer) - 1;
  while (buffer[i] == '\n' || buffer[i] == '\r') {
    buffer[i--] = '\0';
  }
}

void
conf_read(char *filename)
{
  FILE *fp;
  char buf[1024];
  size_t pos;
  int line = 0;
  int exist;

  fp = fopen(filename, "r");

  if(fp)
  {
    /* actually read in the config file, skipping over short and #lines */
    while (fgets(buf, 1024, fp)) {
      line++;
      if(buf[0] == '#' || (strlen(buf) < 2))
        continue;
      chomp(buf);
      pos = strcspn(buf, "=");
      if(pos < strlen(buf))
      {
        buf[pos++] = '\0';

        /* check for existent value */
        exist = defcon_lk(buf); /* search for buf in defaults */
        if(exist != -1 && !pss_defcon[exist].used)
          exist = -1;
        if(exist == -1) /* only if isn't a default or it's not on cmdline */
          assign(buf, buf+pos);
      }
      memset(buf, 0, 1024);
    }
    fclose(fp);
  }
  else
  {
    /* can't open the rc file, make a new one */
    msg_err("can't open WMND config file '%s', using defaults", filename);
    conf_write(filename);
  }
}

void
conf_write(char *filename)
{
  FILE *fp;
  struct var *vp;

  fp = fopen(filename, "w");
  if(!fp)
  {
    msg_err("can't open '%s' for writing", filename);
    exit(1);
  }
  fprintf(fp, "# WMND configuration file (generated automatically)\n\n");
  for(vp = vars; vp; vp = vp->v_next)
    fprintf(fp, "%s=%s\n", vp->v_name, vp->v_value);
  fclose(fp);
}

void
assign(char *name, char *value)
{
  struct var *vp;
  struct var *newv;

  newv = (struct var *) malloc(sizeof(struct var));

  vdel(name);

  if(vars == NULL || strcmp(vars->v_name, name) >= 0)
  {
    newv->v_next = vars;
    vars = newv;
  }
  else
  {
    vp = vars;
    while (vp->v_next && strcmp(vp->v_next->v_name, name) <= 0)
      vp = vp->v_next;
    newv->v_next = vp->v_next;
    vp->v_next = newv;
  }
  newv->v_name = vcopy(name);
  newv->v_value = vcopy(value);
}

void
vdel(char *name)
{
  struct var *vp;
  struct var *newv;

  while (vars && !strcmp(vars->v_name, name))
  {
    vp = vars->v_next;
    free((struct var *) vars);
    vars = vp;
  }
  if(vars)
  {
    for(newv = vars; newv->v_next;)
      if(!strcmp(newv->v_next->v_name, name))
      {
        vp = newv->v_next->v_next;
        free(newv->v_next);
        newv->v_next = vp;
      }
      else
        newv = newv->v_next;
  }
}

char*
vcopy(char *str)
{
  char *newv;
  unsigned int len;

  if(str == NULL)
    return "";
  len = strlen(str) + 1;
  newv = (char*)malloc(len);
  memcpy(newv, str, len);
  return newv;
}

int
strval_fe(const struct pair_strint *data, const char *val)
{
  /* returns the correct value for val as int */
  int cnt = 0;
  while (data[cnt].strval)
  {
    if(!strcmp(val,data[cnt].strval))
      return data[cnt].val;
    cnt++;
  }

  /* not found, return 0 */
  return data->val;
}

int
waveval_fe(const struct drwStruct *data, const char *val)
{
  int cnt = 0;
  while (data[cnt].funcName)
  {
    if(!strcmp(val,data[cnt].funcName))
      return cnt;
    cnt++;
  }
  return 0;
}

int
defcon_lk(const char *token)
{
  int rtval = 0;
  while (pss_defcon[rtval].token)
  {
    if(!strcmp(token,pss_defcon[rtval].token))
      return rtval;
    rtval++;
  }
  return -1;
}

void
defcon_touch(char *token, char *val)
{
  int idx = defcon_lk(token);

  assign(token, val);
  if(idx != -1)
    pss_defcon[idx].used = 1;
}

struct var*
lookup(char *name)
{
  struct var *vp;

  for(vp = vars; vp; vp = vp->v_next)
    if(!strcmp(name, vp->v_name))
      return vp;
  return NULL;
}

char*
value(char *name)
{
  struct var *vp;

  if(!(vp = lookup(name)))
    return NULL;
  return vp->v_value;
}

void
beat_event(void)
{
  unsigned long diff;
  unsigned int min, hr;
  char temp[16];

  msg_dbg(__POSITION__, "activated");
  if(!wmnd.curdev->online)
  {
    if(!bit_get(RUN_ONLINE))
    {
      bit_set(RUN_ONLINE);
      led_control(LED_POWER, 1);
    }
  }
  else
  {
    if(bit_get(RUN_ONLINE))
    {
      bit_off(RUN_ONLINE);
      led_control(LED_POWER, 0);
    }
  }
  if(bit_get(CFG_SHOWTIME) && wmnd.curdev->devstart)
  {
    diff = (long unsigned)(difftime(time(NULL), wmnd.curdev->devstart) / 60);
    min = diff % 60;
    hr = (diff / 60) % 100;
    sprintf(temp, "%02d.%02d", hr, min);
    draw_string(temp, 71, 36);
  }
}

void
click_event(unsigned int region, unsigned int button)
{
  char *action = NULL;

  if(region == -1u)		/* no region */
    return;

  msg_dbg(__POSITION__, "clicked btn %d in region %d", button, region);

  if(region == 0)
  {
    switch (button)
    {
    case Button1:
      devices_select(NULL);
      draw_interface();
      break;
    case Button3:
      bit_tgl(CFG_SHORTNAME);
      msg_dbg(__POSITION__, "shortname: %d", bit_get(CFG_SHORTNAME));
      draw_interface();
      break;
    }
  }
  else
  if(region == 1)
  {
    if(button == Button1)
    {
      bit_tgl(CFG_MODE);
      led_control(LED_POWER, bit_get(RUN_ONLINE));
    }
  }
  else
  if(region == 2)
  {
    if(button == Button1)
    {
      if(wmnd.wavemode < (wmnd.nWavemodes - 1))
        wmnd.wavemode++;
      else
        wmnd.wavemode = 0;
      msg_dbg(__POSITION__, "wavemode: %d", wmnd.wavemode);
    }
    else if(button == Button3)
      /* switch time visualization */
      bit_tgl(CFG_SHOWTIME);
  }
  else
  if(region == 3)
  {
    switch (button)
    {
    case Button1:
      /* switch max screen/history */
      bit_tgl(CFG_MAXSCREEN);
      break;
    case Button3:
      /* toggle max display */
      bit_tgl(CFG_SHOWMAX);
      break;
    }
  }
  else
  if(region == 4)
  {
    switch (button)
    {
    case Button1:
      action = value("bt1_action");
      break;
    case Button2:
      action = value("bt2_action");
      break;
    case Button3:
      action = value("bt3_action");
      break;
    }
    if(action)
      execCommand(action);
  }
}

void
smooth(unsigned long* stat, const unsigned long last, const float smooth)
{
  *stat = last + smooth * (*stat - last);
}

void
mainExit(int sig)
{
  msg_info("caught signal %d, terminating...", sig);
  devices_destroy();
  exit(1);
}

int main(int argc, char **argv)
{
  char *dispname = NULL;
  char *active_interface = NULL;
  char *drv_interface = NULL;
  char *drv_name = NULL;
  int parse_conf = 1;
  char *conf_file = NULL;
  struct Devices *ptr;
  unsigned long int ib, ob, ip, op;
  int ch;
  unsigned btn = 0;
  int rgn = -1;
  XEvent event;
  const struct drwStruct* drwPtr;
  int beats;

  /* initialize messaging functions */
  msg_prgName = argv[0];

  msg_dbg(__POSITION__, "wmnd start");

  // detect the number of avaible wave modes
  for(drwPtr = drwFuncs; drwPtr->funcName; ++drwPtr)
    ++wmnd.nWavemodes;
  msg_dbg(__POSITION__, "detected %d display modes", wmnd.nWavemodes);

  /*
   * set default config options before option parsing so command line can
   * overwrite config options
   */
  wmnd.nr_devices = 0;
  wmnd.flags = 0;
  bit_set(RUN_ONLINE);
  bit_set(CFG_MODE);

  /*
   * initalize default values using pss_defcon
   * we can use ch, because it's used later
   */
  ch = 0;
  while (pss_defcon[ch].token)
  {
    assign(pss_defcon[ch].token,pss_defcon[ch].value);
    ch++;
  }

  /* parse command line */
  while((ch =
      getopt(argc, argv, "bc:C:L:d:i:hlmMf:Fr:s:S:tvw:D:I:qQo:")) != EOF)
  {
    switch(ch)
    {
    case 'b':
      defcon_touch("binary_scale", "yes");
      break;
    case 'c':
      defcon_touch("tx_color", optarg);
      break;
    case 'C':
      defcon_touch("rx_color", optarg);
      break;
    case 'L':
      defcon_touch("md_color", optarg);
      break;
    case 'd':
      dispname = strdup(optarg);
      break;
    case 'i':
      defcon_touch("interface_name", optarg);
      break;
    case 'l':
      defcon_touch("use_long_names", "yes");
      break;
    case 'm':
      defcon_touch("show_max_values", "no");
      break;
    case 'M':
      defcon_touch("use_max_history", "yes");
      break;
    case 'f':
      conf_file = strdup(optarg);
      break;
    case 'F':
      parse_conf = 0;
      break;
    case 'r':
      defcon_touch("refresh", optarg);
      break;
    case 's':
      defcon_touch("scroll", optarg);
      break;
    case 'S':
      defcon_touch("avg_steps", optarg);
      break;
    case 't':
      defcon_touch("display_time", "no");
      break;
    case 'v':
      printversion();
      exit(0);
      break;
    case 'w':
      defcon_touch("wave_mode", optarg);
      break;
    case 'D':
      defcon_touch("driver", optarg);
      break;
    case 'I':
      defcon_touch("driver_interface", optarg);
      break;
    case 'q':
      defcon_touch("quiet", "yes");
      break;
    case 'Q':
      defcon_touch("quiet", "no");
      break;
    case 'o':
      defcon_touch("smooth", optarg);
      break;
    default:
      usage();
      exit(0);
      break;
    }
  }
  if(parse_conf)
  {
    if(!conf_file)
    {
      char *tmp;
      tmp = getenv("HOME");
      conf_file = (char*)calloc(1, strlen(tmp) + 9);
      strcat(conf_file, tmp);
      strcat(conf_file, "/.wmndrc");
    }
    conf_read(conf_file);
  }

  /* set stuff here */
  wmnd.refresh = atoi(value("refresh"));
  wmnd.scroll = MAX(atoi(value("scroll")), 1);
  wmnd.avgSteps = MAX(atoi(value("avg_steps")), 1);
  wmnd.smooth = atof(value("smooth"));
  if(strval_fe(psi_bool, value("binary_scale")))
    wmnd.scale = binary_scale;
  else
    wmnd.scale = metric_scale;
  active_interface = strdup(value("interface_name"));
  if(!strcmp(active_interface, "%first"))
  {
    free(active_interface);
    active_interface = NULL;
  }
  if(!strval_fe(psi_bool, value("use_long_names")))
    bit_set(CFG_SHORTNAME);
  if(strval_fe(psi_bool, value("show_max_values")))
    bit_set(CFG_SHOWMAX);
  if(!strval_fe(psi_bool, value("use_max_history")))
    bit_set(CFG_MAXSCREEN);
  if(strval_fe(psi_bool, value("display_time")))
    bit_set(CFG_SHOWTIME);
  wmnd.wavemode = waveval_fe(drwFuncs, value("wave_mode"));
  drv_name = strdup(value("driver"));
  if(!strcmp(drv_name, "%auto"))
  {
    free(drv_name);
    drv_name = NULL;
  }
  drv_interface = strdup(value("driver_interface"));
  if(!strcmp(drv_interface, "%any"))
  {
    free(drv_interface);
    drv_interface = NULL;
  }
  if(strval_fe(psi_bool, value("quiet")))
    msg_messages = MSG_FERR;

  /* check for at least one display mode */
  if(!drwFuncs->funcName)
  {
    msg_err("no avaible display modes, exiting");
    exit(1);
  }

  /* initialize drivers and devices */
  if(!active_interface)
    active_interface = drv_interface;
  if(devices_init(drv_name, drv_interface))
  {
    msg_err("no drivers loaded, exiting");
    exit(1);
  }

  /* now it's safe to connect signals */
  signal(SIGINT, mainExit);
  signal(SIGTERM, mainExit);
  signal(SIGUSR1, devices_restart);

  msg_dbg(__POSITION__, "open X display");
  if(!(dockapp.d = XOpenDisplay(dispname)))
  {
    // fprintf crashes on some systems if dispname is null
    if(dispname)
      msg_err("unable to open display '%s'", dispname);
    else
      msg_err("unable to open default display");

    exit(-1);
  }
  new_window("wmnd", 64, 64, argc, argv);

  add_mr(0, 3, 3, 38, 9);   /* device */
  add_mr(1, 54, 3, 7, 9);   /* up/down packet/byte mode */
  add_mr(2, 3, 22, 58, 31); /* main display area */
  add_mr(3, 3, 13, 58, 9);  /* scale meter */
  add_mr(4, 3, 54, 58, 7);  /* user script? huh wtf does that do */

  /* updates should begin immediately */
  beats = wmnd.scroll * 100000 / wmnd.refresh;

  /* clear the number of remaining steps */
  wmnd.avgRSteps = 1;

  if(!active_interface)	/* set default device name */
    wmnd.curdev = devices;
  else
    devices_select(active_interface);

  draw_interface();

  msg_dbg(__POSITION__, "looping");
  XSync(dockapp.d, False); /* kick off X11 queue */

  /* loop forever */
  for(;;)
  {
    unsigned int j;

    /* get statistics for each existing device */
    for(ptr = devices; ptr; ptr = ptr->next)
    {
      devices_getstat(ptr, &ip, &op, &ib, &ob);

      /* check for device shutdown */
      if(ib < ptr->ib_stat_last)
        ptr->ib_stat_last = ib;
      if(ob < ptr->ob_stat_last)
        ptr->ob_stat_last = ob;
      if(ip < ptr->ip_stat_last)
        ptr->ip_stat_last = ip;
      if(op < ptr->op_stat_last)
        ptr->op_stat_last = op;

      /*
       * smoothing is performed before led setup, in order to let
       * them blink even when the graph is smoothed. this approach is,
       * however, still unperfect: we compare smoothed actual values 
       * where we should store real values for later use.
       */
      if(wmnd.smooth)
      {
        smooth(&ib, ptr->ib_stat_last, wmnd.smooth);
        smooth(&ob, ptr->ob_stat_last, wmnd.smooth);
        smooth(&ip, ptr->ip_stat_last, wmnd.smooth);
        smooth(&op, ptr->op_stat_last, wmnd.smooth);
      }

      /* setup leds */
      if(ptr == wmnd.curdev)
      {
        if(ptr->ib_stat_last == ib)
          led_control(LED_RX, 0);
        else
          led_control(LED_RX, 1);

        if(ptr->ob_stat_last == ob)
          led_control(LED_TX, 0);
        else
          led_control(LED_TX, 1);
      }

      /* save values in history */
      ptr->his[57][0] += ib - ptr->ib_stat_last;
      ptr->his[57][1] += ob - ptr->ob_stat_last;
      ptr->his[57][2] += ip - ptr->ip_stat_last;
      ptr->his[57][3] += op - ptr->op_stat_last;
      ptr->ib_max_his = MAX(ptr->ib_max_his, ptr->his[57][0]);
      ptr->ob_max_his = MAX(ptr->ob_max_his, ptr->his[57][1]);
      ptr->ip_max_his = MAX(ptr->ip_max_his, ptr->his[57][2]);
      ptr->op_max_his = MAX(ptr->op_max_his, ptr->his[57][3]);

      /* save lastest values */
      ptr->ib_stat_last = ib;
      ptr->ob_stat_last = ob;
      ptr->ip_stat_last = ip;
      ptr->op_stat_last = op;
    }

    /* cosmetic fix: use the number of effectively elapsed beats instead of a
     * time based check to distribute timing discrepancies and rounding
     * errors. This will also save us a syscall */
    if((wmnd.refresh * beats / 100000) >= wmnd.scroll)
    {
      beats = 0;

      /* drift the average stats */
      if(!--wmnd.avgRSteps)
      {
        float div = 1. / wmnd.avgSteps;

        for(ptr = devices; ptr; ptr = ptr->next)
        {
          ptr->avg[0] = div * (ptr->ib_stat_last - ptr->avgBuf[0]);
          ptr->avg[1] = div * (ptr->ob_stat_last - ptr->avgBuf[1]);
          ptr->avg[2] = div * (ptr->ip_stat_last - ptr->avgBuf[2]);
          ptr->avg[3] = div * (ptr->op_stat_last - ptr->avgBuf[3]);
          ptr->avgBuf[0] = ptr->ib_stat_last;
          ptr->avgBuf[1] = ptr->ob_stat_last;
          ptr->avgBuf[2] = ptr->ip_stat_last;
          ptr->avgBuf[3] = ptr->op_stat_last;
        }
        wmnd.avgRSteps = wmnd.avgSteps;
      }

      /* cause a beat_event to parse pending x requests */
      beat_event();

      /* scroll the statistics */
      draw_stats(wmnd.curdev);
      for(ptr = devices; ptr; ptr = ptr->next)
      {
        for(j = 1; j < 58; j++)
        {
          ptr->his[j - 1][0] = ptr->his[j][0];
          ptr->his[j - 1][1] = ptr->his[j][1];
          ptr->his[j - 1][2] = ptr->his[j][2];
          ptr->his[j - 1][3] = ptr->his[j][3];
        }
        memset(ptr->his[57], 0, sizeof(ptr->his[57]));
      }
    }

    /*
     * beat_event and redraw added also on expose and button release
     * events. Now the app is much responsive to user action
     */
    while (XPending(dockapp.d))
    {
      msg_dbg(__POSITION__, "X11 activity");
      XNextEvent(dockapp.d, &event);
      switch (event.type)
      {
      case Expose:
        dockapp.update = 1;
        break;
      case DestroyNotify:
        XCloseDisplay(dockapp.d);
        exit(0);
        break;
      case ButtonPress:
        btn = event.xbutton.button;
        rgn = check_mr(event.xbutton.x, event.xbutton.y);
        break;
      case ButtonRelease:
        if(btn == event.xbutton.button)
        {
          if(rgn == check_mr(event.xbutton.x, event.xbutton.y))
          {
            click_event(rgn, btn);
            beat_event();
            draw_stats(wmnd.curdev);
          }
        }
        break;
      }
    }

    ++beats;
    redraw_window();
    usleep(wmnd.refresh);
  }

  devices_destroy();
  exit(0);
}

void
binary_scale(unsigned char sign, unsigned long value, char *buf)
{
  unsigned char scale;
  unsigned int i;
  char *r;

  if(value > 1073741823)
  {
    /* scale in giga */
    value = value >> 30;
    scale = 'G';
  }
  else
  if(value > 1048575)
  {
    /* scale in mega */
    value = value >> 20;
    scale = 'M';
  }
  else
  if(value > 1023)
  {
    /* scale in kilo */
    value = value >> 10;
    scale = 'K';
  }
  else
    scale = ' ';

  sprintf(buf, "%c%lu", sign, value);
  r = buf;
  r++;

  for(i = 3; i > 0 && *r != '\0'; i--)
  {
    if(*r == '+' || *r == '-' || *r == '.')
      ++i;
    ++r;
  }
  *r++ = scale;
  *r = '\0';
}

void
metric_scale(unsigned char sign, unsigned long value, char *buf)
{
  float f;
  unsigned char scale;
  unsigned int i;
  char *r;

  f = (float) value;
  if(value > 999999999)
  {
    /* scale in giga */
    f /= 1000000000;
    scale = 'G';
  }
  else
  if(value > 999999)
  {
    /* scale in mega */
    f /= 1000000;
    scale = 'M';
  }
  else
  if(value > 999)
  {
    /* scale in kilo */
    f /= 1000;
    scale = 'K';
  }
  else
    scale = ' ';

  sprintf(buf, "%c%f", sign, f);	/* to string */
  r = buf;
  r++;

  for(i = 3; i > 0 && *r != '\0'; i--)
  {
    if(*r == '+' || *r == '-' || *r == '.')
      ++i;
    ++r;
  }
  *r++ = scale;
  *r = '\0';
}

void
scale(char *rx_buf, char *tx_buf, unsigned long rx, unsigned long tx)
{
  char rx_sign, tx_sign;

  if(rx > tx)
  {
    rx_sign = '+';
    tx_sign = '#';
  }
  else
  {
    rx_sign = '-';
    tx_sign = '*';
  }

  // return the speed in bps
  if(wmnd.scroll != 10)
  {
    float div = 10. / wmnd.scroll;
    tx = (unsigned long)(div * tx);
    rx = (unsigned long)(div * rx);
  }
  wmnd.scale(tx_sign, tx, tx_buf);
  wmnd.scale(rx_sign, rx, rx_buf);
}

void
draw_string(const char *buf, unsigned int x, unsigned int y)
{
  unsigned int w, sx = 0, sy = 0;
  unsigned int draw;
  const char *r;

  w = 3;
  draw = 0;
  for(r = buf; *r != '\0'; r++)
  {
    if(*r >= '0' && *r <= '9')
    {
      w = 5;
      sx = 1 + (w * (*r - '0'));
      sy = 85;
      draw = 1;
    }
    else
    if(*r == '.')
    {
      w = 2;
      sx = 62;
      sy = 85;
      draw = 1;
    }
    else
    if(*r >= 'A' && *r <= 'Z')
    {
      w = 5;
      sx = 1 + (w * (*r - 'A'));
      sy = 93;
      draw = 1;
    }
    else
    if(*r == '+')
    {
      w = 6;
      if(bit_get(CFG_MAXSCREEN))
      {
        sx = 133;
        sy = 93;
      }
      else
      {
        sx = 66;
        sy = 46;
      }
      draw = 1;
    }
    else
    if(*r == '-')
    {
      w = 6;
      if(bit_get(CFG_MAXSCREEN))
      {
        sx = 139;
        sy = 93;
      }
      else
      {
        sx = 72;
        sy = 46;
      }
      draw = 1;
    }
    else
    if(*r == '*')
    {
      w = 6;
      if(bit_get(CFG_MAXSCREEN))
      {
        sx = 145;
        sy = 93;
      }
      else
      {
        sx = 78;
        sy = 46;
      }
      draw = 1;
    }
    else
    if(*r == '#')
    {
      w = 6;
      if(bit_get(CFG_MAXSCREEN))
      {
        sx = 151;
        sy = 93;
      }
      else
      {
        sx = 84;
        sy = 46;
      }
      draw = 1;
    }
    if(draw)
    {
      copy_xpm_area(sx, sy, w, 7, x, y);
      draw = 0;
    }
    x += w;
  }
}

void
draw_rate(unsigned long rx, unsigned long tx)
{
  char rx_buf[16];
  char tx_buf[16];

  /* clear rate bar */
  copy_xpm_area(100, 85, 58, 7, 3, 54);

  /* put rx/tx numbers into strings, scaling them */
  scale(rx_buf, tx_buf, rx, tx);

  /* draw rx/tx strings */
  draw_string(rx_buf, 3, 54);
  draw_string(tx_buf, 32, 54);
}

void
draw_max(unsigned long rx, unsigned long tx)
{
  char rx_buf[16];
  char tx_buf[16];

  /* clear rate bar */
  copy_xpm_area(100, 85, 58, 7, 3, 11);

  /* put rx/tx numbers into strings, scaling them */
  scale(rx_buf, tx_buf, rx, tx);

  /* draw rx/tx strings */
  draw_string(rx_buf, 3, 11);
  draw_string(tx_buf, 32, 11);
}

void
draw_stats(struct Devices *ptr)
{
  int bpp = 1;		/* bytes per pixel */
  unsigned int k;
  unsigned long int *p;
  unsigned int in, out;
  unsigned long int rx_max_his, tx_max_his;
  unsigned long long int rx_max, tx_max, max;
  unsigned int size;

  if(bit_get(CFG_SHOWMAX))
    size = 35;		/* with max bar mode */
  else
    size = 41;		/* without max bar */

  if(bit_get(CFG_MODE))
  {
    /* bytes mode */
    in = 0;
    out = 1;
    rx_max_his = ptr->ib_max_his;
    tx_max_his = ptr->ob_max_his;
  }
  else
  {
    /* packets mode */
    in = 2;
    out = 3;
    rx_max_his = ptr->ip_max_his;
    tx_max_his = ptr->op_max_his;
  }

  /* find maximum value in screen history */
  rx_max = tx_max = 0;
  p = (unsigned long *) ptr->his;
  for(k = 0; k < 58; k++)
  {
    rx_max = MAX(rx_max, p[in]);
    tx_max = MAX(tx_max, p[out]);
    p += 4;
  }
  if((max = rx_max + tx_max) > size)
  {
    bpp = max / size;
    if((max % size) > 0)
      ++bpp;
  }

  /* draw rx/tx rate */
  p = ptr->avg;
  draw_rate(p[in], p[out]);

  if(bit_get(CFG_MAXSCREEN))
    draw_max(rx_max, tx_max);
  else
    draw_max(rx_max_his, tx_max_his);

  p = (unsigned long *) ptr->his;
  (*drwFuncs[wmnd.wavemode].funcPtr)(p, in, out, size, bpp, rx_max, tx_max);

  /* copy PPP connection time over the graph */
  if(bit_get(CFG_SHOWTIME) && wmnd.curdev->devstart)
    copy_xpm_area(70, 36, 23, 7, 37, 46);
}

static void
led_control(const unsigned char led, const unsigned char mode)
{
  msg_dbg(__POSITION__, "led: %02x[%02x]", led, mode);
  switch (led)
  {
  case LED_POWER:
    switch (bit_get(CFG_MODE))
    {
    case 1:
      /* bytes */
      if(mode)
        /* on-line */
        copy_xpm_area(116, 64, 5, 7, 55, 4)
      else
        /* off-line */
        copy_xpm_area(122, 64, 5, 7, 55, 4);
      break;
    case 0:
      /* packets */
      if(mode)
        /* on-line */
        copy_xpm_area(128, 64, 5, 7, 55, 4)
      else
        /* off-line */
        copy_xpm_area(134, 64, 5, 7, 55, 4);
      break;
    }
    break;
  case LED_RX:
    if(mode)
    {
      /* turn on */
      if(bit_get(LED_RX))
      {
        msg_dbg(__POSITION__, "RX led already on");
        return;
      }
      copy_xpm_area(86, 69, 5, 4, 41, 4);
      bit_set(LED_RX);
      msg_dbg(__POSITION__, "RX led on");
    }
    else
    {
      /* turn off */
      if(!bit_get(LED_RX))
      {
        msg_dbg(__POSITION__, "RX led already off");
        return;
      }
      copy_xpm_area(92, 69, 5, 4, 41, 4);
      bit_off(LED_RX);
      msg_dbg(__POSITION__, "RX led off");
    }
    break;
  case LED_TX:
    if(mode)
    {
      /* turn on */
      if(bit_get(LED_TX))
      {
        msg_dbg(__POSITION__, "TX led already on");
        return;
      }
      copy_xpm_area(86, 64, 5, 4, 48, 4);
      bit_set(LED_TX);
      msg_dbg(__POSITION__, "TX led on");
    }
    else
    {
      /* turn off */
      if(!bit_get(LED_TX))
      {
        msg_dbg(__POSITION__, "TX led already off");
        return;
      }
      copy_xpm_area(92, 64, 5, 4, 48, 4);
      bit_off(LED_TX);
      msg_dbg(__POSITION__, "TX led off");
    }
    break;
  }
}

void
draw_interface(void)
{
  int i;
  int c;
  int k = 3;
  char temp[7];
  char *cur_name = wmnd.curdev->name;
  int cur_namelen = strlen(cur_name);

  /* refresh */
  copy_xpm_area(65, 54, 38, 9, 3, 3);
  led_control(LED_POWER, bit_get(RUN_ONLINE));

  /*
   * little modify to handle names shorter than 4 chars and minor bugfix
   * for names shorter than 6 chars (memory overwrite)
   */
  if(bit_get(CFG_SHORTNAME))
  {
    strncpy(temp, cur_name, (cur_namelen<4)?cur_namelen:3);
    temp[3] = (cur_namelen < 4)? '\0': cur_name[cur_namelen - 1];
    temp[4] = '\0';
  }
  else
  {
    if(cur_namelen > 6)
      cur_namelen = 6;
    strncpy(temp, cur_name, cur_namelen);
    temp[cur_namelen] = '\0';
  }

  for(i = 0; temp[i]; i++)
  {
    c = temp[i];
    if(c >= '0' && c <= '9') {
      c -= '0';
      copy_xpm_area(c * 6, 64, 6, 9, k, 3);
      k += 6;
    } else {
      if(c >= 'a' && c <= 'z')
        c = c - 'a' + 'A';
      c -= 'A';
      copy_xpm_area(c * 6, 74, 6, 9, k, 3);
      k += 6;
    }
  }
}

void
usage(void)
{
  int cnt = 0;

  fprintf(stderr,
      "wmnd - WindowMaker Network Devices %s\n"
      "Home page: http://www.hydra.ubiest.com/wmnd/,\n"
      "           http://www.wingeer.org/wmnd/\n\n"
      "usage:\n"
      "  -b                  base 2 scale (no fractions)\n"
      "  -c <color>          tx color\n"
      "  -C <color>          rx color\n"
      "  -L <color>          middle line color\n"
      "  -d <display name>\n"
      "  -h                  this help screen\n"
      "  -i <interface name> select this interface at startup\n"
      "  -l                  start using long device names\n"
      "  -m                  start with max values hidden\n"
      "  -M                  use max values from the entire history\n"
      "  -f <config>         read config instead of ~/.wmndrc\n"
      "  -F                  don't parse ~/.wmndrc\n"
      "  -r <rate>           refresh rate in microseconds\n"
      "  -s <scroll>         scroll rate in tenth of seconds\n"
      "  -S <samples>        number of samples to average for the speed indicator\n"
      "  -t                  start without displaying time\n"
      "  -v                  print the version number\n"
      "  -q                  be less verbose\n"
      "  -Q                  enable verbosity\n"
      "  -o <float>          smoothing factor (0-1)\n"
      "  -w <mode>           select display mode (see below)\n"
      "  -D <driver>         specify a driver to use\n"
      "  -I <interface name> tell to driver/s the interface to monitor\n\n"
      "builtin drivers: ", WMND_VERSION);

  /* display builting drivers */
  while(drivers_table[cnt].driver_name)
  {
    if(drivers_table[cnt+1].driver_name)
      fprintf(stderr, "%s, ", drivers_table[cnt].driver_name);
    else
      fprintf(stderr, "%s.\n", drivers_table[cnt].driver_name);
    cnt++;
  }

  /* display builting display modes */
  fprintf(stderr, "builtin display modes: ");
  cnt = 0;
  while(drwFuncs[cnt].funcName)
  {
    if(drwFuncs[cnt + 1].funcName)
      fprintf(stderr,"%s, ",drwFuncs[cnt].funcName);
    else
      fprintf(stderr,"%s.\n",drwFuncs[cnt].funcName);
    cnt++;
  }
}

void
printversion(void)
{
  printf("%s\n", WMND_VERSION);
}

int
devices_init(const char *driver, const char *interface)
{
  /*
   * devices init, initializes all drivers/devices
   * return: 0 on success, 1 on error
   */

  int cnt = -1;
  int devnum;
  int in_loop0;
  struct Devices *prt;
  struct Devices *aftPrt;
  struct Devices *tmpPrt;
  wmnd.nr_devices = 0;

  /* creating and empy first device */
  msg_dbg(__POSITION__, "initializing devices");

  devices = (struct Devices*)malloc(sizeof(struct Devices));
  devices->next = NULL;
  prt = devices;

  /* starting all drivers */
  do
  {
    cnt++;

    /* check if driver is specified and check devicename */
    if(driver && strcmp(driver, drivers_table[cnt].driver_name))
      continue;

    /* device matches or driver is NULL, then check if driver is avaible */
    msg_dbg(__POSITION__, "probing %s", drivers_table[cnt].driver_name);
    aftPrt = prt;
    devnum = (*drivers_table[cnt].list_devices)(interface, prt);
    if(!devnum)
      continue;

    /* driver is avaible, set up all driver's devices */
    for(in_loop0 = 0; in_loop0 < devnum; in_loop0++)
    {
      prt = prt->next;
      prt->drvnum = cnt; /* set the driver number */
      if((*drivers_table[cnt].init_driver)(prt) == 1)	/* init the device */
      {
        msg_err("failed to initialize device %s,%d",
            drivers_table[cnt].driver_name, in_loop0);
        devnum--;

        /* last element in list */
        if(in_loop0 == devnum-1 || !devnum)
        {
          free(prt);
          prt = aftPrt;
          continue;
        }

        /* remove device from list */
        tmpPrt = prt->next;
        free(prt);
        prt = tmpPrt;

        if(in_loop0 == devnum) continue;
        aftPrt = prt;
        in_loop0--;							
      }
      else
      {
        /* initialize statistics for this device */
        int cnt;
        unsigned long int ib, ob, ip, op;

        /* sample some stats to inizialize cleanly the graph */
        (*drivers_table[prt->drvnum].get_stats)(prt, &ip, &op, &ib, &ob);

        prt->ib_max_his = ib;
        prt->ob_max_his = ob;
        prt->ip_max_his = ip;
        prt->op_max_his = op;
        prt->ib_stat_last = ib;
        prt->ob_stat_last = ob;
        prt->ip_stat_last = ip;
        prt->op_stat_last = op;

        /* cleaning history */
        for(cnt = 0; cnt < 58; cnt++)
          memset(prt->his, 0, sizeof(prt->his));

        /* clean average sampling buffers */
        prt->avgBuf[0] = ib;
        prt->avgBuf[1] = ob;
        prt->avgBuf[2] = ip;
        prt->avgBuf[3] = op;
      }
    }

    /* all new devices are ok */
    wmnd.nr_devices += devnum;
  }
  while(drivers_table[cnt+1].driver_name);

  /* remove the first null element */
  prt = devices;
  devices = prt->next;
  free(prt);

  return (!wmnd.nr_devices);
}

void
devices_select(const char *interface)
{
  /*
   * devices_select: sets wmnd.curdev looking up a name
   * if name is null, cycles for next device
   */

  struct Devices *prt = devices;

  if(interface)
  {
    /* searching for a device */
    wmnd.curdev = devices;
    while(prt)
    {
      if(!strcmp(interface, prt->name))
      {
        /* name matches */
        wmnd.curdev = prt;
        break;
      }
      prt = prt->next;
    }
  }
  else
  {
    /* switching active device */
    if(!wmnd.curdev->next)
      wmnd.curdev = devices; /* last element, back to first */
    else
      wmnd.curdev = wmnd.curdev->next;
  }
}

void
devices_getstat(struct Devices *device, unsigned long *ip, unsigned long *op,
    unsigned long *ib, unsigned long *ob)
{
  /*
   * devices_getstat: run appropriate get_stats for device
   */

  device->online =
    (*drivers_table[device->drvnum].get_stats)(device, ip, op, ib, ob);
}

void
devices_destroy()
{
  /*
   * devices_destroy: sends destroy signal to all devices and frees devices
   */

  struct Devices *ptr;
  while(devices->next)
  {
    ptr = devices;
    devices = devices->next;
    (*drivers_table[ptr->drvnum].terminate_driver)(ptr);
    free(ptr->name);
    free(ptr);
  }
}

void
devices_restart(int sig)
{
  /*
   * restart the driver subsystem without interrupting wmnd.
   * call each device destructor and recall the constructor.
   *
   * TODO: warning! still using usafe signals.
   */

  struct Devices* ptr = devices;
  msg_info("restarting devices");

  for(; ptr->next; ptr = ptr->next)
  {
    (*drivers_table[ptr->drvnum].terminate_driver)(ptr);
    (*drivers_table[ptr->drvnum].init_driver)(ptr);
  }
}

