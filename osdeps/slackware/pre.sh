#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
[ -d "${OSDEPS}" ] || exit 1
SLACKPKG="slackpkg -default_answer=yes -batch=on"

# Install everything from the distribution first
# (don't run 'clean-system' because that removes sbopkg)
${SLACKPKG} update
${SLACKPKG} install-new
${SLACKPKG} upgrade-all

# These packages are from Slackware but need manually installation or
# reinstallation
${SLACKPKG} install expat
${SLACKPKG} install gettext
${SLACKPKG} reinstall elfutils
${SLACKPKG} install kmod
${SLACKPKG} install libyaml
${SLACKPKG} install python
${SLACKPKG} install mozilla-nss
${SLACKPKG} install popt
${SLACKPKG} install db48
${SLACKPKG} install sqlite
${SLACKPKG} install readline
${SLACKPKG} install libcroco
${SLACKPKG} install acl

# Update the sbopkg cache
sbopkg -r
