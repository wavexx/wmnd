# search for an integral type of a specified size
# usage: AC_SIZE_SEARCH(typedef, bytes, [type type ...])
# each type must first be checked with AC_CHECK_SIZEOF
AC_DEFUN([AC_SIZE_SEARCH],
[
	AC_MSG_CHECKING([for a $2 bytes type])
	unset xtype
	for type in $3
	do
		bytes="ac_cv_sizeof_`echo \"$type\" | tr \" \" \"_\"`"
		bytes="`eval echo \\$$bytes`"
		if test -n "$bytes" -a "$bytes" = "$2"
		then
			xtype="$type"
			break
		fi
	done

	if test -n "$xtype"
	then
		AC_DEFINE_UNQUOTED([$1], $xtype, [$2 bytes type])
		AC_MSG_RESULT([$xtype])
	else
		AC_MSG_ERROR([unable to find any suitable type])
	fi
	unset IFS
])

# creates a new switch (--enable-) with and help string which controls
# the definition of a conditional.
# usage: AC_ARG_EC(command name, variable, define, help string, check string)
AC_DEFUN([AC_ARG_EC],
[
	AC_ARG_ENABLE([$1], [AC_HELP_STRING([--enable-$1], [$4])])
	AC_MSG_CHECKING([$5])
	if test "$enable_$2" = "yes"
	then
		AC_DEFINE([$3],, [$4])
		AC_MSG_RESULT(yes)
	else
		AC_MSG_RESULT(no)
	fi
])
