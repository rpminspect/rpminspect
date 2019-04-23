AC_PREREQ([2.69])
AC_INIT([rpminspect], [1.0], [dcantrell@redhat.com])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([src/rpminspect/rpminspect.c])
AC_CONFIG_HEADERS([config.h])
AC_USE_SYSTEM_EXTENSIONS

LT_INIT
AC_CONFIG_MACRO_DIRS([m4])

# Generate code coverage targets
AX_CODE_COVERAGE

# Add valgrind check targets
AX_VALGRIND_DFLT([sgcheck], [off])
AX_VALGRIND_CHECK

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_MAKE_SET

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

# Checks for libraries.
PKG_CHECK_MODULES(JSON_C, [json-c])
PKG_CHECK_MODULES(XMLRPC, [xmlrpc xmlrpc_util xmlrpc_client])
PKG_CHECK_MODULES(LIBXML, [libxml-2.0])
PKG_CHECK_MODULES(RPM, [rpm])
PKG_CHECK_MODULES(LIBARCHIVE, [libarchive])
PKG_CHECK_MODULES(LIBELF, [libelf])
PKG_CHECK_MODULES(LIBKMOD, [libkmod])
PKG_CHECK_MODULES(LIBCURL, [libcurl])
PKG_CHECK_MODULES(ZLIB, [zlib])

# Check for libmandoc
AC_SEARCH_LIBS([mparse_alloc], [mandoc], [found=1], [found=0], [$ZLIB_LIBS])
if test $found -eq 1; then
    LIBMANDOC_LIBS="-lmandoc"
    AC_SUBST([LIBMANDOC_LIBS])
else
    AC_MSG_ERROR([*** unable to find mparse_alloc() in libmandoc])
fi

# Check for libiniparser
AC_SEARCH_LIBS([iniparser_load], [iniparser], [found=1], [found=0], [$ZLIB_LIBS])
if test $found -eq 1; then
    INIPARSER_LIBS="-liniparser"
    AC_SUBST([INIPARSER_LIBS])
else
    AC_MSG_ERROR([*** unable to find iniparser_load() in libiniparser])
fi

# Check for dlopen
AC_SEARCH_LIBS([dlopen], [dl], [found=1], [found=0], [$ZLIB_LIBS])
if test $found -eq 1; then
    DL_LIBS="-ldl"
    AC_SUBST([DL_LIBS])
else
    AC_MSG_ERROR([*** unable to find dlopen() in libdl])
fi

# Check for CUnit -- needed for unit tests
PKG_CHECK_MODULES(CUNIT, [cunit], [found_cunit=1], [found_cunit=0])
if test $found_cunit -eq 0; then
    AC_MSG_WARN([WARNING: unable to find CUnit, disabling unit tests])
fi
AM_CONDITIONAL([HAVE_CUNIT], [test x$found_cunit = x1])

AC_SUBST([JSON_C_CFLAGS])
AC_SUBST([JSON_C_LIBS])
AC_SUBST([XMLRPC_CFLAGS])
AC_SUBST([XMLRPC_LIBS])
AC_SUBST([LIBXML_CFLAGS])
AC_SUBST([LIBXML_LIBS])
AC_SUBST([RPM_CFLAGS])
AC_SUBST([RPM_LIBS])
AC_SUBST([LIBARCHIVE_CFLAGS])
AC_SUBST([LIBARCHIVE_LIBS])
AC_SUBST([LIBELF_CFLAGS])
AC_SUBST([LIBELF_LIBS])
AC_SUBST([LIBKMOD_CFLAGS])
AC_SUBST([LIBKMOD_LIBS])
AC_SUBST([LIBCURL_CFLAGS])
AC_SUBST([LIBCURL_LIBS])
AC_SUBST([ZLIB_CFLAGS])
AC_SUBST([ZLIB_LIBS])
AC_SUBST([CUNIT_CFLAGS])
AC_SUBST([CUNIT_LIBS])

AC_CONFIG_FILES([Makefile
                 data/Makefile
                 data/setuid/Makefile
                 data/capabilities/Makefile
                 data/political-whitelist/Makefile
                 data/licenses/Makefile
                 data/abi-checking-whitelist/Makefile
                 data/version-whitelist/Makefile
                 src/Makefile
                 src/librpminspect/Makefile
                 src/rpminspect/Makefile
                 tests/Makefile
                 tests/librpminspect/Makefile])

AC_OUTPUT
