#!/bin/sh
#
# Determine the OS and echo the short name to stdout.
# Not a lot here to keep the variants short.
#

PATH=/bin:/usr/bin
VERSION_ID=0

if [ -r /etc/os-release ] || [ -L /etc/os-release ]; then
    # shellcheck disable=SC1091
    . /etc/os-release
fi

# Crux Linux lacks /etc/os-release or an /etc/*-release file
grep -q ^CRUX /etc/issue >/dev/null 2>&1
IS_CRUX=$?

if [ -r /etc/fedora-release ] && [ "${ID}" = "fedora" ]; then
    if grep -q -i rawhide /etc/fedora-release >/dev/null 2>&1 ; then
        echo "${ID}-rawhide"
    else
        echo "${ID}"
    fi
elif [ -r /etc/centos-release ] && [ "${ID}" = "centos" ]; then
    case "${VERSION_ID}" in
        7|8|9)
            echo "${ID}${VERSION_ID}"
            ;;
        *)
            echo "unknown OS: ${ID}:${VERSION_ID}" >&2
            ;;
    esac
elif [ -r /etc/redhat-release ] && [ "${ID}" = "rhel" ]; then
    v="$(echo "${VERSION_ID}" | cut -d '.' -f 1)"
    case "${v}" in
        7|8)
            echo "${ID}${v}"
            ;;
        *)
            echo "unknown OS: ${ID}:${v}" >&2
            ;;
    esac
elif [ -r /etc/almalinux-release ] && [ "${ID}" = "almalinux" ]; then
    v="$(echo "${VERSION_ID}" | cut -d '.' -f 1)"
    case "${v}" in
        8|9)
            echo "${ID}${v}"
            ;;
        *)
            echo "unknown OS: ${ID}:${v}" >&2
            ;;
    esac
elif [ -r /etc/rocky-release ] && [ "${ID}" = "rocky" ]; then
    v="$(echo "${VERSION_ID}" | cut -d '.' -f 1)"
    if [ "${v}" = "8" ]; then
        echo "${ID}${v}"
    else
        echo "unknown OS: ${ID}:${v}" >&2
    fi
elif [ ${IS_CRUX} -eq 0 ] && [ -f /etc/pkgadd.conf ] && [ -f /etc/pkgmk.conf ]; then
    echo "crux"
elif [ -r /etc/oracle-release ] && [ "${ID}" = "ol" ]; then
    v="$(echo "${VERSION_ID}" | cut -d '.' -f 1)"
    if [ "${v}" = "8" ]; then
        echo "oraclelinux${v}"
    else
        echo "unknown OS: ${ID}:${VERSION_ID}" >&2
    fi
else
    case "${ID}" in
        opensuse-leap|opensuse-tumbleweed|ubuntu|debian|slackware|arch|gentoo|alpine|mageia|freebsd)
            echo "${ID}"
            ;;
        amzn)
            if [ "${VERSION_ID}" = "2" ]; then
                echo "${ID}${VERSION_ID}"
            else
                echo "unknown OS: ${ID}:${VERSION_ID}" >&2
            fi
            ;;
        *)
            echo "unknown OS: ${ID}" >&2
            ;;
    esac
fi
