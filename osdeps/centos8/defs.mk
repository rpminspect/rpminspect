PKG_CMD = dnf --enablerepo=powertools install -y
PIP_CMD = env CRYPTOGRAPHY_DONT_BUILD_RUST=1 pip-3 install -I
