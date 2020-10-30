# Source Code Control Notes

### Signed-off-by

Using

```sh
git commit -s
```

to sign-off on commits is preferred, but not required.  See https://developercertificate.org/ for more information.

### Short Log Headers

To categorize commits and make release log generation easier, please use categorization headers on the first line of git commit messages.  The headers the project uses are:

| Prefix | Description |
| ------ | ----------- |
| [bld]  | General release and build process changes |
| [tst]  | Test suite commits |
| [fix]  | General bug fix in the library or frontend program |
| [lib]  | librpminspect feature or significant change |
| [new]  | New inspections or inspection changes (not bug fixes) |
| [cmd]  | rpminspect(1) changes |
| [cfg]  | Config file or data/ file changes |
| [ci]   | Changes to the GitHub Actions CI scripts and files |
| [doc]  | Documentation changes |

This list may expand over time.

NOTE: Short log messages without a header like this will be excluded from release announcements.  That may be appropriate for some commits.
