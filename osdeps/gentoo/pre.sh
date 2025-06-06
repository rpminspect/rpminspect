#!/bin/sh

# Enable Python support in things like rpm
echo 'USE="caps python lzma"' >> /etc/portage/make.conf

# Allow installation of "masked" packages
[ -d /etc/portage ] || mkdir -p /etc/portage
rm -rf /etc/portage/package.accept_keywords
{ echo "app-shells/ksh ~amd64";
  echo "dev-util/gcovr ~amd64";
  echo "dev-util/libabigail ~amd64";
} >> /etc/portage/package.accept_keywords
