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

# check for a size type
# note how I use the returning value of AC_RUN_IFELSE(ac_status),
# as there's NO other way of capturing a test's output.
AC_DEFUN(AC_SIZE_CHECK,
[
	AC_MSG_CHECKING([for "$1" size])
	AC_RUN_IFELSE(
	[
	  int main() {return (sizeof($1) * 8);}
	], [AC_MSG_RESULT([indeterminate])],
		[translit($1, ' ', '_')_size="$ac_status"
		AC_MSG_RESULT([$ac_status bits])]
	)
])

# search for an integral type of a specified size
# usage: AC_SIZE_SEARCH(typedef, bits, [type type ...])
# each type must first be checked with AC_SIZE_CHECK
AC_DEFUN(AC_SIZE_SEARCH,
[
	AC_MSG_CHECKING([for a $2 bits type])
	unset xtype
	for type in $3
	do
		bits="`echo \"$type\" | tr \" \" \"_\"`_size"
		bits="`eval echo \\$$bits`"
		if test -n "$bits" -a "$bits" = "$2"
		then
			xtype="$type"
			break
		fi
	done

	if test -n "$xtype"
	then
		AC_DEFINE_UNQUOTED([$1], $xtype, [$2 bits type])
		AC_MSG_RESULT([$xtype])
	else
		AC_MSG_ERROR([unable to find any suitable type])
	fi
	unset IFS
])

