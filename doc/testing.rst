Testing
=======

Follow the steps in the Development section and then::

    meson test -C build

You can get verbose build output with::

    meson test -C build -v

The verbose mode is useful when tests fail because you can see the
debugging information dumps and other output.  You may also use the
``Makefile`` to run the test suite or a specific subset of tests.  For
example::

    make check

Runs all of the tests where as::

    make check xml

Will just run the xml inspection tests.  The ``Makefile`` target is
provided as a convenience so you don't need to remember the meson_
commands.

You can also use a Python_ virtualenv_ to run and debug the tests.
Make sure pip_ and virtualenv_ are installed and then::

    virtualenv -p python3 my_test_env
    . my_test_env/bin/activate
    pip-3 install meson ninja rpm-py-installer rpmfluff setuptools
    export RPMINSPECT=$PWD/build/src/rpminspect
    export RPMINSPECT_YAML=$PWD/data/generic.yaml
    export RPMINSPECT_TEST_DATA_PATH=$PWD/test/data

To run all tests, execute the command::

    python3 -Bm unittest discover -v test/

To run a specific test suite, execute::

    python3 -Bm unittest discover -v test/ test_emptyrpm.py

.. _meson: https://mesonbuild.com/

.. _Python: https://www.python.org/

.. _virtualenv: https://docs.python.org/3/library/venv.html

.. _pip: https://pypi.org/project/pip/
