osdeps
======

The subdirectories map to different operating systems used in the
GitHub Actions for rpminspect.  The GitHub Actions are defined in the
.github/workflows/*.yml files at the top level of the source tree.

The GitHub Actions call:

    make instreqs

For each operating system's environment.  That make target does the
following:

* If there is a 'pre.sh' script for the OS, run it.  This is a shell
  script that runs in the target environment.  Use it for any set up
  required before installing packages.

* Using the operating system's software installer, install all of the
  items listed in the 'reqs.txt' file.  For Linux, this file lists
  package names.  One package name per line.  Order does not matter.
  The operating system software installer is in 'defs.mk' and is the
  **PKG_CMD** variable.

* Using pip, the Python modules listed in 'pip.txt' are installed
  using the **PIP_CMD** from the 'defs.mk file.

* If there is a 'post.sh' script for the OS, run it.  Use this shell
  script for any final environment setup that you were unable to
  accomplish by install packages or Python modules.  For some systems
  this means building and installing a build or test dependency by
  hand.

Please make sure the subdirectories here map to job names in the
'ci.yml' file for the GitHub Actions.
