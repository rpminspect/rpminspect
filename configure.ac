AC_PREREQ([2.69])
AC_INIT([rpminspect], [1.0], [dcantrell@redhat.com])
AM_INIT_AUTOMAKE([foreign])
AC_USE_SYSTEM_EXTENSIONS
AC_CONFIG_SRCDIR([src/rpminspect/rpminspect.c])
AC_CONFIG_HEADERS([config.h])
LT_INIT
AC_CONFIG_MACRO_DIRS([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_MAKE_SET

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h stddef.h stdint.h stdlib.h string.h sys/ioctl.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_INT64_T
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([getcwd memset mkdir munmap realpath regcomp rmdir strcasecmp strchr strdup strerror strstr])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/librpminspect/Makefile])

AC_OUTPUT
