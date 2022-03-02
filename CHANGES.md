Changes in rpminspect-1.9
-------------------------

General release and build process changes:
* Improve call with koji list-targets
* Skip GPG signing the source archives for Copr builds
* Use rpmspec to gather BuildRequires for Copr
* Allow 'python_program' meson configuration option
* Add 'copr-srpm' Makefile target

Config file or data/ file changes:
* Skip *.html files in the xml inspection
* Drop diff(1) command setting from generic.yaml

Changes to the GitHub Actions CI scripts and files:
* Fix pre.sh for Extra CI on Gentoo Linux
* Fix the CentOS 7 Extra CI job
* Small cleanups to the Extra CI definitions
* Fix post.sh for centos 7 Extra CI job
* Expand PATH in post.sh for centos7 Extra CI
* Make sure /usr/local/bin is in the centos7 Extra CI PATH
* Match the matrix.container name correctly in extra-ci.yml
* Support special RPM vendor handling on ALT Linux
* Fix ALT Linux job in Extra CI
* Drop Open Euler stuff from utils/determine-os.sh
* Patch source and build system for FreeBSD
* shellcheck fixes for osdeps post.sh scripts
* Break out the CI and Extra CI configs in to separate files
* "/etc/pkg" -> "/etc/pki" in some GitHub Actions
* Fix /usr/lib/rpm/*/macros modification for ALT Linux
* Pull CentOS images from quay.io, prepare for CentOS 9 Stream
* Add Slackware Linux 15.0 to the GitHub Actions collection
* Use -checkgpg=off with slackpkg initially on Slackware Linux
* More minor fixes to the Slackware Linux GitHub Action
* Just set GPGCHECK=off for the Slackware Linux GitHub Action
* Run 'slackpkg update' before install git and requirements
* Try to instruct slackpkg to, yes, import the GPG key
* Install libxdiff-devel in Fedora-derived GitHub Actions jobs
* Drop libxdiff-devel package installation
* Drop diffutils from all CI job reqs.txt files

rpminspect(1) changes:
* Prevent SIGSEGV when get_product_release() fails
* Default favor_release setting to 'newest'
* Default favor_release setting to 'newest'
* Fix two small memory leaks in rpminspect(1)
* Exit with code 3 if the named profile (-p) is not found
* Fix -Werror=use-after-free findings from gcc 12
* Fix get_product_release() for single build jobs
* Implement the -s/--suppress option in rpminspect
* Do not remove default or user-specified --workdir paths
* Pretend that TTY has 80 columns, if the real width is unknown

Documentation changes:
* Small tweaks to the RELEASE checklist
* Update usage examples in README.md
* Update usage.rst to reflect current inspections

General bug fix in the library or frontend program:
* Make sure is_elf() returns true for ELF archives (*.a)
* Match MIME type on Icons in the desktop inspection
* Report removed files at INFO level in rebase comparisons
* Handle single build runs in is_rebase()
* Handle the -w option correctly for fetch and non-fetch modes
* Code formatting
* Read ABI level blocks with "level N" or "level-N" names
* Ensure summary mode output works for a number of inspections
* Prevent SIGSEGV in rpminspect is the configuration is incomplete
* Prevent SIGSEGV when bad_functions is empty for -D
* Avoid stairstepping the text in summary output mode
* Make sure multiple package providers are collected in rpmdeps
* Correct the name of RPM weak dep macros in deprules.c
* Do not incorrectly report added files for single builds
* Cleaner error reporting when elf_version() returns EV_NONE
* Strip workdir from the Details in shellsyntax reports
* Some build comparisons with missing peers crash 'rpmdeps'
* Close a number of non-fatal memory leaks
* Remove unnecessary free() calls in librpminspect and rpminspect
* Match shared lib Requires correctly & handle multiple Provides
* Handle explicit shared lib deps with %{_isa} notation
* Handle packages that provide automatic shared lib deps
* Correct the addedfiles reporting messages
* Memory management fixes with trim_rich_dep() function
* Minor improvements to delta_out() in librpminspect
* Skip deprule version matching on NULL in expected_deprule_change()
* Change = -> == in an if expression
* Fix three small memory leaks originating in init.c
* Honor explicit Requires deps that use zero-epoch syntax
* Patch a number of non-fatal valgrind findings
* Do not assume peer is not NULL in set_peer()
* Minor librpm interaction fixes for the unicode inspection
* Variable initialization fixes for libxdiff
* YAML parsing error for the failure_severity setting
* Do not call rpmFreeMacros() in load_macros()

librpminspect feature or significant change:
* Expand the emptyrpm inspection to handle %ghost entries
* Replace direct use of "diagnostics" with NAME_DIAGNOSTICS
* Add new 'summary' output mode (#26)
* Final update for librpminspect inspections and summary mode
* Use the -o option on msgunfmt(1) in 'changedfiles'
* Default annocheck results to RESULT_INFO
* Guard capabilities and kmod stuff in inspect.c
* Add 'rpmdeps' inspection
* Support new config file section for 'rpmdeps'
* Add 'rpmdeps' inspection
* Completed 57 test cases for 'rpmdeps' for Requires dependencies
* Set 'addedfiles' to work for single and compare jobs
* Drop use_ignore parameter from foreach_peer_file()
* Remove legacy _FORTIFY_SOURCE check in the elf inspection
* Begin handling of rich dependency syntax in 'rpmdeps'
* Add is_rich_dep() to mark rich dependency strings in deprules
* Use libxdiff instead of relying on /usr/bin/diff
* Add libxdiff directly to the source tree
* Report annocheck failures as RESULT_VERIFY
* Make annocheck failure reporting severity a config file setting

Test suite commits:
* Support older versions of rpmfluff in baseclass.py
* Use the codecs module in test_unicode.py
* Create ProvidedSourceFile for use in test_unicode.py
* Set QA_RPATHS=63 to disable check-rpaths in rpmbuild
* Improve the debugging output for failing tests
* Add test cases for the rpmdeps inspection
* In rpmdeps_requires tests, expect OK and not INFO for two
* Ensure rpmdeps Requires test bins link with test lib
* Skip tests in test_rpmdeps_requires.py that need 'elfdeps'
* Fix a few small bugs in the Alpine Linux post.sh script
* Add six remaining test_rpmdeps_requires.py test cases
* Added 54 tests for Provides dependencies in 'rpmdeps'
* Added 54 tests for Conflicts dependencies in 'rpmdeps'
* Add remaining 270 test cases for the rpmdeps inspection
* Correct weak dep test cases for 'rpmdeps'
* Skip weak dependency rpmdeps test cases for older librpm versions
* Pass --nodeps to rpmbuild in the test suite
* Force the use of %attr for add_installed_directory()
* Skip unexpanded macro test cases on ALT Linux for rpmdeps
* Use '=' and not '>=' for Provides in rpmdeps test cases
* Skip most weak dependency tests on ALT Linux
* Account for ALT Linux including Epoch values in dependencies
* Remove old have_caps_support block from test_capabilities.py


Changes in rpminspect-1.8
-------------------------

General release and build process changes:
* Updates to how Koji jobs are prepared and submitted
* Add RELEASE checklist

Changes to the GitHub Actions CI scripts and files:
* Add 'make shellcheck' target and update ShellCheck CI job
* Install findutils for the shellcheck GitHub Action

Documentation changes:
* Add changes for the 1.7 release
* Note additional dependencies in README.md
* Reformat OS list in README.md

General bug fix in the library or frontend program:
* Handle unexecutable %prep sections in 'unicode'
* Remove hardcoded maximum version check on Java bytecode
* Correct everything shellcheck found in shell scripts
* Exit the manual_prep_source() child process correctly
* Use archive_read_free() in unpack.c

librpminspect feature or significant change:
* Convert RPM header cache to a hash table, fix leak

Test suite commits:
* Define SimpleSrpmBuild for tests that only need an SRPM
* Add unicode test cases covering rpmSpecBuild() failures


Changes in rpminspect-1.7
-------------------------

General release and build process changes:
* Update the spec file template
* Drop mkrpmchangelog.sh use for Copr builds
* Remove mkrpmchangelog.sh from the source tree
* Increment version to 1.7
* s/\.gz/.xz/ in .copr/Makefile
* Additional Copr build fixes
* Adjust %autosetup line for Copr builds
* Fix output formatting in utils/mkannounce.sh
* Add libicu-devel as a BuildRequires in the spec file
* Add some additional comments to the Makefile
* Add 'MIT' to the project license string

Config file or data/ file changes:
* Add a new 'macrofiles' section to the config file
* Quote forbidden Unicode code points in generic.yaml
* Comment fixes for the 'unicode' section in generic.yaml

Changes to the GitHub Actions CI scripts and files:
* Install the rust package on Alpine Linux
* Add 'cargo' to the reqs.txt list for Alpine Linux
* Install rustc and cargo on all systems now for Python cryptography
* Make sure Fedora CI instances have 'libicu-devel' installed
* Make sure ICU is installed for Extra CI targets
* Disable the Alt Linux job in Extra CI for now
* Enable Alt Linux, disable CentOS 7 in Extra CI
* Set LANG=en_US.UTF-8 when running the test suite
* icu -> icu-dev in reqs.txt for Alpine Linux
* Rename 'x86_64' job category to 'linux'

rpminspect(1) changes:
* Call load_macros() and rpmFreeMacros() from rpminspect

Documentation changes:
* Add CHANGES.md file summarizing changes per release

General bug fix in the library or frontend program:
* Fix small memory leak in macros.c
* Call rpmFreeMacros() after the disttag inspection runs
* TryExec= line parsing for .desktop files
* Remove unnecessary assert() on fname in match_fileinfo_mode
* Do not report 'OK' result in emptyrpm for expected empties
* addedfiles requires a before and after build
* Minor improvement to the text in REMEDY_ADDEDFILES
* Correctly match debuginfo trees to subpackages
* Simiply the copytree() function to remove malloc errors
* Remove unnecessary '()' from function names in error messages
* Allow exitcode parameter to be NULL in run_cmd_vpe()
* Non-zero exit from desktop-file-validate is an error
* Use realloc() instead of reallocarray()
* Remove incorrect free() call in inspect_unicode.c
* Set seen and globalresult in inspect_unicode.c
* Use a long int for linenum and colnum in inspect_unicode.c
* Comment clarification in include/constants.h
* Support rpm < 4.15.0 in the unicode inspection
* Fallback on fedora_name only if the _abbrev fields are empty

librpminspect feature or significant change:
* Support %autorelease and other macros in disttag inspection
* Check subpackages when running the 'desktop' inspection
* Support standard system icons in the 'desktop' inspection
* In 'annocheck', report the commands and exit codes
* Add list_add() to listfuncs() in librpminspect
* Use fork()/execvpe() in runcmd.c instead of popen()
* Modify inspect_annocheck.c to use run_cmd_vpe()
* Allow an optional subdirectory on run_cmd() and run_cmd_vpe()
* Use the new run_cmd_vpe() API in 'abidiff' and 'kmidiff'
* Update all run_cmd() calls to the new API
* Add load_macros() function to macros.c
* Remove code from inspect_disttag.c that's in load_macros()
* Introduce mime_type() function in magic.c
* Change UChar usage to UChar32
* Expand dump_cfg() for the -D debug mode output
* Use u_strchr32() when searching for forbidden code points
* Improve reporting in unicode with prep_source() fails

New inspections or inspection changes (not bug fixes):
* Add the 'unicode' inspection to check source code

Test suite commits:
* Collect subpackages when performing Koji build tests
* Remove musl special handling in test_abidiff.py
* Remove unused 'import subprocess' from test_abidiff.py
* Get the tearDown() functions all working correctly
* Begin test_unicode.py with unicode inspection tests
* Complete test_unicode.py and use different example code
* Add additional known bad unicode test cases


Changes in rpminspect-1.6
-------------------------

General release and build process changes:
* Expand determine-os.sh to detect Crux Linux and Alt Linux
* Add %find_lang to the package spec file

Changes to the GitHub Actions CI scripts and files:
* Support older libraries on CentOS 7
* Define OPENSSL_VERSION to 0 if it's undefined
* Initial set of changes for Alpine Linux
* Python black formatting fixes
* Install epel-release in pre.sh on centos7
* Pass CRYPTOGRAPHY_DONT_BUILD_RUST=1 to pip on centos7
* Add AlmaLinux 8 to the GitHub Actions extra-ci job
* Restore previous centos7 pip and setuptools behavior
* Add almalinux handling to extra-ci.yml
* s/dnf/yum/g in pre.sh for centos7
* Add an i386 job to extra-ci.yml
* Try using i386 command for the i386 jobs
* Install util-linux for /usr/bin/i386
* Just handle i386 build in the qemu job
* Fix 32-bit builds and add Fedora i686 CI targets
* OS_SUBDIR clean up in the Makefile
* Fixed typo in the Makefile
* Need libffi-devel even for i686 builds
* Set 32-bit build flags in ci.yml
* Fix installing libffi-devel on i686 jobs
* Add Rocky Linux 8 to the Extra CI job collection
* Fix name of Rocky Linux Docker image
* Add missing Amazon Linux CI files
* Amazon Linux lacks glibc-devel.i686, so disable some tests
* Python flake8 and black formatting fixes
* Install tar before git task on amzn
* Correct Amazon Linux name in extra-ci.yml
* Add Mageia Linux to Extra CI
* Must use dnf on Mageia Linux, not yum
* clamav-dd -> clamav-db on Mageia Linux
* Ignore top level docs in CI jobs
* Add Alt Linux coverage to Extra CI
* Remove /usr/lib/rpm/shell.req on Alt Linux
* Remove forced RPMTAG_VENDOR value on Alt Linux
* Install gcovr via pip for Alt Linux
* Add Oracle Linux 8 to the extra-ci collection
* Fix syntax error in utils/determine-os.sh
* Output "oraclelinux" instead of ${ID} for Oracle Linux
* Disable fedora:rawhide in ci.yml
* Support Alt Linux p10 (platform 10)
* Add openEuler Linux 20.03 to the extra-ci collection
* Fix openEuler detection in utils/determine-os.sh
* ShellCheck fixes for determine-os.sh
* Run extra-ci jobs for changes to utils/ files
* Disable openEuler 2.0 in extra-ci

rpminspect(1) changes:
* Handle SIGWINCH in the download progress bar display
* Use sigaction() instead of signal() in rpminspect(1)
* Discontinue realpath() call on argv[0
* Honor -T/-E correctly even with security-focused checks
* Fix the progress bar display problems for '-f -v' mode.

Documentation changes:
* Update the AUTHORS.md file
* Drop mention of LibreSSL from README.md
* Add readthedocs rst source files
* Update translation template
* Link to generic.yaml from configuration.rst
* Mention readthedocs.io in README.md
* Update list of CI Linux distributions in README.md

General bug fix in the library or frontend program:
* Only look at RPMTAG_SOURCE entries for removed sources
* Remove unnecessary empty list check in inspect_upstream.c
* Ignore noarch packages in the 'arch' inspection
* Drop all HEADER_* defines, switch to NAME_* (#397)
* desktop: demote as INFO a missing Exec w/ TryExec (#395)
* Add 'types' block support for the config file (#404)
* Check return code of yaml_scan_parser()
* Use a simpler and correct regexp for the disttag inspection (#412)
* Translate some additional warning messages
* Use cap_compare() when comparing file capabilities (#410)
* Skip .spec files in the types inspection
* Description and Summary changes are reported as INFO
* Fall back on full license name if there are no abbrevs
* Follow-on to the license inspection changes for fedora_name
* Ignore "complex" spec file macros in get_macros()
* Fix a lot of xmlrpc-c memory leaks in builds.c
* Support the legacy libcurl API in librpminspect
* Fix SIGSEGV caused by misplaced xmlrpc_DECREF() call
* Adjust where the 'good' bool is set in the emptyrpm loop
* Slight code reformatting in inspect_disttag.c
* Read file capabilities from the RPM header
* Size origin_matches at 3 rather than 1.
* Non well-formed XML fails xml, but invalid is info
* Handle DT_RUNPATH/DT_RPATH owned directories correctly
* Convert the security rules from a hash table to a list
* Ensure the annocheck inspection behaves for build comparisons

librpminspect feature or significant change:
* Add list_contains() to librpminspect
* Skip source packages in 'emptyrpm'
* desktop: factor check for Exec
* desktop: factor check for Icon
* desktop: reset severity/waiverauth before add_result()
* Add debugging output to the YAML config parsing code
* Support relative ignore globs (#404)
* Report new patches at the INFO level only
* Allow directories owned by the build in 'runpath'
* Always run inspections with possible security results
* Define security_t and secrule_t in librpminspect
* Add strshorten() to strfuncs.c
* Add libcurl download progress bars for 'rpminspect -v'
* Document the escape sequences used for the progress bar
* Replace get_header_value() with get_rpm_header_value()
* Remove get_cap() function and fix test_ownership.py
* Add security rule reading code to librpminspect
* Removed RESULT_WAIVED from severity_t
* Improve remedy reporting to tell users what data file to edit
* Improve permission and ownership reporting strings w/ fileinfo
* Properly override config file blocks in subsequent reads
* Move match_path() and ignore_path() to paths.c
* Set OPENSSL_API_COMPAT in lib/checksums.c
* Add product security workflow functions and test cases
* Product security workflow enhancement for SECRULE_SECURITYPATH
* Product security workflow handling for SECRULE_MODES
* Product security workflow handling for SECRULE_CAPS
* Product security workflow handling for SECRULE_SETUID
* Product security workflow handling for SECRULE_WORLDWRITABLE
* Product security workflow handling for SECRULE_EXECSTACK
* Product security workflow handling for inspect_elf.c
* Do not warn if RPM header is missing a tag
* Suppress debug output for the config file parsing by default

New inspections or inspection changes (not bug fixes):
* Output new 'diagnostics' section in rpminspect report (#280)

Test suite commits:
* Python black fixes for baseclass.py
* Python black fixes for baseclass.py
* Update unit tests for removal of RESULT_WAIVED
* Modify the 'ownership' tests for old rpm releases and non-root
* Increase test suite timeout to 900
* Support really old versions of RPM (4.0.4) and Alt Linux


Changes in rpminspect-1.5
-------------------------

General release and build process changes:
* Generate regular changelog in utils/srpm.h
* Skip branches without targets in submit-koji-builds.sh
* Simplify the utils/determine-os.sh script
* Fix $(OS) check in the Makefile
* BuildRequires libmandoc-devel >= 1.14.5

Config file or data/ file changes:
* Add commented out per-inspection ignore blocks
* Note all regular expression settings use regex(7) syntax
* Note size_threshold can be the keyword 'info'

Changes to the GitHub Actions CI scripts and files:
* Fedora and CentOS systems in ci need 'diffstat'
* opensuse-leap CI job requires 'diffstat'
* Fix the Debian CI jobs in GitHub Actions
* Fix and enable the Ubuntu extra-ci job in GitHub Actions
* Use 'pip' instead of 'pip3' for the Ubuntu command
* Use 'apt-get -y install' in ubuntu's pre.sh
* Enable the opensuse-tumbleweed GHA job again
* Make sure the Gentoo GHA job has 'diffstat'
* Get the Arch Linux GHA job working again
* Use ubuntu:latest for the ubuntu GHA image
* Fix the ubuntu GitHub Actions extra-ci job
* Make sure the centos8 job has git available before cloning
* Install cpp-coveralls using pacman on Arch Linux
* Install cpp-coveralls using pip on Arch Linux
* Install cpp-coveralls in pre.sh on Arch Linux
* Install required Python modules in pre.sh on Arch Linux
* Do not upgrade pip on Arch Linux, go back to using pip.txt
* Do not run 'apt-get update' as a second time on Debians systems
* Update the OpenSUSE Tumbleweed files, but disable it anyway
* Manually install mandoc on centos7 for now

rpminspect(1) changes:
* Allow any number of builds specified for fetch only mode
* Fix fetch only mode download directory
* Do not crash with the -c option specifies a non-existent file
* Remove what working directories we can

Documentation changes:
* Update license table in README.md
* Update GitHub Action status badges in README.md
* Update TODO list

General bug fix in the library or frontend program:
* Use llabs() instead of labs() in the filesize inspection
* Improve 'has invalid execstack flags' reporting
* Use long unsigned int to report size changes in 'patches'
* Fix some errors in the changedfiles inspection
* Check DT_SONAME in is_elf_shared_library()
* Skip debuginfo and debugsource files in abidiff
* Report INFO level for patches findings by default
* Handle old or broken versions of libmagic in 'changedfiles'
* Use json_tokener_parse_ex() to get better error reporting
* Fix reading of the javabytecode block in the config file
* Catch missing/losing -fPIC correctly on .a ELF objects (#352)
* Refactor elf_archive_tests() and its helper functions
* Followup fix for find_no_pic, find_pic, and find_all
* Drop DEBUG_PRINT from source generated by pic_bits.sh
* Clean up the config file section reading code
* Perform symbolic owner and group matching in 'ownership' (#364)
* Restrict download_progress() to systems with CURLOPT_XFERINFOFUNCTION
* Report annocheck failures correctly in librpminspect.
* Call mparse_reset() before mparse_readfd()
* Ensure ctxt->lastError.message is not NULL before strdup (#382)
* Handle corrupt compressed files in 'changedfiles' (#382)
* Correctly find icons for desktop files in subpackages (#367)
* Followup to the Icon= check in the desktop inspection (#367)

librpminspect feature or significant change:
* Change strappend() to work as a variadic function
* Define inspection_ignores in struct rpminspect
* Add add_ignore() to init.c
* Stub out libcurl download progress callback function
* Read per-inspection ignore lists from the config file.
* Implement per-inspection path ignore support (#351)
* Allow 'size_threshold: info' in the config file (#261)
* Check ignore list in 'files' for path prefixes to ignore (#360)
* Support a list of expected empty RPMs in the config file (#355)
* Disable debugging output for the ignore lists in init.c
* Drop debugging output in the 'xml' inspection

Test suite commits:
* Update the 'changedfiles' test cases
* Make sure abidiff test cases add a DT_SONAME to the test lib
* Update the test/test_patches.py cases for patches changes
* The lost PIC tests need to invoke gcc with -fno-PIC
* Make sure brp-compress is disabled in test_manpage.py


Changes in rpminspect-1.4
-------------------------

General release and build process changes:
* Trim git commit summary prefix from changelog lines
* Recommend or Require libabigail >= 1.8.2
* Enable werror=true and warning_level=3 in default_options
* Improve mkannounce.sh to handle stable and devel releases

Changes to the GitHub Actions CI scripts and files:
* Restrict style checks to specific directories
* Adjust lib/meson.build for Fedora rawhide
* Drop gate.yml and begin non-x86_64 arches in ci.yml
* Add armv7, aarch64, and s390x to the other_arches job
* Modify file triggers and matrix use in ci.yml
* Split 32-bit osdeps out to post.sh scripts in osdeps/
* Split style.yml in to shellcheck.yml and python.yml
* Rewrite extra-ci.yml to use the matrix strategy method for GHA
* Move the emulated CI jobs to extra-ci.yml
* s/pkg/pki/ for the centos jobs
* Debian and Ubuntu fixes for CI
* Python pip on Debian is called 'pip'
* Try to fix just debian:stable
* Enable debian:testing in extra-ci.yml
* Enable centos8 in extra-ci.yml again
* Enable centos7 in extra-ci.yml again
* Enable opensuse-leap and gentoo in extra-ci.yml again
* Run each test script individually on emulated targets
* Try a different syntax for the emulated matrix jobs
* Install s390 glibc headers on s390x fedora systems
* Add stretch and buster to the emulated targets list
* Drop Debian buster from the emulated targets
* Install gcc-multilib only on Debian x86_64 and s390x systems
* libc-dev:i386 -> libc6-dev:i386
* Disable Debian targets in extra-ci temporarily
* s/AUR/git/g in osdeps/arch/post.sh

Documentation changes:
* Update TEST_METADATA status in TODO and MISSING

General bug fix in the library or frontend program:
* Handle compressed but otherwise empty man pages (#308)
* Correct misuse of 'entry' with 'hentry' variables (#321)
* Use hentry->key over hentry->value in 'pathmigration'
* Change init.c error reporting over to err/warn functions
* In read_cfgfile(), keep track of block depth correctly (#329)
* A few more yaml parsing fixes for block vs group
* Report libclamav version and CVD versions (#258)
* Ensure first argument of warn(3) is a format string.
* Get rid of invalid free() in get_product_release()

librpminspect feature or significant change:
* Migrate more code off hsearch and to uthash
* Replace hsearch() with uthash in the kmod inspection
* Replace hsearch() with uthash in the abidiff inspection
* Change tsearch/twalk use to uthash
* Finish normalizing all the error reporting statements
* Add inspection_id() to librpminspect

New inspections or inspection changes (not bug fixes):
* Report the program version number in the results (#309)
* Disable broken ELF heurisitic and size limit in libclamav
* Modify dump_cfg() to write valid YAML to stdout (#306)

Test suite commits:
* Flake8 fixes for test_manpage.py
* s/self.rpm/self.after_rpm/ in two test_manpage.py tests
* Detect 32-bit and musl presence in test_elf.py
* Skip lost -fPIC tests if gcc lacks -m32 support


Changes in rpminspect-1.3.1
---------------------------

Documentation changes:
* Update translation template

General bug fix in the library or frontend program:
* Remove duplicate elf_end() call in init_elf_data() (#303)


Changes in rpminspect-1.3
-------------------------

General release and build process changes:
* Move the master branch to version 1.3
* Change 'Suggests' to 'Recommends' in the spec file
* Create mkannounce.sh to help make release announcements easier
* shellcheck fixes for mkannounce.sh

Config file or data/ file changes:
* Add 'kmidiff' and 'politics' to the inspections section of generic.yaml

Changes to the GitHub Actions CI scripts and files:
* Build and install 'rc' from source on opensuse-leap
* Add .github/ and osdeps/ directories to extra-ci.yml
* Build and install 'rc' from source on opensuse-tumbleweed
* Adjust curl(1) command line used for 'rc' in opensuse CI jobs
* s/PowerTools/powertools/g in the centos8 PKG_CMD definition
* The output of html2text on opensuse systems is different
* Fixes for GitHub Actions on Debian and Ubuntu
* Disable Rust support in pip modules, more extra-ci fixes
* Update pip and setuptools on debian and ubuntu CI jobs
* Make sure pip is updated on debian, centos7, and centos8
* Fixes for extra-ci on arch, centos7, centos8, and debian
* Adjust docker image names for opensuse and arch
* Add Gentoo Linux to the Extra CI set
* extra-ci.yml typo fix for the gentoo job
* Use gentoo/stage3 as the container for the gentoo CI job
* Disable opensuse-tumbleweed and archlinux CI jobs
* Make sure util/determine-os.sh picks up Gentoo Linux
* Use 'pip install' for PIP_CMD on gentoo
* Set PIP_CMD to 'pip install -user' for gentoo
* Stop doing an 'emerge --sync' on the gentoo CI job
* Replace 'emerge --sync' with a manual portage sync
* Use 'emerge-webrsync' to update portage on gentoo

rpminspect(1) changes:
* Support the new output function call syntax (#264)

Documentation changes:
* Add doc/git.md to explain source control conventions
* Update doc/git.md on how to track upstream
* Update TODO and README.md files
* Typo fix in README.md

General bug fix in the library or frontend program:
* Use warn() for non-fatal errors in mkdirp()
* Swap out some more fprintf()/fflush() reporting with warn()
* #include <err.h> in peers.c and rmtree.c
* Correctly handle the -w option on rpminspect(1) (#256)
* Drop the relative path handling for the '-w' option
* Skip debug packages in 'filesize', display changes correctly
* Fix YAML config file reading for BLOCK_INSPECTIONS
* Expand dump_config() to cover all config file settings
* Simplify list_to_string() so it handles 1-elements lists right
* Output the system-out xunit portion as CDATA (#264)
* Slightly change how strxmlescape() works
* Fix block handling problems in the YAML config reader
* Note single builds cannot be rebases in is_rebase()
* Handle a NULL from list_to_string() in abspath()
* Fix some memory leaks found by valgrind

librpminspect feature or significant change:
* Require libabigail >= 1.8 in rpminspect.spec.in
* Enable multiple --headers-dir1 and --headers-dir2 args in 'abidiff'
* Rename HEADER_MAN to HEADER_MANPAGE (#264)
* Add inspection_header_to_desc() to librpminspect (#264)
* Move init_elf_data() to readelf.c, move data to struct rpminspect
* Remove check_ipv6() from inspect_elf.c
* Add abspath() to canonicalize path strings
* Add strxmlescape() to strfuncs.c in librpminspect (#264)
* Handle the empty string case in abspath()
* Expand dump_cfg() to show 'runpath' settings
* Hook up the driver for the 'runpath' inspection
* Add uthash and move the file matching code to it.

New inspections or inspection changes (not bug fixes):
* Add 'xunit' output format support (#264)
* Create the 'badfuncs' inspection
* Add the 'runpath' inspection to librpminspect
* In the runpath inspection, fail if DT_RPATH and DT_RUNPATH exist
* Do not match path prefixes in the runpath inspection

Test suite commits:
* Update inspect_elf.c unit tests for librpminspect changes
* Add 'badfuncs' test cases
* Rename test/data/lto.c to test/data/mathlib.c
* Install 'patchelf' for tests on fedora and centos
* Pass -D to rpminspect in the test suite
* Add integration tests for the 'runpath' inspection
* Python flake8 and black fixes in test_runpath.py
* Python black fixes for test_runpath.py


Changes in rpminspect-1.2
-------------------------

* For BUILDTYPE=release, generate the correct type of changelog
* Minor logic error in submit-koji-builds.sh
* Fix reading existing spec file in submit-koji-builds.sh
* Bump development build version to 1.2
* Use is_rebase() in the 'upstream' inspection
* Use rpmtdSetIndex() and rpmtdGetString() in get_header_value()
* Add get_rpmtag_fileflags() to files.c and call from extract_rpm()
* Use correct Version and Release values in download_build()
* #include <rpm/rpmfiles.h> -> #include <rpm/rpmfi.h>
* Add the 'config' inspection to librpminspect
* Rephrase reporting messages in the 'config' inspection
* Add the 'doc' inspection to librpminspect
* Update TODO list
* Minor updates to try and make gate.sh more reliable
* Add config and doc to the inspections list in generic.yaml
* Rename the '%files' inspection to 'files' (#194)
* Modify baseclass.py to allow 'before' and 'after' NVR tuples
* Use the after tuple to override the NVR in test_abidiff.py
* Use the after tuple to override the NVR in test_upstream.py
* Write rpminspect output to a file in the test suite
* Add 28 test cases for the 'config' inspection
* Fix the errors in the 'config' inspection found by the test suite
* Fix Python problems in the test suite reported by black and flake8
* Add Makefile targets for black and flake8
* One more formatting issue reporting by Python black in test_config.py
* More 'python black' formatting errors reported for test_config.py
* https://mandoc.bsd.lv -> http://mandoc.bsd.lv
* Add a -D/--dump-config option to rpminspect(1)
* Use global reported variable in 'config' inspection
* Fix reporting errors in the 'doc' inspection
* Add test_doc.py with 'doc' inspection test cases
* Ignore flake8 W291 in test_doc.py where we explicitly want whitepsace.
* Add init_rebaseable() to librpminspect
* Check the rebaseable list in is_rebase() in librpminspect
* Update TODO list
* Define a new GitHub Action using utils/gate.sh
* Update the README.md file
* shellcheck fixes for utils/gate.sh
* Use utils/find-ninja.sh to determine what ninja-build command to use
* Install fedora-packager for the gate.yml GitHub Action
* Remove before and after variables from gate.sh; unused
* Remove unnecessary basename() calls in inspect_upstream.c
* Do not assume an or bn contain strings in is_rebase() (#196)
* Adjust what things run during with GitHub Actions
* Add get_rpm_header_string_array() to librpminspect
* Replace init_source() with get_rpm_header_string_array() in inspect_upstream.c
* free() allocated output string in inspect_changelog.c on errors
* s/10240/16384/ in archive_read_open_filename() call in unpack.c
* Add the 'patches' inspection to librpminspect
* Add uncompress_file() to librpminspect
* Add filecmp() and use that in place of zcmp/bzcmp/xzcmp
* README.md updates
* Restrict some GitHub Actions to source code and test suite changes.
* Only enable lz4 compression if ARCHIVE_FILTER_LZ4 is defined
* Go ahead and wrap the rest of the libarchive compression filters
* s/class Test/class /g
* Make sure uncompress_file() supports xz compression
* Handle more compressed file MIME types.
* Add test_changedfiles.py to the test suite.
* Add test_patches.py with test cases for the 'patches' inspection
* flake8 fixes in the test suite
* Python format fixes for test_changedfiles.py
* Python format fixes in test_patches.py
* More Python format fixes for test_patches.py
* Remove unnecessary 'a' in DESC_PATCHES
* Better explanation as to why the EmptyLicenseTag tests are skipped.
* Test suite cleanup; add rebase= and same= to TestCompareSRPM
* Black formatting fixes for the test suite.
* Remove unused imports in test_upstream.py
* Revert black fixes for test_config.py
* Fix my email address in test suite source files.
* Support single package URLs for before and after builds (#190)
* Handle invalid/missing RPMs in get_product_release()
* Use warnx(), errx(), and err() in src/rpminspect.c
* Modify submit-koji-builds.sh to pick up all pkg-git branches.
* Update the rpminspect.1 man page to reflect current status.
* Update translation template files in po/
* Support relative directory paths for the -w option (#188)
* Implement the 'virus' inspection and add test cases for it.
* Update po/ template files
* Python formatting fixes for test_virus.py
* Update the osdeps/*/reqs.txt files.
* More osdeps updates for the clamav needs
* Install 'xz' for the 'style' GitHub Action
* Fix a variety of small memory leaks in librpminspect
* Stop the freshclam service for the Ubuntu gate job
* Support slightly older versions of libclamav in inspect_virus.c
* Add the 'politics' inspection to librpminspect.
* In tearDown() in the test suite, call rpmfluff clean() methods
* Add test_politics.py with 'politics' inspection test cases
* Python black format fixes for test_politics.py
* 'it should added' -> 'it should be added'
* Increase the runtime timeout for test_virus.py
* Install the timeout decorator on all OSes in our GitHub Actions
* Install timeout-decorator with pip, not timeout
* Expand librpminspect with support for SHA-224, SHA-384, and SHA-512
* Define DEFAULT_MESSAGE_DIGEST in constants.h and use that.
* Replace some fprintf()/fflush() calls with warn()/warnx() calls
* Rename the 'DT_NEEDED' inspection to 'dsodeps'
* Rename 'LTO' inspection to 'lto'
* Update translation template and fix two incorrect error strings.
* Note all valid message digests in data/politics/GENERIC
* Improve reporting in the patches inspection
* Only fail 'changedfiles' for VERIFY and higher results
* If 'removedfiles' only reports INFO messages, pass the inspection
* If 'addedfiles' only reports INFO results, pass the inspection
* If 'patches' only reports INFO results, pass the inspection
* No need to check value of allowed in permissions_driver()
* Do not let INFO results fail the 'doc' inspection.
* Do not let all INFO results in 'upstream' fail the inspection
* Fix RPMFILE_FLAGS handling for %config files (#221)
* Still report file changes in the 'config' inspection for rebases
* Correctly check RPMFILE_DOC flags in the 'doc' inspection
* Include rpm/rpmfi.h insted of rpm/rpmfiles.h
* Only check regular files and symlinks in the 'doc' inspection
* Remove unnecessary assert() statements in filecmp()
* Remove incorrect warnx() reportings based on filecmp() return value
* Exclude man pages from the 'doc' inspection
* Honor the -a command line option for downloads as well as runtime (#233)
* Allow optional 'commands' block in the config file
* Fix assorted non-critical memory leaks
* Remove unnecessary warn() after a failed stat()
* Additional memory fixes for the abidiff inspection (#244)
* Free ELF symbol names list in find_lto_symbols() before return
* Followup to the memory fixes for read_abi() and free_abi()
* Prevent invalid pointer dereferencing in invalid result in 'patches' (#245)
* Avoid reusing the same abi_pkg_entry_t struct in read_abi()
* Allow a set of excluded path prefixes in 'pathmigration'
* Fix the YAML parsing for the pathmigration block
* Document the BRANCHES variable for 'make koji'
* Include the .asc file when submitting new Koji builds (#191)
* Include the .asc file in the spec file


Changes in rpminspect-1.1
-------------------------

* Formatting fixes in Makefile help output
* Begin config file restructuring starting with rpminspect-data-generic
* Support multiple configuration files.
* Docs work in progress.
* Only fail the annocheck inspection for RESULT_VERIFY.
* Read debuginfo if available when running the 'annocheck' inspection.
* Add the '%files' inspection to librpminspect
* Add __attribute__((__sentinel__)) to the run_cmd() prototype
* Add test suite cases for the '%files' inspection.
* Added the 'types' inspection to compare MIME types between builds.
* Update TODO file
* Update the MISSING file
* s/rpminspect.yaml/generic.yaml/ in the Makefile and README
* Skip debuginfo and debugsource packages in the 'types' inspection
* Add test_types.py to the test suite
* Note the 'types' inspection generic.yaml
* Modify add_entry() in init.c to skip duplicate entries
* Start GitHub Action workflow files for rpminspect.
* Install meson in ci-ubuntu.yml
* Change 'nls' option in meson_options.txt to a boolean
* Install gettext for ci-ubuntu
* Add more build dependencies to ci-ubuntu.yml
* Drop the 'method' parameter from dependency() lines in meson.build
* Split xmlrpc libs to separate dependency() lines in meson.build
* More xmlrpc updates for meson.build and lib/meson.build
* Try to support systems with xmlrpc-c without the pkgconfig file.
* Changes to build on Ubuntu, specifically the GitHub Actions system
* Syntax error in ci-ubuntu.yml
* Add ci-fedora.yml for GitHub Action CI on Fedora
* Fix errors in ci-fedora.yml
* Put all of the ci-ubuntu.yml steps in ci-ubuntu.yml
* Remove install-libmandoc.sh and ubuntu-pkgs.sh helper scripts.
* Install python3-setuptools in ci-ubuntu.yml
* Install rpm-build in ci-fedora.yml
* Install libxmlrpc-core-c3-dev in ci-ubuntu.yml
* Disable ci-ubuntu.yml for now, enable code coverage in ci-fedora.yml
* Remove Travis-CI files.
* coverage fixes for ci-fedora.yml
* Install git in ci-fedora.yml
* Enable manual dispatching of the CI on Fedora tests
* Remove actionspanel thing for GitHub Actions, drop Coveralls block
* Comment the ci-fedora-yaml file
* Fix the 'elf' inspection and test_elf.py on Ubuntu
* Enable the ci-ubuntu GitHub Action again
* Check all return values of getcwd()
* Ignore installed Python modules with pip3 in ci-ubuntu.yml
* Build 'execstack' test program with -Wl,-z,lazy
* Add ci-centos8.yml to enable CI on CentOS 8 as a GitHub Action
* s/centos8:latest/centos8/g
* Trying 'container: centos:centos8'
* Drop 'sudo' from ci-centos8.yml
* Rename README to README.md
* Enable GitHub Action for CI on CentOS 7
* Drop -I from the pip install line in ci-centos7.yml
* Use the 'make instreqs' target for install test suite deps.
* Add back 'dnf -y install 'dnf-command(builddep)'' to ci-fedora.yml
* Nope, that doesn't do it.  Just install make in ci-fedora.yml first
* More 'make instreqs' fixes.
* s/scripts/utils/g in the Makefile
* s/TOPDIR/topdir/g in the Makefile
* Install make in ci-centos7.yml
* Move REQS and PIP lists out of the Makefile to files in osdep/
* '^$$' -> '^$'
* Slightly different sourcing of the osdep/ files
* Set OS using := in the Makefile
* Make the reqs.txt files in osdep/ contain all deps
* linux-headers -> linux-headers-$(uname -r) for Ubuntu
* Remove html401-dtds from osdep/centos8/reqs.txt
* Use `` instead of $() since this list goes through make(1)
* Does $(shell uname -r) work in this case?
* More osdep/ work and simplification.
* Rename 'osdep' to 'osdeps'
* meson patches for opensuse
* Move mandoc installation to post.sh in osdep/ubuntu
* Add GitHub Action for CI on OpenSUSE
* opensuse:latest -> opensuse/leap:latest
* Install tar in ci-opensuse.yml
* Install gzip in ci-opensuse.yml
* More fixes for ci-opensuse.yml
* Small fixes to determine-os.sh
* ci: Add Python linting jobs
* ShellCheck fixes for the regress/ scripts
* ci: Add ShellCheck to lint shell scripts
* Adjust how the Makefile reports unknown operating system.
* Just check $ID in determine-os.sh for opensuse
* Use pip for PIP_CMD on opensuse-leap
* Update the centos images before doing anything else.
* Install curl in opensuse-leap
* Get 'rc' from Fedora on opensuse and copy it to /usr/local
* Install kernel-default-devel for opensuse CI
* Move the 'uses' part of the centos CI jobs to the first step
* ShellCheck fixes for the utils/ scripts.
* Some flake8 fixes in test/, using yapf
* Combine all of the GitHub Actions CI runs in to ci.yml
* Manually install rpmfluff on CentOS 7; pip is failing here
* Use rpmfluff-0.5.7 explicitly for centos7
* Use rpmfluff-0.5.6 on centos7
* OK, let's try rpmfluff-0.5 for centos7
* Last try, rpmfluff-0.5.4.1 for centos7
* Back to trying to manually install rpmfluff for centos7
* Style the Python code with Black
* Add the Black formatting commit to blame revision ignore list
* OK, just copy rpmfluff.py in place
* Make sure to manually install rpmfluff-0.5.7.1 for centos7
* Don't assume we have a header or even a list of files (#161)
* python: flake8: drop * imports
* python: flake8: wrap long lines to less than 100 characters
* python: flake8: drop unused imports
* python: flake8: remove unused local variables
* python: flake8: add PEP8 whitespace
* python: flake8: mark in-line bash scripts as raw strings
* python: rename several duplicate test cases
* ci: merge style workflows
* Adjust rpminspect.spec.in for file moves and default changes.
* Return the reallybadword to the metadata tests
* Adjust test_metadata.LosingVendorCompareKojiBuild to expect VERIFY
* The shared libmandoc check should not look for a static libmandoc
* Update the AUTHORS file
* Rename ipv6_blacklist to forbidden_ipv6_functions
* Rename stat-whitelist to fileinfo
* Rename 'caps_whitelist' to 'caps' and drop the use of 'whitelist'
* Rename abi-checking-whitelist/ to abi/ in /usr/share/rpminspect
* Rename 'version-whitelist/' to 'rebaseable/' in /usr/share/rpminspect
* Rename "political-whitelist/" to "politics/" in /usr/share/rpminspect
* Drop unnecessary method re-definitions in base test classes
* Use super() rather than explicitly calling the parent class
* Call configFile() on object instance rather than using the parent class
* Upload coverage report to codecov
* Improve the error reporting for test result checking
* Convert to AUTHORS.md file, add Makefile target to generate it.
* Introduce the 'movedfiles' inspection and a lot of other fixes (#155)
* AUTHORS -> AUTHORS.md in rpminspect.spec.in
* Add basic tests for the filesize inspection
* Multiply the file size difference before dividing
* Update README.md
* Update README.md (more Markdown changes)
* Update POTFILES and rpminspect.pot
* chmod 0755 test_filesize.py
* tests: optionally check the result message
* tests: add further filesize tests for shrinking files
* inspect_filesize: drop extra - from the message about file shrinkage
* Enable 'permissions' inspect for single build analysis.
* Add 24 new test cases to cover the 'permissions' inspection.
* chmod 0755 test_permissions.py
* Make sure all RESULT_INFO results are set to NOT_WAIVABLE
* Fix some specific problems with the 'permissions' inspection.
* Pass "-r GENERIC" to rpminspect in the TestCompareKoji class
* Add 12 more permissions test cases for setuid file checks
* Update TODO list
* Update test_symlink.py tests for new waiver_auth values
* Add a fedora-rawhide job and renamed 'fedora' to fedora-stable
* Update the rpminspect.pot translation template
* Relicense librpminspect (lib/ and include/) as LGPL-3.0-or-later
* Ignore .tox/ subdirectory
* License the rpminspect-data-generic subpackage as CC-BY-4.0
* Add a copy of the Apache 2.0 license for the 5 files in librpminspect
* Update the License tag in the spec file and the %license lines
* Add debian-testing as a CI workflow; add missing osdeps files.
* Update determine-os.sh to handle Fedora stable and rawhide
* Drop the use of 'sudo' in ci.yml
* sudo required for Ubuntu CI job, install make for debian-testing
* See what $ID is set to in determine-os.sh
* Workaround a bug in meson 0.55.0 for Fedora CI jobs
* Add 'debian' catch to utils/determine-os.sh
* Rename 'osdeps/debian-testing/' to 'osdeps/debian/'
* Add opensuse-tumbleweed to the CI job list
* Add libmagic-dev to osdeps/debian/reqs.txt
* Fix memory corruption in init_rpminspect
* Add comment clarifying the License tag in the spec file.
* If check_results() raises AssertionError, dump the JSON output
* Fix test_changelog.py test cases that are failing.
* Fix UnbalancedChangeLogEditCompareKoji
* Handle rpm versions with x.y.z.w version numbers in test_symlinks.py
* Fix mandoc build problems in opensuse-tumbleweed CI job
* Install gcovr using pip on opensuse-tumbleweed
* Handle systems that lack pkg-config files for libelf and libcap
* Add archlinux CI job in GitHub Actions
* Forgot --noconfirm on the 'pacman -Syu' line.
* Add missing DESC_MOVEDFILES block to inspection_desc()
* More minor fixes to the Arch Linux CI job.
* Install gcovr with pip for the Arch Linux CI job.
* Support building on systems that lack <sys/queue.h>
* Add detection for <sys/queue.h> to meson.build
* Ensure an int is used for snprintf() in inspect_manpage_path()
* WIP: 'abidiff' inspection
* Only report permissions change if there is a mode_diff (#181)
* Fix -Werror failures in inspect_abidiff.c
* Add sl_run_cmd() to librpminspect.
* Add get_arches() to librpminspect
* WIP: abidiff inspection
* Some minor edits to the README.md file
* More minor updates to the README.md file
* Replace get_arches() with init_arches()
* Add test_addedfiles.py to the integration test suite
* Expand find_one_peer() to soft match versioned ELF shared libraries
* Add the beginnings of the 'abidiff' inspection code.
* Report out findings in the abidiff inspection.
* Update the test suite to cover rpmfluff 0.6
* libmandoc configure workaround needed on Debian too
* shellcheck fixes for the scripts in utils/
* Add abi.c, the code that reads in the ABI compat level files (#144)
* Be sure to close the open file before exiting init_fileinfo()
* Python formatting cleanups
* Add --diff to the Python format checker
* Add new setting to abidiff section of the config file
* Add -n/--no-rebase command line option to disable rebase detection
* Store size_threshold as a long int rather than a char *
* Check abidiff(1) results against the ABI compat level definitions (#144)
* Add 'apt-get -y install libgcc-s1:i386' to pre.sh for Debian
* Add integration test cases for the abidiff inspection (#144)
* Add 'dpkg --configure -a' to pre.sh for debian
* Install libterm-readline-perl-perl for debian CI
* Install libabigail for Fedora and CentOS CI jobs
* libgcc-s1:i386 -> lib32gcc-s1 for debian CI
* Install libabigail for opensuse-leap, opensuse-tumbleweed, and arch CI
* Install libabigail for debian and ubuntu CI
* Install libabigail-dev for debian and ubuntu, not libabigail
* Install libabigail-tools on opensuse-leap and opensuse-tumbleweed
* Install libabigail-git for arch linux CI
* Move free_argv_table() to runcmd.c
* Install 'abigail-tools' for debian-testing and ubuntu CI
* Install libabigail using the Arch User Repo on arch CI
* Explain the osdeps/ subdirectory.
* No, just clone libabigail from git and build it manually on arch
* Add beginning of kmidiff inspection, put ABI functions in abi.c
* Read list of possible kernel executable filenames from the config file.
* Drop abidiff_ and kmidiff_ from extra_args; add kernel_filenames
* Just call the abidiff and kmidiff extra args settings "extra_args"
* Define 'kmi_ignore_pattern' in the config file.
* Handle builds that lack all debuginfo packages (#186)
* Do not assume peer->after_hdr exists (#187)
* Store copy of original pointer in strsplit() to free at the end.
* Use mmap() and strsplit() in read_file() rather than a getline() loop
* Fix memory leaks in abi.c functions
* open() failure in readfile() is not fatal, just return NULL
* Add utils/gate.sh
* Have check_abi() pass back the ABI compat level found
* Update descriptions for abidiff and kmidiff inspections
* Hook up the kmidiff inspection.
* Use read_file() in init_fileinfo() and init_caps()
* Use read_file() in validate_desktop_contents()
* Use read_file() in disttag_driver()
* Adjust how init_fileinfo() and init_caps() iterate over file contents
* Fix 'tox -e format' style problems found.
* Avoid comparing elf files that are not shared libraries
* Support --kmi-whitelist in the kmidiff inspection
* Trim worksubdir from paths in reported abidiff and kmidiff commands
* Remove the kmi_ignore_pattern setting for the config file.
* Create include/queue.h to replace the _COMPAT_QUEUE blocks everywhere
* Update AUTHORS.md
* Report metadata changes for rebased packages as INFO
* Do not fail the specname inspection when given a non-SRPM
* For passing upstream inspections, do not report a remedy string.
* Do not fail the lostpayload inspections if it only gives INFO messages
* Clarify unapproved license message in the license inspection
* Use FOPEN_MAX for nopenfd parameter in nftw() calls
* Make sure to close open file descriptors from get_elf() calls.
* Include 'src' architecture in the rpminspect runs in gate.sh
* Make sure kmidiff is listed in the spec file
* TODO updates
* Update rpminspect.pot and POTFILES for translations


Changes in rpminspect-1.0
-------------------------

* Use this project's user.name and user.email for Koji builds
* Use YAML for the rpminspect configuration file and profiles
* Reduce the number if (null) prints in debug mode for shellsyntax
* Update README
* Fixes a ValueError if hostname has no periods
* Remove '#include <iniparser.h>' from rpminspect.h
* Install python3-pyyaml in the Docker test environment.
* Use fedora:rawhide for the Docker tests.
* Try multiple ways of finding the kernel development files in test_kmod.py
* Syntax error in test_kmod.py
* Force kver in list iteration to a string.
* Update 'hardened' annocheck definition, add another LTO prefix
* Debugging output in inspect_lto.c
* In test_ownership.py, use the built rpminspect rather than a script.
* More debugging output in inspect_lto.c
* In find_lto_symbols(), start at SHT_PROGBITS instead of SHT_SYMTAB.
* In test_symlinks, use built rpminspect rather than a shell script.
* Use built executables in test_upstream.py tests rather than scripts.
* Add '%global __os_install_post %{nil}' to rpmfluff spec headers.
* Make sure man pages we expect gzipped are gzipped in test_manpage.py
* Set QA_SKIP_BUILD_ROOT=1 in %install in test_symlinks.py tests.
* Add '%global __arch_install_post %{nil}' to symlinks spec headers.
* Install kernel-core for the test suite.
* Find the kernel build directory in test/data/derp-kmod/Makefile
* Skip ELOOP symlink tests if rpm >= 4.15.90 is used
* Pass the kernel build directory to derp-kmod/Makefile from test_kmod.py
* Install 'make' in the Docker test environment
* Support Linux 5.6.0 struct proc_ops in derp-kmod
* Final derp-kmod fixes for the 5.6.0 and higher kernels.
* rpminspect.conf -> rpminspect.yaml in rpminspect.spec.in
* Set default JVM byte code version to 43 in rpminspect.yaml
* Update the local test instructions to run individual test scripts.
* Report changed files as RESULT_INFO when rebasing packages (#150)
* Ignore missing XML entity definition errors (#148)
* Add some basic verbose output from rpminspect(1)
* Split 'emptyrpm' inspection in to 'lostpayload' and 'emptyrpm' (#147)
* Move _() usage for DESC_* macros to inspect.h
* Handle INSPECT_LOSTPAYLOAD in inspection_desc()
* Search correct files for POTFILES additions.
* Update POTFILES and rpminspect.pot template
* Add DESC_PATHMIGRATION and reformat the struct inspect for reading.
* Small update to the MISSING file.
* Install python3-devel for the test suite.
* Install libffi-devel for the test suite
* Use the rpm Python module in test_syslinks.py to get rpm version.
* Add check_results() to test/baseclass.py
* Report %changelog section differences as INFO (#123)
* Report dangling symlinks as INFO for now (#145)
* Add get_specfile_macros() and get_macros() to librpminspect (#152)
* Fix ELF_K_AR handling in get_elf_machine() (#153)
* Remove stray 7 from an #include line
* Don't worry about EM_BPF objects in ELF_K_AR file types (#153)
* Fix -fPIC loss/gain reporting in the elf inspection (#153)
* Support macros in the Release tag in the 'disttag' inspection (#152)
* Simplify the get_elf_section() function a bit.
* Do not report all after objects without -fPIC as having lost PIC (#153)
* Drop eu-elfcmp(1) usage in the changedfiles inspection.
* Rename on_stat_whitelist() to on_stat_whitelist_mode(), fix some errors.
* Add on_stat_whitelist_owner() and on_stat_whitelist_group()
* Ignore debug paths in the symlinks inspection.
* Install 'setup' in the Docker test image
* Add mock(1) in the Docker test environment.
* Remove DEBUG_PRINT for the config file name read in init.c
* Display errno value when getpwnam_r() or getgrnam_r() fail
* Handle missing users and groups from the system
* Add sssd-client to the Docker test environment
* More debugging output while working on Travis-CI problems
* Further Travis-CI debugging for the ownership inspection.
* Continuing to debug this problem in Travis-CI
* And continued Travis-CI test_ownership debugging...
* Fix problem constructing package download URLs in librpminspect.
* Only try to read the UID or GID in the ownership inspection.
* Update translation template file.
* Update po/POTFILES list
* Final getpwnam_r()/getgrnam_r() changes for whitelist.c
* Restrict RPM spec file macro gathering to %define and %global.
* If shdr in _get_elf_helper() is NULL, return NULL.
* s/%%/%/g in results.h
* Account for whitespace other than ' ' on Release: lines (#157)
* Add a new disttag test case to cover tab field separators.
* Ignore multiline macros in get_specfile_macros()
* Add more example data to test_disttag.py to cover recent bug reports.
* Add ignore_path() function to librpminspect
* Expand foreach_peer_file() with use_ignores parameter.
* Revert work-in-place changes in inspect_elf.c so the test suite passes.
* Update the 'ignore' section in rpminspect.yaml
* Expand strreplace() to support removing substrings.
* Trim rpminspect working directory from annocheck(1) details.
* Skip debuginfo files in the annocheck inspection.
* If reltarget is "", do not try to further modify it (#159)
* Add manually-invoked regression testing scripts.
* BR libmandoc-devel >= 1.14.5
* BR libmandoc-devel without specific version for EPEL-7 and F-30
* Make sure all static path buffers use PATH_MAX consistently.
* Support a positional parameter on 'make check' to run part of test suite.
* Followup to the PIC check for static ELF libraries (#153)
* Create TARGET_ARG to get optional target arguments.
* Change *-dtds packages from Requires to Suggests for el8 and fedora.
* 'sort | uniq' -> 'sort -u'
* Rename 'make release' to 'make new-release'; add 'make release'
* Fix build_module() in test_kmod.py


Changes in rpminspect-0.13
--------------------------

* Remove the GitHub Release page stuff from utils/release.sh
* Drop meson_version from meson.build
* Change meson.build to require xmlrpc-c >= 1.32.5
* Fix some errors when running with libiniparser 3.1
* Only set CURLOPT_TCP_FASTOPEN if we have it available.
* Expand the template rpminspect.conf file for the test suite.
* Handle 'localhost.localdomain' FQDN in the test suite base clases
* Rework the test_manpage.py tests to work with rpm >= 4.11.x
* BR xmlrpc-c >= 1.32.5 and iniparser >= 3.1
* README updates
* Modify the Makefile so it works with 'ninja' or 'ninja-build'
* Rename the tests/ subdirectory to test/
* Split meson.build out in to different meson.build files.
* Move builds.c to lib/, remove builds.h from src/
* Move rpminspect.conf to data/, expand data/meson.build
* Fix the --version output to remove '@' wrapping the version number.
* Remove diff.3, the code is gone from lib/
* Begin doc/Doxyfile for API documentation.
* Add Doxygen documentation for badwords.c, builds.c, and checksums.c
* Make sure the changelog inspection runs with before/after pairs (#130)
* Ignore debuginfo and debugsource packages in the kmod inspection.
* Skip the kmod inspection if there is no peer_file (#131)
* Handle kernel modules that move paths between builds (#131)
* Test cases for kernel modules changing paths between builds (#131)
* Add Doxygen documentation to four C files, update others.
* First part of reworking the add_result() API.
* Add init_result_params() to reset the struct result_params structures.
* Additional Doxygen documentation blocks for librpminspect.
* More librpminspect documentation work.
* Un-static some of the inspect_elf.c functions.
* Remove MPARSE_MAN to let libmandoc autodetect the type (#132)
* Revise list_to_string() to support optional delimiter.
* Add get_elf_section_names() to librpminspect
* Support [lto] section with lto_symbol_name_prefixes in rpminspect.conf
* Add 'LTO' inspection to librpminspect (#129)
* Add 'LTO' inspection test cases (#129)
* Fix free(): double free detected in tcache 2 (#134)
* Do not strdup() header and remedy in add_result_entry()
* Store package extract root in rpmpeer_entry_t for each package.
* Add strtype() to librpminspect to return string indicating file type.
* Add the 'symlinks' inspection to librpminspect (#133)
* Add tests for the 'symlinks' inspection to the test suite
* Update README file
* chmod 0755 test_symlinks.py
* symlink inspection adjustments based on feedback (#135 & #136)
* Simplify the license inspection routine (#138)
* Add get_elf_machine() to readelf.c (#139)
* Elf64_Half -> GElf_Half in dt_needed_driver()
* Skip eBPF ELF objects in the 'elf' inspection (#139)
* Stop appending a newline to string in strappend()
* Collect all results from getLatestBuild Koji XML-RPC call (#137)
* Return EM_NONE in get_elf_machine()
* In download_build(), fix how srcfmt is set.
* Fix some memory errors associated with the results and parameters.
* Add a new faux-result to the results output for 'rpminspect'
* Use params.msg for reporting in check_bin_rpm_changelog()
* Add teardown steps in baseclass to clean up rpm build files
* Adding tests to validate file ownership and capabilities tests
* Added build cleanup in baseclass and fixed setUp typo - still testing
* Many more tests fixed - down to six failures.
* Fixes to mockbuild tests
* Fixed some duplicate class names and now passing all tests
* Added docstrings to all tests and other small bugfixes
* Added docstrings to all tests and other small bugfixes
* Formatting and style fixes
* Make sure only RPM files are passed to get_rpm_info()
* Update TODO list
* Return get_rpm_info() and add_peer() have void returns.
* When public headers change in 'changedfiles', do not free param.details
* Check is eptr->data is NULL in find_one_peer (#142)
* Define EM_BPF if we lack it.
* Skip 'upstream' inspection if no source packages are provided.
* Add explicit librpminspect Requires to the main package.
* Add test cases for the 'upstream' inspection.
* Simplify how the versions are collected in inspect_upstream()
* Update translation template.
* New release (0.13)


Changes in rpminspect-0.12
--------------------------

* Do not use headerLink() in extract_rpm()
* Expand the 'kmod' inspection to check module dependencies.
* Expand 'kmod' inspection to cover PCI device IDs
* Update the TODO list
* Update Dockerfile.test for Fedora 31
* Updates for the MISSING file
* Add DEBUG_PRINT() macro to librpminspect.
* Add 'favor_release' setting in rpminspect.conf under [vendor] (#98)
* Add more lines to [inspections] in src/rpminspect.conf
* Change up the DEBUG_PRINT() macro
* Don't assume favor_release is set in rpminspect.conf
* Format stdout and stderr correctly from baseclass, run with -d
* Correctly recognize parenthesized license substrings (#101)
* Shorten the names of test classes.
* Generate a dummy 'changelog' for the CI tests
* Adjust how the dummy changelog is made for Travis-CI
* One more slight change to how .travis.yml runs mkrpmchangelog.sh
* Nope, just can't spell the name of my own script.
* Copy the dummy changelog to /root/rpmbuild/SOURCES
* Add /bin to PATH in mkrpmchangelog.sh
* Make sure glibc[-devel].i686 is installed for the test suite.
* Add test kernel module for use in the integration test suite.
* Update the TODO list
* Note the test suite needs kernel-devel installed.
* Expand the derp kernel module for depends and alias support.
* Fix up some of the kmod functions in librpminspect
* In the test suite, use the same before & after package version.
* Add test_kmod.py to the test suite.
* Stub out test_ownership.py and test_shellsyntax.py
* * extract_rpm -> *extract_rpm
* Handle parenthesized license substrings with all tokens (#102)
* Combine prototype sections in rpminspect.h for kmods.c
* Implement the 'arch' inspection.
* Add a section to the README file explaining how to run the tests.
* Add the 'subpackages' inspection.
* Ignore 'build' directory.
* Fix the kernel module alias tests in test_kmod.py
* Follow the pep8 style
* Remove unused import in tests
* Start some spec file changes to support building on OpenSUSE
* Update TODO and MISSING
* Move the assert() for ri in inspect_modularity() up.
* Use 'pip3' in .travis.yml and Dockerfil.test
* More updates to Travis-CI control files.
* And another change for the .travis.yml file
* Revert "Start some spec file changes to support building on OpenSUSE"
* Handle return code 1000 from the Koji hub.
* Expand meson.build and rpminspect.spec.in for EPEL-8
* Support libiniparser v3.x and v4.x APIs
* Added the 'changelog' inspection.
* Add possibility to run integration test suites separately
* Begin i18n support for string translations.
* Mark strings in lib/ for translation, update rpminspect.pot
* How to run and debug integration tests
* Fixes for i18n string marking changes.
* More build fixes for the change to translatable strings.
* Favor libmandoc.a when build rpminspect
* Display the line number in DEBUG_PRINT()
* Expand the config file with [pathmigration] and [ignore]
* Fix double free in list_free() (#107)
* Really fix the double free() in arch/subpackages (#107)
* Do not require a tty when running (#109).
* Add the 'pathmigration' inspection and test cases.
* Rework the 'xml' inspection to better handle DTDs and SVG files (#110)
* Default rpminspect.conf updates
* Expand 'filesize' to report changes at different levels by percentage
* Update TODO list.
* Fix bug in shellsyntax where before and after builds are invalid.
* Add shellsyntax tests for /bin/sh
* Add libdiff code snapshot to librpminspect
* Patch diff.c so it builds in librpminspect
* Include diff.h in rpminspect.h
* Refactor variable names
* Add some helper functions for reading and working with text files.
* Implement the unified_diff() library function.
* Add two utility functions to librpminspect.
* Change upstream_driver() to use unified_diff()
* Use unified_diff() in changedfiles rather than running DIFF_CMD
* Fix for 'error: format not a string literal and no format arguments'
* Split unified_diff() in to unified_file_diff() and unified_str_diff()
* Change 'make POTFILES' to 'make update-pot', update po/rpminspect.pot
* Update TODO list
* Add /bin/bash tests
* Add DEBUG_PRINT() to annocheck_driver()
* Rename 'Screendump' to 'Details' in the output listings.
* s/depends/dependencies/ in meson.build
* Fix SIGSEGV in inspect_changelog() (#115)
* Add tests for shellsyntax for /bin/ksh
* Handle RPMs with empty or missing changelogs (#116)
* Update the AUTHORS file.
* Add tests for shellsyntax for /bin/zsh
* Add tests for shellsyntax for /bin/tcsh
* Add tests for shellsyntax for /bin/csh
* Refactor the unified_diff.c code to be more reliable (#116)
* Correct RPM %changelog output to match rpm (#116)
* Stop adding DIFF_COMMON lines twice when the hunk continues (#116)
* Adjust the unified_output() function for more output control (#116).
* Remove libdiff code and return to using diff(1) via fork and exec.
* Fix some memory leaks in inspect_changelog.c
* Update the test_changelog.py to do list.
* Added the tests for losing the %changelog in the after builds
* Update the 'make instreqs' target, README, and rpminspect.spec.in
* Unify the readme with yum as rpminspect works on yum as well as dnf
* Require xmlrpc >= 1.51.0
* Add get_nevr() and get_rpmtag_str() to lib/rpm.c
* Do not abort if the hdr is NULL in get_changelog() (#127)
* Update test_changelog.py with more changelog inspection tests.
* Remaining set of changelog inspection fixes.
* Make sure remaining test_changelog.py test cases pass.
* Handle shell syntax tests that don't return non-zero on failure.
* Add tests for shellsyntax for /bin/rc
* Add release.c to librpminspect source.
* If no new %changelog entry is found, report OK if NVRs match. (#127)
* Modify license inspection reporting.
* Split add_result() in to add_result() and add_result_entry()
* Prevent reporting unapproved license tokens for approved expressions.
* Disable empty license tag tests, add ValidGlibcLicenseTagKoji
* Temporarily disable UnprofessinalChangeLogEntryCompareKoji
* Update po/ files.
* Adjust how GitHub release assets are uploaded.
* New release (0.12)


Changes in rpminspect-0.11
--------------------------

* Change the way get_mime_type() works so it caches the type.
* Support an optional [annocheck] section in the config file (#62)
* Add a 'make help' target in the helper Makefile
* Add 'annocheck' inspection to librpminspect (#62)
* Add weak dependency for /usr/bin/annocheck (#62)
* EM_S390 maps to R_390_* macros in /usr/include/elf.h
* Only run the changedfiles inspection under certain conditions (#74)
* Skip source packages in the changedfiles inspection (#74)
* Add back missing free() in rpminspect.c, but in the right place.
* Simplify the returns out of changedfiles_driver()
* Add the 'DT_NEEDED' inspection to librpminspect.
* Expand 'elf' inspection description to mention forbidden function check.
* Change result severity in 'upstream' based on package versions.
* Cache rpmfile_entry_t checksums.
* Add 'filesize' inspection to librpminspect.
* Updated top level docs.
* Check all license abbreviations in the 'license' inspection (#83)
* Use MESON_BUILD_DIR in the Makefile 'all' target.
* Drop MAGIC_SYMLINK from magic_open() in get_mime_type()
* Move stat-whitelist checker to whitelist.c
* fixup! Move stat-whitelist checker to whitelist.c
* Change the prototype for add_result()
* Add the 'permissions' inspection.
* Replace check_stat_whitelist() with on_stat_whitelist()
* Add 'make instreqs' target to install build and runtime dependencies.
* Handle ENOENT failures from realpath() in unpack_archive() (#84)
* Only run the DT_NEEDED check for ET_DYN files (#85)
* Update the HISTORY file
* Add a [specname] config file setting to rpminspect.conf (#86)
* Boilerplate updates
* Implement runtime profiles (#82)
* Update TODO list
* Compare JVM major version against the minimum JVM version (#89)
* When reporting public header changes, skip first 3 lines from diff(1)
* Address errors reported in DT_NEEDED inspection in BZ#1793113
* Various word fixes in the rpminspect.1 man page
* PRIMARY_BASENAME -> PRIMARY_FILENAME
* Correct the javabytecode major JVM version comparison (#89)
* Reset field to MODE when reading the stat-whitelist (#90)
* Add vendor_data_dir setting to rpminspect.conf
* Rename [tests] section in rpminspect.conf to [settings]
* Expand rpminspect.conf with an [inspections] section
* Improve failure reporting in the integration test suite
* Handle symlinks in copytree() in src/builds.c (#92)
* Make sure "noarch" and "src" are always honored as package arches (#93)
* Update TODO list.
* Formatting changes on MISSING
* Add get_rpm_header_arch() to librpminspect
* Further improve the handling of 'noarch' and 'src' for -a (#93)
* Use get_rpm_header_arch() to retrieve RPMTAG_ARCH
* Move to using libcap instead of libcap-ng
* Fix Koji scratch build download support in librpminspect (#94)
* Add the 'capabilities' inspection to librpminspect.
* Renamed [vendor-data] to just [vendor] in the config file.
* Implement an RPM header cache in librpminspect
* Restrict get_cap() to S_IFREG files.
* Note the licenses for all of the dependent components.
* get_rpm_header() pointer fixes
* Implement Freedesktop.org icon lookup routine for 'desktop' (#95)
* updating imports to use libcap, matching BuildRequires in specfile
* If vendor is not set in rpminspect.conf, skip tag check.
* Handle upstream source adds/removes with pkg version changes (#96)
* Boilerplate updates
* Improve "Missing license database" error message.
* Update TODO list
* Use xasprintf() instead of strdup() here for parameters
* Begin the 'kmod' inspection for kernel modules.
* Typo fix in src/rpminspect.conf:  s/not/no/
* Remove some memory leaks in librpminspect and rpminspect.
* Shorten -l output, use -v to get current output.
* Remove -x from #! in utils/release.sh
* New release (0.11)


Changes in rpminspect-0.10
--------------------------

* Disable jq usage in the release script for now
* Generate a complete %changelog block for the spec file
* Force overwrite of generate .tar file in .copr/Makefile
* Run srpm and check targets before koji, pass name of tarball
* Add utils/submit-koji-builds.sh, which drives 'make koji'
* Genericize submit-koji-builds.sh a bit more
* Start a .gitignore file
* Changes to the release and build automation for the RPM changelog
* Handle int32 and int64 size results from Koji over XMLRPC (#61)
* Make --copr mode on utils/mkrpmchangelog.sh generic
* Existing packages that go away in a comparison are VERIFY (#59)
* inspect_elf() RESULT_OK should be NOT_WAIVABLE
* Refactor strprefix()
* Fix a memory leak in init_rpminspect() with the javabytecode array.
* Fix a memory leak in the license inspection.
* Update release and build steps in the README
* test_elf.py tests that verify passing tests are Not Waviable
* Build package download URLs correctly, drop use_volume_name setting
* Clean up rpminspect error messages when downloading.
* For fetch-only mode (-f), default workdir to getcwd()
* The new libmandoc API works, drop explicit 1.14.4 BuildRequires
* Add is_text_file() to librpminspect
* Handle multiline results in run_cmd() correctly (#59)
* Show diff(1) output for changing upstream text sources (#59)
* Adjust run_cmd() calls to match the new API
* Explain how the -w option works with the -f option in the man page
* In REMEDY_LICENSE, explain that valid licenses need to be in licdb
* Fix some memory leaks in the license inspection.
* Add a [products] section to the config file for mapping release strings (#68)
* Fix json-c memory leak.
* Small memory leak fixes for the product release string gathering.
* Trying to get the automated release notice posting working on github.
* Use printf to build github release body text
* changelog -> ${CWD}/changelog
* Make upstream spec file include the changelog like downstream.
* Syntax errors fixed in utils/release.sh
* Escape newlines in the release body text.
* Run the release script with -x for now, for debugging.
* Shift things around a bit in the release.sh script
* Use jq(1) in utils/release.sh to generate the JSON data for GitHub.
* Slight changes to the jq(1) commands.
* Even more small changes to the jq(1) command.
* Do not build the spec file or SRPM in the release.sh script
* Correctly generate the git log entry for the release.
* Edit the release on github rather than create a new one.
* Still working out problems with automating releases on github.
* Fix 'Builds have different product release (el7 != el7) (#70)
* Remove unnecessary free()
* Shorten the eu-elfcmp reporting (#71)
* Switch to using a BUFSIZ buffer for getline() in run_cmd()
* Expand the get_product_release() functionality (#72)
* Detect product release strings correctly with underscores (#76)
* Support SRPM files that lack RPMTAG_SOURCE entries (#78)
* Handle subpackages with different version numbers (#77)
* Add support for Koji scratch builds in rpminspect (#60)
* Make get_product_release() even if [products] is missing.
* Drop the extra strdup(), xmlrpc_decompose_value() handles it for us.
* Update the rpminspect(1) man page.
* line-buffer stdout - makes "rpminspect ... | tee" behave better
* Clean up memory leaks with the Koji task handling code.
* Add a SIGABRT handler to rpminspect(1)
* bad free()
* Change the task member of koji_task_entry_t to be 'struct koji_task *'
* Skip shellsyntax inspection on source packages (#79)
* Fix set_worksubdir() TASK_WORKDIR test (#80)
* New release (0.10)


Changes in rpminspect-0.9
-------------------------

* Complete the tests/test_manpage.py integration tests
* Add xml inspection integration tests.
* disttag inspection failures should be not waivable
* All RESULT_OK results should be NOT_WAIVABLE
* Expand integration test suite to support waiver auth checking
* Improve forbidden_path_prefixes results reporting (#59)
* Add a HISTORY file explaining a bit about the history of rpminspect
* Use lstat(2) in copyfile() so symlinks are correctly handled
* Fix a problem with peer detection when comparing single RPM files.
* Stop setting whichbuild in so many places in build.c
* Two hidden bugs in inspect_elf.c resolved via the integration suite
* Add test_elf.py to the tests/ subdirectory
* Add elf_ipv6_blacklist to the sample rpminspect.conf file
* chmod 0644 fortify.c
* Reformat a line in get_elf_section() to make it more readable.
* Add forbidden IPv6 function use tests to tests/test_elf.py
* Forgot to add tests/data/forbidden-ipv6.c
* Add test_elf.py test cases for DT_TEXTREL on 32-bit architectures
* Use headerGetString() throughout librpminspect
* Add some missing free() calls in run_cmd()
* Use string_list_t ** for user_data in elf_archive_iterate()
* Add losing -fPIC on 32-bit builds test in test_elf.py
* Add get_nevra() to librpminspect to get RPMTAG_NEVRA
* Do not output Waiver Authorization for RESULT_INFO results
* Call rpmFreeRpmrc() from main() before the program exits
* Adjust how and where rpmtdFree() is called
* Use the GNU version of basename(3) and ensure we don't use the libgen version.
* Fix forbidden_path_prefixes check in the addedfiles inspection (#59)
* Add missing free() to inspect_desktop.c
* Simplify the is_valid_license() code that concatenates tokens
* Simple Makefile to drive different parts of the build.
* Add the git log to the release notification published to github
* Fix error in the 'make release' target
* If asset ID cannot be found, dump what github returned on stdout
* Use jq(1) to escape strings for JSON
* New release (0.9)


Changes in rpminspect-0.8
-------------------------

* Adjust .copr/Makefile so it can make release SRPM files
* Drop the GITHASH and GITDATE work from utils/release.sh
* Finish the rest of the test cases for test_license.py
* Finish the rest of the test cases for test_disttag.py
* Finish the rest of the test_metadata.py test cases
* Fix a typo in tests/baseclass.py
* Add IPv6 blacklist check
* emptyrpm inspection works for single builds
* Remove list_split() prototype; function removed
* Change HEADER_EMPTYRPM to 'empty-payload'
* Use headerGetString() in inspect_emptyrpm.c, set OK to NOT_WAIVABLE
* Finished the test_emptyrpm.py test cases
* Use headerGetString instead of headerGetAsString in builds.c
* Allow -k on test suite rpminspect runs; do not auto add -devel pkgs
* Fix some fundamental problems with the peer detection code.
* Always report payloads remaining empty between builds.
* Fix TestExistingPkgMissing in the test suite.
* Update AUTHORS file
* Comment updates in strfuncs.c
* Add 'desktop_icon_paths' setting to rpminspect.conf
* Fix a number of bugs in the desktop inspection.
* Almost on the Icon checking code.
* Fix segfault when RPM path contains ..
* Fix the Icon file check in the desktop inspection.
* In get_rpm_info(), still do headerFree() even if add_peer fails.
* add_peer() should return 0 even if before and/or after files are empty.
* If json.loads() raises a decode error, show what was done.
* Reserve 0 and 1 for inspection indicator, 2 means program error (#57)
* Add support for the new >= 1.14.5 mandoc API (#15)
* Show Java .jar file containing the .class file (#56)
* s can be NULL when strreplace() is called, it's just a no-op
* Increase Python test suite timeout to 300 seconds
* Add tests/test_desktop.py to the integration test suite.
* Convert spec dependencies to package names.
* Stubbed out test_manpage.py test cases.
* New release (0.8)


Changes in rpminspect-0.7
-------------------------

* Fixes to release.sh getting the Github asset ID for uploads.
* New integration test suite using the Python unittest module.
* Use meson rather than ninja to invoke the test suites in Travis-CI
* Skip the valgrind run for tests at the moment in Travis-CI.
* Start disttag integration tests.
* Fix a number of bugs in the disttag inspection.
* Make sure rpmfluff is installed for Travis-CI tests.
* Install rpmfluff from pip rather than the Fedora repo.
* Need rpm-build installed for rpmfluff.
* Need to see what the value is in Travis-CI
* pip -> pip-3 in .travis.yml
* Set RPMINSPECTCONF env var for running the integration tests.
* Pull in RPMINSPECTCONF in RequiresRpminspect in baseclass.py
* Pass -c to rpminspect in the disttag tests.
* *sigh*
* Minor fixes to the inspect_disttag.c code.
* disttag inspection tests for the integration test suite
* Move more common test stuff to baseclass.py
* Rewrite test_disttag.py to use TestSRPM and TestRPMs.
* Expand the RequiresRpminspect class in the test suite.
* Use do_make() instead of make() method on rpmfluff.
* Fix the 'license' inspection on catching unapproved licenses.
* Integration tests for the 'license' inspection.
* Reclassify a number of inspections as valid for single builds.
* In the 'specname' inspection, correct return values.
* Integration tests for the 'specname' inspection
* Expand baseclass.py for the test suite, add TestKoji
* Update test_command.py for changes made in baseclass.py
* Add 'metadata' inspection integration tests.
* Stub out additional test cases for disttag, license, and specname.
* Stub out some build comparison tests for test_metadata.py
* Stub out before and after base classes for the integration tests.
* Don't assume we have rpmfluff in baseclass.py
* Revert "Don't assume we have rpmfluff in baseclass.py"
* Install dependencies to run the test suite for Copr builds.
* Install python3-pip and then use that to install rpmfluff in Copr.
* Support -Dtests=false as a way to skip the test suite.
* Use git archive when building on Copr
* Forgot 'HEAD' at the end of the git archive command.
* Sync up .copr/Makefile with rpminspect-data-fedora
* --ouput -> --output
* Update release.sh and add srpm.sh from the continuous-goodies project.
* Rename installate-packages.sh to instpkgs.sh
* Change the API for the add_result() function and add threshold.
* Support runtime override of the result threshold (#47)
* Move release.sh and srpm.sh to utils/
* Change the URL mentioned in the %changelog section
* docs: update README with correct ninja-build package name
* Do not mask upstream source changes (#51)
* Fix threshold-controlled exit code (#50)
* Be smarter about reporting the package epoch in inspect_upstream.c
* Write the rest of the test_specname.py tests.
* The upstream inspection requires a before and after build.
* Revert "The upstream inspection requires a before and after build."
* Simplify the upstream inspection more.
* Add base test classes for comparing SRPMs, RPMs, and Koji builds.
* Do not pass -T to rpminspect if self.inspection is not set.
* Add a set of default integration tests.
* Correct Jim Bair's name in AUTHORS
* Add logo PNG to docs/ directory.
* Match approved license tags with spaces in them (#53)
* Support License tags with abbreviations with spaces and booleans.
* Add mkrpmchangelog.sh to utils/
* Update the README
* New release (0.7)


Changes in rpminspect-0.6
-------------------------

* In addedfiles, use RESULT_VERIFY for new security files.
* Update the README
* Modify pic_bits.sh to write to an output file.
* Do not include config.h in badwords.c, define _GNU_SOURCE
* Do not include config.h in any of the source files in librpminspect.
* Define PACKAGE_VERSION when building librpminspect.
* Include meson.build in EXTRA_DIST
* Do not include config.h in unit test source files.
* Use meson in rpminspect.spec rather than autotools/make/libtool
* Add support for build with Meson.
* Do not include config.h in the rpminspect source files.
* Define _GNU_SOURCE and PACKAGE_VERSION when building rpminspect.
* Still using autotools for Travis-CI for the moment.
* Add -D_GNU_SOURCE to CFLAGS for the unit test suite.
* Add version to library() in meson.build
* Begin Travis-CI change over to meson+ninja.
* Enable -Werror for builds in Travis-CI, wrap tests in valgrind.
* Adjust test run for Travis-CI, enable coverage reports.
* Remove most autotools files.
* Fix coverage command in .travis.yml
* Change over to gcov for coverage, disable lcoveralls for now.
* New release.sh script, remove final Makefile.am
* Add cpp-coveralls to get gcov reports to coveralls.io
* Remove incorrect 'gcov' from Dockerfile.test
* Install gcovr in Dockerfile.test
* Make sure cpp-coveralls is installed in both places.
* Fix the 'noexecstack' and 'execstack' test executables.
* Expand out the release.sh script and give it option handling.
* Remove the old make-release.sh, replaced by release.sh
* Update the README
* More attempts at getting cpp-coveralls to work
* Disable pip-3 in Travis-CI for now.
* Run the test suite in Travis-CI
* Install python3-pip in the Fedora Docker image.
* Install pip-3 in the Travis-CI image.
* Correct the pip usage in the Travis-CI script.
* OK, just call it 'pip'
* Alright, finally try to run the tests wrapped in valgrind.
* Some problems using --wrap with valgrind, might be old meson.
* Fix the valgrind test run in meson.build and .travis.yml
* Adjust .copr/Makefile to work with meson.build
* Minor updates to the TODO file
* Make sure meson is installed for copr.
* Rename tests/librpminspect/ to tests/lib/
* Rename src/librpminspect/ to lib/
* Move src/rpminspect/ source files to src/
* Run installate-package.sh from .copr/Makefile
* OK, we do need to manually install meson.
* And gcc is needed to run meson.
* Install all of the BuildRequires so meson runs for Copr builds.
* More fixes for our Copr builds.
* Add the 'upstream' inspection to librpminspect.
* result can be NULL when calling run_cmd()
* In 'changedfiles', ignore /usr/lib/debug and /usr/src/debug
* Discontinue use of cpp(1) in changedfiles.
* Always combine stdout and stderr in run_cmd()
* Return the correct exit code in main() (#39)
* Allow text output mode to work when it can't get a terminal width. (#42)
* Make -Werror=format-security happy (#44)
* In the 'metadata' inspection, do not assume the Vendor tag is set. (#43)
* Add new config file settings for the ownership inspection.
* Add the 'ownership' inspection to librpminspect.
* Make sure the 'specname' inspection returns the correct value (#45)
* Update requirements in the README file
* Guard a few TAILQ_EMPTY calls on possible NULL pointers.
* Return correct result from inspect_emptyrpm()
* Return correct result from inspect_metadata()
* Ignore .pyc and .pyo Python bytecode files in 'changedfiles'
* Remove desktop_file_validate from rpminspect.conf
* Add 'shells' setting to rpminspect.conf, default in constants.h
* Replace libcap usage with libcap-ng
* Drop old <sys/capability.h> include line.
* Code cleanups caught by clang (#37)
* Add the 'shellsyntax' inspection.
* Add contrib report-json2html.py
* Update the release.sh script.
* More release.sh script fixes for Github uploads.
* Handle the -a/--all option in release.sh
* Minor changes to release.sh
* More fixes for release.sh, getting closer.
* New release (0.6)


Changes in rpminspect-0.5
-------------------------

* Stub out the Github uploading parts of make-release.sh
* Update the README's mention of the -f option.
* Support running rpminspect on local RPM packages (#23)
* Add get_elf_soname() to readelf.c
* Put an extra empty line between Screendump and Remedy
* Add the 'removedfiles' inspection now.
* Remove WAIVABLE_BY_RELENG from librpminspect
* Remove WAIVABLE_BY_RELENG from test-strfuncs.c
* Define C/C++ header file extensions in rpminspect.conf
* Correct argument order for strprefix() use in inspect_changedfiles.c
* Define and use more filename related constants.
* Update README to include dnf install instructions
* Fix SEGV in get_product_release
* Start integration test suite in the tests/ subdirectory.
* Fix gcc -Werror errors as caught by Travis-CI.
* Fix distcheck
* Create CODE_OF_CONDUCT.md
* Update TODO list
* Rename /usr/share/rpminspect/setuid to /usr/share/rpminspect/st_mode
* Rename again for data/st_mode to data/stat-whitelist
* Expand struct rpminspect with 'stat_whitelist_dir'
* Add stat_whitelist to struct rpminspect.
* Implement the 'addedfiles' inspection.
* Add regression test for segfault
* Fix config file detection
* Fix errors caught by Travis-CI
* Update the AUTHORS file.
* Update the TODO list.
* Properly free the stat_whitelist, fix init_stat_whitelist()
* New release


Changes in rpminspect-0.4
-------------------------

* Support multiple buildhost subdomains in rpminspect.conf (#25)
* Explain how to use rpminspect in the README file. (#24)
* Note that buildhost_subdomain can list multiple subdomains.
* Add support for specifying architecture list on the command line (#27)
* Hook up the -a/--arches command line argument to downloads (#27)
* Make sure local builds allow 'src' as an architecture.
* Code cleanup.
* Remove unnecessary commented out code.
* doc: fix typos
* Split the -T option functionality in to -T and -E (#28)
* Add get_mime_type() in magic.c in librpminspect
* Fix up some #include lines in librpminspect
* Add run_cmd() to librpminspect, a popen() handler.
* Remove validate_desktop_file() from inspect_desktop.c
* Add checksum() function to librpminspect
* Define new config file parameter:  security_path_prefix
* Add the 'changedfiles' inspection.
* Remove unused variables
* Improve the -f/--fetch-only mode.
* Add utils/make-release.sh script
* Update release instructions in the README
* New release


Changes in rpminspect-0.3
-------------------------

* Set XMLRPC_XML_SIZE_LIMIT_ID to INT_MAX.
* Set CURLOPT_TCP_FASTOPEN to 1 for curl downloads.
* s/\.dist/archive/g in unpack.c
* Hard links extract but return !ARCHIVE_OK from archive_read_extract() (#19)
* Fix hard link extraction in extract_rpm() (#19)
* Trim trailing slashes from release strings.
* Strip extra () after bugfix for issue #19
* Prevent the desktop inspection from segfaulting for comparisons.
* Support ~ expansion for --workdir=PATH specifications (#22)
* Update github URLs
* Increment version for release


Changes in rpminspect-0.2
-------------------------

* Stop using regular expressions for the badwords test.
* Add more badwords tests
* Go back to linking elftest with -no-install
* Rearrange tests/librpminspect/Makefile.am
* Run the CI tests with -Wall -Werror
* Fix warnings when compiling with -DNDEBUG
* Remove special handling for the before and after SRPM packages.
* During the manpage inspection, report BAD if the db is missing
* Clean up the text output function in librpminspect.
* Skip source packages in _manpage_driver()
* Skip source packages in _is_desktop_entry_file()
* Skip source packages in _elf_driver()
* Skip source packages in _xml_driver()
* Skip a line before 'Keeping working directory' output
* For RESULT_OK inspections, do not output the message.
* Add the 'disttag' inspection.
* Add the 'specname' inspection.
* When result msgs exists in output_text.c, skip a line before the Result
* Drop log file check from is_local_build()
* Handle XMLRPC_TYPE_NIL values when gathering build information.
* Support module metadata in get_koji_build() and struct koji_build
* Extend rpminspect.conf to allow different Koji download URLs
* Extend Koji build fetching to get module packages
* Update README
* Recognize empty RPMs in extract_rpm()
* Add the 'modularity' test to librpminspect
* Drop unnecessary length assert in _printword()
* Move the koji build type value to the struct rpminspect structure
* Expand the modularity inspection to return RESULT_BAD if met.
* Update the TODO list
* s/_download_rpms/_download_artifacts/g
* Download and sort modulemd*txt files for KOJI_BUILD_MODULE types.
* Fix copr builds so the snapshot builds use the right tarball name.
* For Copr builds, modify the %setup line in %prep
* Make list_to_table() a non-static function.
* Update README, I've got things to say.
* rpminspect grew a dependency:  libyaml
* Support filtered RPMs as specified in modulemd.txt metadata files.
* Add fetch-only mode (-f) to rpminspect
* Adjust the Release value for copr builds.
* Despair, Inc. is not globally known.
* In is_payload_empty(), handle a NULL filelist.
* Type corrections for Koji communcation.
* Add unpack.c to librpminspect, which gives us unpack_archive()
* Increase max number of open directories in _validate_desktop_contents()
* Bug fix in extract_rpm(): can't use headerIsEntry() on RPMTAG_FILENAMES
* Use TAILQ_EMPTY() to determine if the after RPM has an empty payload.
* Add current state of the javabytecode inspection.
* Check the JVM major number from the right position.
* Add product_release member to struct rpminspect.
* rpminspect now has the -r/--release option for product release.
* Add code to librpminspect to support the [javabytecode] block.
* Define example [javabytecode] block in rpminspect.conf
* In inspect_javabytecode, fill out more of the tests.
* Add __attribute__ ((fallthrough)) where switch falls through on -f
* eptr->data is a string here
* Prevent builds with mandoc >= 1.14.5
* Simplify the HEADER_* strings.
* Do not crash if missing the [javabytecode] block in rpminspect.conf (#16)
* Explain the jvm_table and jvm_keys structures a bit more.
* Move JVM version checking to check_class_file()
* Run .class file checks for .jar files.
* Extend the javabytecode test to compare before/after JVM versions.
* Remove unused variable in inspect_javabytecode.c
* Reformat some TODO file lines
* Move to fedora:30 in the Dockerfile.test file.
* BuildRequires: make in rpminspect.spec.in
* Expand 'make release' in Makefile.am
* Explain the release process.
* Change the release process a bit.
* Prepare release v0.2


Changes in rpminspect-0.1
-------------------------

* Initial import.
* Add autotools build system files.
* Add COPYING file with GPLv3 in it.
* Make sure all of librpminspect includes config.h
* Pick up dependencies with pkg-config scripts.
* Add src/rpminspect/ to the SUBDIRS list so it builds.
* Use PACKAGE_VERSION in koji.c
* Include config.h in builds.c
* Use PACKAGE_VERSION and include config.h in rpminspect.c
* Update README
* Fix remaining autotools problems.
* Update README file.
* Update TODO file.
* Install header files for librpminspect.
* Start an rpminspect.spec.in template file.
* Include missing files in the 'make dist' target and install things.
* Add utility functions for kernel module alias checking
* Fix the list functions when used with empty lists.
* Fix a bug in list_union.
* Ensure librpminspect is linked against libdl
* Round out the autotools files a bit more.
* Add Copr build control files.
* Install autogen.sh requirements for .copr/Makefile:srpm
* Install the spec file BuildRequires for copr builds.
* Slight change to installate-packages.sh for copr.
* Run autogen.sh from the spec file if we need to.
* Update the .copr/Makefile and how it makes SRPM files.
* Install git for copr builds.
* Fix the hash table usage in extract_rpm.
* Add file peer matching.
* Fix the comment on get_elf_imported_functions
* Return an empty list instead of NULL for no symbols.
* Add ELF comparison tests
* Only call find_file_peers() when you have before and after builds.
* Fix hardlink extraction in extract_rpm()
* Begin unit test suite using CUnit.
* Ignore test suite files we don't want to commit to the git repo.
* Add missing function prototypes for koji.c to rpminspect.h
* Add unit tests for applicable functions in koji.c in librpminspect
* Add unit tests for tty.c in librpminspect
* Modify printwrap() to return the number of lines printed.
* Add test-strfuncs.c to the librpminspect unit test suite
* Remove compression.c from librpminspect
* Fix an error in init_rpminspect()
* Add test-free.c and test-init.c to the librpminspect unit tests.
* Begin test-inspect_elf.c unit tests.
* Ignore more files from version control.
* Fix .gitignore
* Add some unused pkg-config variables
* Fix the pic_bits.sh call
* Fail 'make check' if CUnit is unavailable.
* Add a Travis configuration
* Add a check-code-coverage target.
* Simplify the coveralls upload
* Fix the test suite runners.
* Run the travis tests with VERBOSE=1
* Remove the tests for free functions.
* Fix the execstack tests
* Allow for skipped tests in the test runner
* Skip test_tty_width if stdin is not a tty
* Add a valgrind check
* Fix free_rpminspect
* Add valgrind to the test environment
* Check if cfgfile is still NULL after realpath().
* Drop MPARSE_SO for man page validation, clean up messages (#5)
* Use config.h in the test files.
* Improve the assertion error messages.
* Fix the xasprintf macro
* Add a function to do string replacement.
* Add the static keyword to local functions in files.c
* Add version substitution to peer file matching.
* Save the libarchive entry path in the file struct
* Use localpath instead of fullpath for file peer matching
* Start a collections of docs.
* Fix config file finding.
* Store the 'epoch' and 'size' values for each RPM.
* If the volume name is 'DEFAULT', do not add it to the koji URL.
* Report disappearing subpackages from INSPECT_EMPTYRPM
* Skip disappearing subpackages in INSPECT_LICENSE
* Add the 'desktop' inspection to librpminspect
* Add config file settings for the 'desktop' test.
* Add a Requires for /usr/bin/desktop-file-validate
* Update .gitignore file.
* Complete test-inspect_elf.c for all but two tests.
* Make sure test-inspect_elf.c opens helper files correctly.
* Allow the -T option in rpminspect(1) to work with negated values.
* Adjust package Summary and set base Release to '1'.
* Remove stray tabs in Makefile.am for librpminspect
* Explain 'use_volume_name' in default rpminspect.conf file
* Incorporate fixes mentioned in Fedora package review bug
* Update spec template and build scripts to better comply with policy
* Update Copr automated build process
* Replace %autosetup with %setup
* I don't know how rpm macros work in spec files
* Add hours and minutes to the timestamp in GITDATE
* Exit installate-package.sh if you are not root.
* Extend GITDATE with hours and minutes
* Report RESULT_OK for passing inspections.
* Set version to 0.1.
* Add a 'make release' target
* Ignore detached GPG signature files
