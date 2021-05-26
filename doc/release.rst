Release Process
===============

The ``make release`` command creates a new release.  The command
handles the version number change, tests, git tag, and repo push.
Release entries on GitHub_ are created manually.  The signed archive
is attached manually to the release entry.

The ``make koji`` command submits official builds for `Fedora Linux
<https://getfedora.org>`_ and EPEL_.

.. _EPEL: https://fedoraproject.org/wiki/EPEL

.. _GitHub: https://github.com/
