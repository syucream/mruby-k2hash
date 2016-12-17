#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([mruby-k2hash], [0.0.1], [syucream1031@gmail.com])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([src/mruby_k2hash.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX

# k2hash root
K2HASH_PREFIX=./k2hash
AC_ARG_WITH(k2hash_prefix, AC_HELP_STRING([--with-k2hash-prefix=DIR],
    [pathname to k2hash_prefix [[k2hash_prefix]]]),
    [K2HASH_PREFIX="$with_k2hash_prefix"])
AC_SUBST(K2HASH_PREFIX)

# mruby root
MRUBY_PREFIX=`pwd`/mruby
AC_ARG_WITH(mruby_prefix, AC_HELP_STRING([--with-mruby-prefix=DIR],
    [pathname mruby_prefix [[mruby_prefix]]]),
    [MRUBY_PREFIX="$with_mruby_prefix"])
AC_SUBST(MRUBY_PREFIX)

# Checks for libraries.
# AC_HAVE_LIBRARY([mruby])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
