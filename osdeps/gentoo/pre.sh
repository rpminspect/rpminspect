#!/bin/sh

# Enable Python support in things like rpm
echo "USE='caps python'" >> /etc/portage/make.conf

# Allow installation of "masked" packages
echo "app-shells/ksh ~amd64" >> /etc/portage/package.accept_keywords
echo "dev-util/gcovr ~amd64" >> /etc/portage/package.accept_keywords
echo "dev-util/libabigail ~amd64" >> /etc/portage/package.accept_keywords
