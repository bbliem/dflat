#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([sharp], [1.0], [morak@dbai.tuwien.ac.at])
AM_INIT_AUTOMAKE([sharp], [1.0])
LT_PREREQ([2.2])
LT_INIT([dlopen])
AC_CONFIG_SRCDIR([src/Problem.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([Makefile
		 include/Makefile
		 src/Makefile])

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S

# Checks for libraries.
AC_ARG_WITH([gmp],
	    [AS_HELP_STRING([--without-gmp],
			    [disable support for GNU MP])
	    ],
	    [],
	    [with_gmp=yes])
          
AS_IF([test "x$with_gmp" != xno],
      [
       AC_CHECK_LIB([gmp], [__gmpz_init], [], [AC_MSG_FAILURE([GNU MP library not found, see http://gmplib.org/ (--without-gmp to disable)])])
       AC_CHECK_HEADER([gmp.h], [], [AC_MSG_FAILURE([GNU MP header file not found, see http://gmplib.org/ (--without-gmp to disable)])])
       AC_LANG_PUSH(C++)
        AC_CHECK_HEADER([gmpxx.h], [], [AC_MSG_FAILURE([GNU MP C++ header file not found, see http://gmplib.org/ (--without-gmp to disable)])])
       AC_LANG_POP
      ])

# Checks for C header files.
AC_CHECK_HEADER([sys/resource.h], [], [AC_MSG_FAILURE([sys/resource.h header file not found])])

# Checks for C++ header files

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL

# Checks for library functions.
AC_CHECK_FUNCS([strerror])

# Define optional features
AC_ARG_ENABLE([debug],
	      [AS_HELP_STRING([--enable-debug],
			     [enable debug features])
	      ],
	      [AC_DEFINE([SHARP_ENABLE_DEBUG], [1], [Define if debug mode is enabled])])

# Pass information to automake
AM_CONDITIONAL([USE_LIBGMP], [test "x$with_gmp" = xyes])

AC_OUTPUT
