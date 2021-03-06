WMND - Window Maker Network Devices
===================================

WMND (WindowMaker Network Devices) is a highly customizable network monitoring
dockapp for Window Maker (and compatibles) for many operative systems and
devices. WMND can be found at http://www.thregr.org/~wavexx/software/wmnd/

.. contents::


Hints for WMND
--------------

Generic
~~~~~~~

WMND supports command-line options, 'wmnd -h' prints help about them.

Use option -i to monitor a particular interface at startup::

  wmnd -i eth0 &
  wmnd -i ppp0 &

Without the -i option, wmnd will auto-magically use the interface that is first
found in /proc/net/dev (or the current driver), but skip the lo and irda.

Use the -I option to load only a specific interface into wmnd. By default wmnd
loads all available interfaces.

To monitor the lo or irda devices you must manually force wmnd as shown below::

  wmnd -I lo &
  wmnd -I irda &

To monitor dialup interfaces under linux either use the -I ppp0 flag or use the
streams solaris_fpppd driver (-D solaris_fpppd -I ppp0).  The linux_proc driver
supports multiple interfaces on the command line through the -I flag::

  wmnd -D linux_proc -I eth0,eth1,ppp0

This way you can combine multiple interfaces (whether they're online or not) on
the same WMND instance. This trick allows also to create handy dialup shortcuts
using the button actions (by passing the active interface name and status on
the command line). Look at the example wmndrc file for details.


GUI Usage
~~~~~~~~~

You can cycle in real-time through all available active interfaces by simply
left-clicking on the interface name gadget on the upper-left corner of wmnd.

By default, wmnd show device name in short term of four characters, for
example, the ippp0 will be displayed as ipp0.  You can toggle the device name
between short and long by right-click on it.

Left-click on the main graphic area to cycle the graphic mode.

Left-click to toggle the history max or screen max, default is screen max when
wmnd is startup.  Right-click to hide or display.

Left-click on the letter gadgeted on the right-top corner can switch between
the Byte or Packet counter mode. "B" for byte, "p" for packet.

Click on the bottom rate meter can invoke the user command defined in resource
file .wmndrc.

Be sure to drag WMND on it's outer edges, it's a bit picky due to the large gfx
pixmap it keeps ;-). You can also use a keyboard+mouse shortcut (perhaps
ALT+left-click) in your window manager to drag it around.

The history/graph can be viewed and inspected more conveniently by clicking
with the middle button on the scale meter. With this feature, multiple
interfaces can be monitored concurrently. See the "trend" section below.


Transition from 0.2/0.3
~~~~~~~~~~~~~~~~~~~~~~~

If you are coming from an old release of WMND (0.2/0.3 series) you must be
aware that some values/scales of the ~/.wmndrc file have changed. Copy a fresh
wmndrc from the source distribution (examples/wmndrc) or erase your old wmndrc
to get the new defaults.


Firing up WMND with sensible defaults
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Default values are easily changed from the command line or through the
~/.wmndrc file. The ~/.wmndrc file gets created automatically the first time
you execute WMND (unless you use -F) using the internal defaults and command
line flags. That is, if you want to change "permanently" the defaults (so you
can start it without command line fuss) just remove ~/.wmndrc and launch WMND
using all the flags again. Beware however that internal defaults may
change. Consider reading the example wmndrc that comes with the distribution.


Creating PPP dialup scripts
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Because a very good PPP HowTo already exists, it's quite pointless for us to
explain to you how you should and make them. Look for mouse button actions
(bt?_action) into the example wmndrc.

Read the PPP HowTo, and you'll see that it's very easy to create your own PPP
scripts.


Window Managers
---------------

WindowMaker
~~~~~~~~~~~

WindowMaker users simply drag and drop the WMND dock.app on the WindowMaker
Dock (preferred) or on the Fiend, and then press the right mouse button on the
outer edges of WMND and then enable 'Auto launch' from the Dock pop-up menu.


AfterSTEP
~~~~~~~~~

AfterSTEP users put the following in their .steprc::

  "Wharf wmnd - MaxSwallow "wmnd" wmnd -i eth0 -w &".

Change the WMND's title name in order to not swallow multiple instances into a
single dock (see -n).


BlackBox or FluxBox
~~~~~~~~~~~~~~~~~~~

You must enable the slit in other make wmnd visible. To add wmnd to the slit
simply run wmnd from the command line::

  wmnd &

and it will pop-up into a new slot.


Other Window managers
~~~~~~~~~~~~~~~~~~~~~

For other windowmanagers, WMND runs nicely as 64x64 pixel shaped icon on your
desktop.

Under gnome add the "swallow" applet and set it to run wmnd. The same thing can
be done under KDE using the kdeswallow applet.

PS: FVWM can swallow it too, so we've heard ;-)


Display modes
-------------

Configure --enable-modes flags (space separated): traditional mgraph waveform
wmwave wmnet sepgraphs twisted charts needle lines.

Traditional:
	Tx and Rx are piled on the same line within a single graph (like wmifs
	original behavior).

MGraph:
	Tx and Rx scale is calculated uniquely and the slower channel is shown
	in front of the other one.

Waveform:
	Tx and Rx are piled on the same line and mirrored using the central
	line, in way to generate a "waveform like" silhouette

WmWave:
	Tx and Rx are opposite to the central line, however the scale is
	calculated using both values

WmNet:
	This graph shows a reversed wmwave mode without the central line.
	(like wmnet behavior)

Sepgraphs:
	Tx and Rx are shown on separated graphs. Tx in top and Rx in bottom,
	divided by a central line. Scales are calculated separately.

Twisted:
	This mode displays two vertical "waveform like" graphs flowing in
	reverse directions. The scale is unique.

Charts:
	Displays four charts disposed in this manner:

	  | TX Current Speed Indicator
	  | TX Average Speed Indicator
	  | RX Current Speed Indicator
	  | RX Average Speed Indicator

	The range is from 0 to the maximal value in history. The average speed
	is calculated using the latest 58 samples for each channel.

Needle:
	Displays three needles. The arrangement is as follows:

	  | TX Average Needle
	  |   Bandwidth utilization Needle
	  | RX Average Needle

	The range For TX and RX needles is between 0 and the maximal value in
	history. The bandwidth utilization one uses this formula:

	  (tx_avg+rx_avg)/(tx_max+rx_max)

	I particularly like this mode :), the needles speed are slow and smooth
	like a real analog display.

Lines:
	Displays a trend graph using two lines.


Drivers
-------

Introduction
~~~~~~~~~~~~

Since release 0.3, WMND is now portable to many operating systems by using
compile-time defines and an internal driver table (now it should be possible to
make WMND monitor just about anything). Current supported OSs/drivers includes:

Configure --enable-drivers flags (space separated) drivers: linux_proc
freebsd_sysctl solaris_fpppd solaris_kstat irix_pcp generic_snmp netbsd_ioctl.

linux_proc:
	Supports Linux >2.2 by using the /proc/net/dev file.

freebsd_sysctl:
	FreeBSD (and Darwin) sysctl based.

solaris_fpppd:
	Free PPPd (for Linux, Solaris, and maybe others) for ppp dialup
	interfaces.

solaris_kstat:
	Solaris kstat based.

irix_pcp:
	Based on PCP (Performance Co-Pilot API 2).

generic_snmp:
	Since WMND 0.4.5 you can now monitor IF-MIB local and/or remote snmp
	interfaces a-la MRTG, but in realtime!

netbsd_ioctl:
	NetBSD ioctl based.

Due to the new handling of the drivers, WMND won't add new devices (like PPP
interfaces in /proc/net/dev) and remove them as they appear on the
/proc/net/dev file. Offline devices are now shown as red (disabled). If you're
under linux and still require to monitor a dialup interface you'll need to feed
it on the command line through the -I flag.


irix_pcp
~~~~~~~~

The IRIX driver is based on the PCP API 2.x (Performance Co-Pilot). You'll need
'pcpd' running for WMND to work. Interface format::

  [host@]interface

These filesets are required::

  pcp_eoe.sw.eoe
  pcp.sw.base

If you don't have these, you can download PCP directly from here:
http://www.sgi.com/software/co-pilot/ (pcp_eoe.* filesets are into
"IRIX Overlays, 2/4" and "Foundation 2").

In some cases WMND may fail to compile due to the presence of Motif XPM
headers: be sure to have freeware's XPM installed (see
http://freeware.sgi.com/) and have "/usr/freeware/include" paths before any
other.

There seems to be a Linux version of PCP, but I can't (don't) want to try
it. It will probably work.


generic_snmp
~~~~~~~~~~~~

Since 0.4.5 wmnd adds a new snmp driver for local and/or remote IF-MIB
compliant devices. This driver requires the NET-SNMP library, available at
http://www.net-snmp.org/, version 5 or higher. The drivers name is
generic_snmp. It uses the parameters sent through the -I flag to initialize the
device/s list. The format is as follows::

  [community@]host[:interface]

Parameters inside [] are optional. If you do not explicitly supply these
parameters, generic_snmp will use "public@localhost".

To monitor an entire switch, you can usually do::

  wmnd -D generic_snmp -I public@switch

(or "-I switch", which is shorter). To monitor only a specific interface::

  wmnd -D generic_snmp -I public@switch:1

Interface numbers start at 1 (0 means all interfaces, like an empty interface
specification). Beware that EACH remote interface, at the default refresh
speed, burns AT LEAST 2kB/s of continuous data stream through your
network. Monitoring a complete switch (24 ports), always at the default refresh
speed, burns circa 30kB/s. You can slow down the refresh speed (-r) to reduce
this traffic. The extremely fast queries done by wmnd can also reduce the
available cpu of your snmp server/hardware and decrease overall performance.

Also beware that using the -I flag on the command-line can potentially expose
the community name of your remote snmp server. Better to use ~/.wmndrc and
chmod it to 600 in this case.


Building WMND 0.4
-----------------

Configure flags
~~~~~~~~~~~~~~~

Since release 0.4 WMND is now compliant to the GNU Packaging standards and
enables to use the GNU autotools to automagically build WMND for your box with
little or no difficulty. See INSTALL for a first-time introduction.

Configure accepts several options to enchance/minimize wmnd functionality and
size. To forcely disable the dummy driver::

  $ ./configure --disable-dummy-driver

To select only some display modes::

  $ ./configure --enable-modes="traditional wmwave"

To forcely build specified drivers (beware that the dummy driver should be
disabled with --disable-dummy-driver, also, extra libraries that may be needed
by the driver won't be checked automatically)::

  $ ./configure --enable-drivers="linux_proc"

The --help flag will show you a complete list of command line flags that the
configure script supports.


GCC 2.96 sucks
~~~~~~~~~~~~~~

WMND 0.4 won't compile with RedHat's gcc 2.96. Either downgrade to 2.95 or
upgrade to 3. DON'T SEND bug reports about INLINE not working under gcc 2.96,
it's a RedHat-only bug.


Notes for packagers
~~~~~~~~~~~~~~~~~~~

I'm not a professional packager, but you may want to consider these notes to
improve the WMND package.

The irix_pcp and generic_snmp drivers depends upon libraries that are not
installed on a distribution/OS by default. Both these drivers are supported by
different operating systems. I suggest you to build at least one package that
doesn't require them.


Trend support
~~~~~~~~~~~~~

You can examine the current history in a larger window by clicking with the
middle mouse button on the scale meter (the panel under the device name).
Bytes/Packets mode affects the counters involved.

You can leave the trend's window open, cycle the active interface and
middle-click again to monitor multiple interfaces concurrently. trend settings
and history can be adjusted via wmndrc. Read the sample file coming in the
source for details.

You will need trend[1] to be installed for this feature to work.
Any version of trend starting with Rev #68 02/11/2007 should work.

[1] http://www.thregr.org/~wavexx/software/trend/


Copyright
---------

WMND was originally based on WMiFS, forked around 2001 by Reed Lai. WMND is
currently maintained by Yuri D'Elia and distributed under GNU GPL v2 or
above. See AUTHORS and COPYING for detailed licensing details.


Bugs
----

A list of wishes and bugs can be found at the Debian WMND bugs page[1].

[1] http://bugs.debian.org/cgi-bin/pkgreport.cgi?src=wmnd

The FreeBSD driver is known to have problems on laptop systems where you can
dynamically insert/remove addictional PCMCIA interfaces. This is rather a
design problem of the driver. A developer with FreeBSD's MIB knowledge would be
helpful.

Certainly there are more. I actually test WMND only on Solaris, Linux and IRIX
boxes. You can report bugs to the current maintainer's email:
<wavexx@thregr.org>. Please be as descriptive as possible and always include
at least:

* WMND version
* your host/target operating system
* compiler used
* A backtrace of the crash would be helpful, but less essential.
