# enable warnings for the selected compiler
AC_DEFUN(AC_CC_WARNINGS,
[
	# enable the --silent option
	AC_ARG_ENABLE(silent,
		[AC_HELP_STRING(
			[--enable-silent],
			[disable implicit warnings]
		)]
	)

	if test "$enable_silent" != "yes"
	then
		AC_MSG_CHECKING([for warning flags])
		case "$CC" in
			gcc* | g++*)	ac_wflags="-Wall -Wno-format";;
			*)		AC_MSG_RESULT(unsupported compiler);;
		esac
		if test -n "$ac_wflags"
		then
			AC_MSG_RESULT([$ac_wflags])
			CFLAGS="$CFLAGS $ac_wflags"
		fi
	fi
])

# force supported compilers to be pedantic
AC_DEFUN(AC_CC_PEDANTIC,
[
	# enable the --pedantic option
	AC_ARG_ENABLE(pedantic,
		[AC_HELP_STRING(
			[--enable-pedantic],
			[enable pedantic flags]
		)]
	)

	if test "$enable_pedantic" = "yes"
	then
		AC_MSG_CHECKING([for pedantic flags])
		case "$CC" in
			gcc*)	ac_pflags="-pedantic";;
			*)	AC_MSG_RESULT(unsupported compiler);;
		esac
		if test -z "$ac_pflags"
		then
			AC_MSG_RESULT([none])
		else
			AC_MSG_RESULT([$ac_pflags])
			CFLAGS="$CFLAGS $ac_pflags"
		fi
	fi
])

# enable production releases by default
AC_DEFUN(AC_CC_DEBUG,
[
	# enable the --debug option
	AC_ARG_ENABLE(debug,
		[AC_HELP_STRING(
			[--enable-debug],
			[enable debugging flags]
		)]
	)

	if test "$enable_debug" != "yes"
	then
		CPPFLAGS="$CPPFLAGS -DNDEBUG"
	fi
])

