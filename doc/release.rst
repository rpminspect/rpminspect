Release Process
===============

The ``make release`` command creates a new release.  The command
handles the version number change, tests, git tag, and repo push.

Release entries on GitHub_ are created manually.  The signed archive
is attached manually to the release entry.  Use the ``make announce``
target to generate a text file that can be used for the GitHub_
release entry and the blog post.  The output of ``make announce`` also
needs to be prepended to the CHANGES.md file and included in that
release.

The ``make koji`` command submits official builds for `Fedora Linux
<https://getfedora.org>`_ and EPEL_.

.. _EPEL: https://fedoraproject.org/wiki/EPEL

.. _GitHub: https://github.com/
