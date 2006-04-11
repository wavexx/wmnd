# process this file with autoconf >= 2.5 to produce a configure script.
# report bugs and comments to wavexx@users.sf.net

# initialization
AC_INIT(wmnd, 0.4.12, wavexx@users.sf.net)
AC_CONFIG_SRCDIR(src/wmnd.c)
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

# build date
ISODATE="`date +%Y-%m-%d`"
AC_SUBST(ISODATE)

# C compiler rules
AC_PROG_CC
AC_HEADER_STDC
AC_HEADER_TIME
AC_CHECK_HEADERS(sys/time.h)
AC_CHECK_HEADERS(net/ppp_defs.h)

# math library
AC_SEARCH_LIBS(sin, m,, [AC_MSG_ERROR(math library is required)])

# X libraries
AC_PATH_XTRA
CFLAGS="$CFLAGS $X_CFLAGS"
LDFLAGS="$LDFLAGS $X_PRE_LIBS $X_LIBS $X_EXTRA_LIBS"

# check for XPM headers, libraries
AC_HAVE_LIBRARY(X11,, [AC_MSG_ERROR(X11 library is required)])
AC_HAVE_LIBRARY(Xext,, [AC_MSG_ERROR(Xext library is required)])
AC_HAVE_LIBRARY(Xpm,, [AC_MSG_ERROR(Xpm library is required)])

# check for standard typedefs
AC_TYPE_PID_T
AC_TYPE_SIGNAL
AC_TYPE_SIZE_T


# like AC_ARG_ENABLE, but with COMMON SENSE(tm) added
AC_DEFUN([ARG_ENABLE],
[
	AC_ARG_ENABLE([$1], [$2],, [enableval="no"])
	AS_IF([test "$enableval" = "no"], [$4], [$3])
])

AC_DEFUN([ARG_DISABLE],
[
	AC_ARG_ENABLE([$1], [$2],, [enableval="yes"])
	AS_IF([test "$enableval" = "no"], [$3], [$4])
])


# drivers selection
ARG_ENABLE(drivers,
	[AS_HELP_STRING(
		[--enable-drivers="..."],
		[manually selects drivers. defaults to auto])],
[
	ac_drivers="$enable_drivers"
	AC_MSG_NOTICE([overriding driver autodetection: $ac_drivers])
], [
	# let the user specify an empty set
	ac_drivers="auto"
])

# driver detection block: skip this block if the user manually specify
# drivers on the command line
AS_IF([test "$ac_drivers" = "auto"],
[
	# reset the state variable
	ac_drivers=""
	
	# linux_proc
	AC_CHECK_FILE(/proc/net/dev,
		[ac_drivers="$ac_drivers linux_proc"]
	)
	
	# freebsd_sysctl
	AC_MSG_CHECKING(for FreeBSD sysctl availability)
	AC_PREPROC_IFELSE(
	[
		#include <net/if_mib.h>
		#include <sys/sysctl.h>
	], [
		ac_drivers="$ac_drivers freebsd_sysctl"
		AC_MSG_RESULT(yes)
	], [
		AC_MSG_RESULT(no)
	])
	
	# solaris_fpppd
	AC_CHECK_HEADER(sys/stropts.h,
		[AC_CHECK_FILE(/dev/ppp,
			[ac_drivers="$ac_drivers solaris_fpppd"]
		)]
	)
	
	# solaris_kstat
	AC_CHECK_HEADER(kstat.h,
		[AC_SEARCH_LIBS(kstat_open, kstat,
			[ac_drivers="$ac_drivers solaris_kstat"]
		)]
	)

	# netbsd_ioctl
	AC_MSG_CHECKING(for NetBSD ioctl availability)
	AC_COMPILE_IFELSE(
	[
		#include <sys/types.h>
		#include <sys/sockio.h>
		#include <sys/ioctl.h>
		#include <net/if.h>
		#include <ifaddrs.h>

		void aFunc() { ioctl(0, SIOCGIFDATA, NULL); }
	], [
		ac_drivers="$ac_drivers netbsd_ioctl"
		AC_MSG_RESULT(yes)
	], [
		AC_MSG_RESULT(no)
	])

	# irix_pcp
	AC_CHECK_HEADER(pcp/pmapi.h,
		[AC_SEARCH_LIBS(pmLookupName, pcp,
			[ac_drivers="$ac_drivers irix_pcp"]
		)]
	)

	# generic_snmp
	AC_CHECK_HEADER(net-snmp/net-snmp-config.h,
		[AC_SEARCH_LIBS(snmp_sess_init, [snmp netsnmp],
			[ac_drivers="$ac_drivers generic_snmp"]
		)]
	)
])
for driver in $ac_drivers;
do
	# we must repeat the define constants many times
	# to make autoheader automatically recognize them all
	case "$driver" in
	linux_proc)	AC_DEFINE(USE_LINUX_PROC, "linux_proc", [enable linux proc driver]);;
	freebsd_sysctl)	AC_DEFINE(USE_FREEBSD_SYSCTL, "freebsd_sysctl", [enable freebsd sysctl driver]);;
	netbsd_ioctl)	AC_DEFINE(USE_NETBSD_IOCTL, "netbsd_ioctl", [enable nebsd ioctl driver]);;
	solaris_fpppd)	AC_DEFINE(USE_SOLARIS_FPPPD, "solaris_fpppd", [solaris streams pppd]);;
	solaris_kstat)	AC_DEFINE(USE_SOLARIS_KSTAT, "solaris_kstat", [enable solaris kstat driver]);;
	irix_pcp)	AC_DEFINE(USE_IRIX_PCP, "irix_pcp", [IRIX Performance Co-Pilot]);;
	generic_snmp)	AC_DEFINE(USE_GENERIC_SNMP, "generic_snmp", [Generic SNMP module]);;
	*)		AC_MSG_ERROR([unknown driver name $driver]);;
	esac
	
	drivers="$drivers $driver"
done


# creates a new switch (--enable-) with and help string which controls
# the definition of a conditional.
AC_DEFUN([ARG_EC],
[
	ARG_ENABLE([$1], [AS_HELP_STRING([--enable-$1], [$3])],	[
		AC_DEFINE([$2],, [$3])
	])
])


# Dummy driver
ARG_DISABLE(dummy-driver,
	[AS_HELP_STRING([--disable-dummy-driver],
		[disable the dummy driver])],,
[
	drivers="$drivers testing_dummy"
	AC_DEFINE(USE_TESTING_DUMMY,, [enable the dummy driver])
])

# Simple options
ARG_EC(sine-dummy, USE_SINE_TESTING_DUMMY,
	[enhanced dummy driver])
ARG_EC(snmp-descr, USE_GENERIC_SNMP_DESCR,
	[prefer ifDescr instead of ifName for snmp devices])
ARG_EC(inexact-timing, INEXACT_TIMING,
	[use an inexact (but smoother/faster) timing method])
ARG_EC(trend, USE_TREND,
	[enable support for trend (history zoom/inspection)])


# display modes selection
ARG_ENABLE(modes,
	[AS_HELP_STRING(
		[--enable-modes="..."],
		[manually select visual modes. defaults to all])],,
	[enable_modes="traditional mgraph waveform wmwave wmnet sepgraphs twisted charts needle lines"]
)
for mode in $enable_modes;
do
	# we must repeat the define constants many times
	# to make autoheader automatically recognize them all
	case "$mode" in
	traditional)	AC_DEFINE(USE_DRW_TRADITIONAL,, [traditional drawing mode]);;
	mgraph)		AC_DEFINE(USE_DRW_MGRAPH,, [mgraph drawing mode]);;
	waveform)	AC_DEFINE(USE_DRW_WAVEFORM,, [waveform drawing mode]);;
	wmwave)		AC_DEFINE(USE_DRW_WMWAVE,, [wmwave drawing mode]);;
	wmnet)		AC_DEFINE(USE_DRW_WMNET,, [wmnet drawing mode]);;
	sepgraphs)	AC_DEFINE(USE_DRW_SEPGRAPHS,, [sepgraphs drawing mode]);;
	twisted)	AC_DEFINE(USE_DRW_TWISTED,, [twisted drawing mode]);;
	charts)		AC_DEFINE(USE_DRW_CHARTS,, [charts drawing mode]);;
	needle)		AC_DEFINE(USE_DRW_NEEDLE,, [needle drawing mode]);;
	lines)		AC_DEFINE(USE_DRW_LINES,, [lines drawing mode]);;
	*)		AC_MSG_ERROR([unknown display mode $mode]);;
	esac
	
	dspmodes="$dspmodes $mode"
done


# check for inline statement useability
AC_C_INLINE
AC_DEFINE(INLINE, inline, [code compatibility stub])

# check for basic type sizes
AC_CHECK_SIZEOF(unsigned int)
AC_CHECK_SIZEOF(unsigned long)

# output files
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	wmnd.lsm
	wmnd.spec
])

AC_OUTPUT

AC_MSG_NOTICE([configuration:

trend support:         ${enable_trend:-no}
enhanced dummy driver: ${enable_sine_dummy:-no}
inexact timing:        ${enable_inexact_timing:-no}
prefer snmp ifDescr:   ${enable_snmp_descr:-no}
enabled drivers:      ${drivers:- none}
enabled display modes:${dspmodes:- none}
])
