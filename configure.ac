# process this file with autoconf >= 2.5 to produce a configure script.
# report bugs and comments to wavexx@users.sf.net
# $Id$

# initialization
AC_INIT(wmnd, 0.4.6, wavexx@users.sf.net)
AC_CONFIG_SRCDIR("src/wmnd.c")
AM_CONFIG_HEADER(config.h)

# detect terget host
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE

# build date
ISODATE="`date +%Y-%m-%d`"
AC_SUBST(ISODATE)

# Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_AWK
AC_PROG_MAKE_SET

# add warnings and stricter checking
AC_CC_WARNINGS
AC_CC_PEDANTIC
AC_CC_DEBUG

# Checks for standard header files.
AC_HEADER_STDC

# time headers
AC_HEADER_TIME
AC_CHECK_HEADERS(sys/time.h)

# math library
AC_SEARCH_LIBS(sin, m,, [AC_MSG_ERROR(m library is required)])

# X libraries
AC_PATH_XTRA
CFLAGS="$CFLAGS $X_CFLAGS"
LDFLAGS="$LDFLAGS $X_LIBS $X_EXTRA_LIBS"

# check for XPM headers, libraries
AC_HAVE_LIBRARY(X11,, [AC_MSG_ERROR(X11 library is required)])
AC_HAVE_LIBRARY(Xext,, [AC_MSG_ERROR(Xext library is required)])
AC_HAVE_LIBRARY(Xpm,, [AC_MSG_ERROR(Xpm library is required)])

# check for standard typedefs
AC_TYPE_PID_T
AC_TYPE_SIGNAL
AC_TYPE_SIZE_T


# drivers selection
AC_ARG_WITH(drivers,
	[AC_HELP_STRING(
		[--with-drivers="..."],
		[manually selects drivers. defaults to auto]
	)],
	[ac_drivers="$with_drivers"],
	[ac_drivers="auto"]
)


# driver detection block: skip this block if the user manually specify
# drivers on the command line
if test "$ac_drivers" = "auto";
then
	# reset the state variable
	ac_drivers=""
	
	# check whether enable the LINUX_PROC driver
	AC_CHECK_FILE(/proc/net/dev,
		[ac_drivers="$ac_drivers linux_proc"]
	)
	
	# freebsd sysctl driver
	AC_MSG_CHECKING([for *BSD net/if_mib.h presence])
	AC_PREPROC_IFELSE([#include <net/if_mib.h>],
		[AC_MSG_RESULT([yes])
		AC_CHECK_HEADERS(sys/sysctl.h,
			[ac_drivers="$ac_drivers freebsd_sysctl"]
		)],
		[AC_MSG_RESULT([no])]
	)
	
	# solaris/linux streams pppd driver
	AC_CHECK_HEADER(sys/stropts.h,
		[AC_CHECK_FILE(/dev/ppp,
			[ac_drivers="$ac_drivers solaris_fpppd"]
		)]
	)
	
	# check whether enable the KSTAT driver
	AC_CHECK_HEADER(kstat.h,
		[AC_SEARCH_LIBS(kstat_open, kstat,
			[ac_drivers="$ac_drivers solaris_kstat"]
		)]
	)

	# irix pcp
	AC_CHECK_HEADER(pcp/pmapi.h,
		[AC_SEARCH_LIBS(pmLookupName, pcp,
			[ac_drivers="$ac_drivers irix_pcp"]
		)]
	)

	# generic snmp
	AC_CHECK_HEADER(net-snmp/net-snmp-config.h,
		[AC_SEARCH_LIBS(snmp_sess_init, snmp,
			[ac_drivers="$ac_drivers generic_snmp"]
		)]
	)
fi
for driver in $ac_drivers;
do
	# we must repeat the define constants many times
	# to make autoheader automatically recognize them all
	case "$driver" in
	linux_proc)	AC_DEFINE(USE_LINUX_PROC, "linux_proc", [enable linux proc driver]);;
	freebsd_sysctl)	AC_DEFINE(USE_FREEBSD_SYSCTL, "freebsd_sysctl", [enable freebsd sysctl driver]);;
	solaris_fpppd)	AC_DEFINE(USE_SOLARIS_FPPPD, "solaris_fpppd", [solaris streams pppd]);;
	solaris_kstat)	AC_DEFINE(USE_SOLARIS_KSTAT, "solaris_kstat", [enable solaris kstat driver]);;
	irix_pcp)	AC_DEFINE(USE_IRIX_PCP, "irix_pcp", [IRIX Performance Co-Pilot]);;
	generic_snmp)	AC_DEFINE(USE_GENERIC_SNMP, "generic_snmp", [Generic SNMP module]);;
	*)		AC_MSG_ERROR([unknown driver name $driver]);;
	esac
	
	drivers="$drivers $driver"
done


# dumb driver
AC_ARG_WITH(dummy-driver,
	[AC_HELP_STRING(
		[--without-dummy-driver],
		[compile without the dumb driver]
	)]
)
AC_MSG_CHECKING(whether enable dummy driver)
if test "$with_dummy_driver" != "no";
then
	AC_DEFINE(USE_TESTING_DUMMY, "testing_dummy", [internal fallback driver])
	drivers="$drivers testing_dummy"
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi


# enchanged dummy driver
AC_ARG_ENABLE(sine-dummy,
	[AC_HELP_STRING(
		[--enable-sine-dummy],
		[enchanged dummy driver]
	)]
)
AC_MSG_CHECKING(whether enchange the dummy driver)
if test "$enable_sine_dummy" = "yes";
then
	AC_DEFINE(USE_SINE_TESTING_DUMMY,, [enchanged dummy driver])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi


# SNMP driver workaround
AC_ARG_ENABLE(snmp-descr,
	[AC_HELP_STRING(
		[--enable-snmp-descr],
		[use ifDescr instead of ifName for snmp devices]
	)]
)
AC_MSG_CHECKING(whether generic_snmp should use ifDescr instead of ifName)
if test "$enable_snmp_descr" = "yes";
then
	AC_DEFINE(USE_GENERIC_SNMP_DESCR,, [use ifDescr instead of ifName])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi


# display modes selection
AC_ARG_WITH(display-modes,
	[AC_HELP_STRING(
		[--with-display-modes="..."],
		[manually select visual modes. defaults to all]
	)],,
	[with_display_modes="traditional mgraph waveform wmwave wmnet sepgraphs twisted charts needle lines"]
)
for mode in $with_display_modes;
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
AC_MSG_CHECKING([for inline optimization])
AC_TRY_COMPILE(, [
  #undef INLINE
  #if !(defined(__STRICT_ANSI__) || defined(__cplusplus))
  #define INLINE inline
  #endif

  INLINE void
  aFunc()
  {} 
], inlineDef="inline")
if test -z "$inlineDef"
then
	# try again with __inline
	AC_TRY_COMPILE(, [
	  #undef INLINE
	  #if !(defined(__STRICT_ANSI__) || defined(__cplusplus))
	  #define INLINE __inline
	  #endif
	  
	  INLINE void
	  aFunc()
	  {} 
	], inlineDef="__inline")
fi
AC_DEFINE_UNQUOTED(INLINE, $inlineDef, [inline useability])

if test -n "$inlineDef"
then
	AC_MSG_RESULT([$inlineDef])
else
	AC_MSG_RESULT(no)
fi


# check for basic type sizes
AC_SIZE_CHECK(unsigned int)
AC_SIZE_CHECK(unsigned long)

# on all systems char is an 8 bits type
AC_DEFINE([__u8], [unsigned char], [basic unsigned 8 bits type])

# on recent HW all int types (int, long) are actually 32 bits types. On old
# 16bit CPUs this may not be true. Also, I should consider adding a check for
# unsigned short, just for those future 128 bits CPUs :)).
AC_SIZE_SEARCH(__u32, 32, ["unsigned int" "unsigned long"])


# output files
AC_CONFIG_FILES([
  Makefile
  doc/Makefile
  examples/Makefile
  src/Makefile
  wmnd.lsm
  wmnd.spec
])

AC_OUTPUT

AC_MSG_NOTICE([enabled drivers:$drivers])
AC_MSG_NOTICE([enabled display modes:$dspmodes])

