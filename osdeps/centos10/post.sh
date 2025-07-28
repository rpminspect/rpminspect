#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin

# Update clamav database
chown clamupdate:clamupdate /var/lib/clamav
freshclam
