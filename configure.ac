dnl # LICENSE TBD
dnl #
dnl # Copyright (c) 2023 Oracle and/or its affiliates.
dnl # Author: Tom Hromatka <tom.hromatka@oracle.com>
dnl #

AC_INIT([belayd],[0.0.0])

dnl #
dnl # autoconf configuration
dnl #
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([configure.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE(-Wall foreign subdir-objects)

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

dnl #
dnl # build flags
dnl #
AM_CPPFLAGS="-I\${top_srcdir}/include -I\${top_builddir}/include"
AM_CFLAGS="-Wall"
AM_LDFLAGS="-Wl,-z -Wl,relro"
AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_LDFLAGS])

dnl #
dnl # version information
dnl #
VERSION_MAJOR=$(echo ${VERSION} | cut -d'.' -f 1)
VERSION_MINOR=$(echo ${VERSION} | cut -d'.' -f 2)
VERSION_MICRO=$(echo ${VERSION} | cut -d'.' -f 3)
AC_SUBST([VERSION_MAJOR])
AC_SUBST([VERSION_MINOR])
AC_SUBST([VERSION_MICRO])

dnl #
dnl # Determine which C compiler to use
dnl #
AC_PROG_CC
AM_PROG_CC_C_O

dnl #
dnl # Dependencies
dnl #
AC_CHECK_LIB([json-c], [json_object_object_get_ex], [],
	     [AC_MSG_ERROR([please install json-c])])

AC_CHECK_HEADERS([json-c/json.h], [],
		 [AC_MSG_ERROR([please install the json-c development package])])

LT_INIT

dnl #
dnl # Enable/Disable code coverage
dnl #
AX_CODE_COVERAGE

dnl #
dnl # makefiles
dnl #
AC_CONFIG_FILES([
	Makefile
	doc/Makefile
	doc/examples/Makefile
	include/Makefile
	src/Makefile
	src/plugins/Makefile
	src/plugins/causes/Makefile
	tests/Makefile
])

dnl #
dnl # generate the configure and makefiles
dnl #
AC_OUTPUT
