#!/bin/sh
#
# Build new release in Koji for Fedora and EPEL
#

PATH=/usr/bin
CWD="$(pwd)"
WRKDIR="$(mktemp -d)"

# Allow the calling environment to override the list of dist-git branches
if [ -z "${BRANCHES}" ]; then
    BRANCHES="master f31 epel7"
fi

cleanup() {
    rm -rf ${WRKDIR}
}

trap cleanup EXIT

# Verify specific tools are available
for tool in klist fedpkg ; do
    ${tool} >/dev/null 2>&1
    if [ $? -eq 127 ]; then
        echo "*** Missing '${tool}', perhaps 'yum install -y /usr/bin/${tool}'" >&2
        exit 1
    fi
done

# Need a tarball
if [ $# -eq 0 ]; then
    echo "*** Missing tarball of new release" >&2
    exit 1
fi

TARBALL="$(realpath $1)"

if [ ! -f ${TARBALL} ]; then
    echo "*** $(basename ${TARBALL}) does not exist" >&2
    exit 1
fi

tar tf ${TARBALL} >/dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "*** $(basename ${TARBALL}) is not a tar archive" >&2
    exit 1
fi

# Need a krb5 ticket
klist >/dev/null 2>&1
if [ $? -eq 1 ]; then
    echo "*** You lack an active Kerberos ticket" >&2
    exit 1
fi

klist | grep -q "krbtgt/FEDORAPROJECT.ORG@FEDORAPROJECT.ORG" >/dev/null 2>&1
if [ $? -eq 1 ]; then
    echo "*** You need a FEDORAPROJECT.ORG Kerberos ticket" >&2
    exit 1
fi

cd ${CWD}
${CWD}/utils/mkrpmchangelog.sh > ${WRKDIR}/newchangelog

cd ${WRKDIR}
fedpkg co rpminspect
cd rpminspect

for branch in ${BRANCHES} ; do
    git clean -d -x -f

    # make sure we are on the right branch
    fedpkg switch-branch ${branch}

    # add the new source archive
    fedpkg new-sources ${TARBALL}

    # update the rolling %changelog in dist-git
    if [ -f changelog ]; then
        mv changelog cl
        cp ${WRKDIR}/newchangelog changelog
        echo >> changelog
        cat cl >> changelog
        rm -f cl
    else
        cp ${WRKDIR}/newchangelog changelog
    fi

    # copy in the new spec file, but replace the %changelog
    cat ${CWD}/rpminspect.spec > rpminspect.spec
    sed -i '/^%changelog/,$d' rpminspect.spec
    echo "%changelog" >> rpminspect.spec
    cat changelog >> rpminspect.spec

    # commit changes
    git add sources changelog rpminspect.spec
    fedpkg ci -c -p
    git clean -d -x -f

    # build
    fedpkg build --nowait
done
