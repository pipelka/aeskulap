#!/bin/bash

AUTOCONF_REQUIRED_VERSION=2.52
AUTOMAKE_REQUIRED_VERSION=1.8

cd `dirname $0`
TOPDIR=`pwd`

# check for proper versions

check_version ()
{
    if expr $1 \>= $2 > /dev/null; then
        echo "yes (version $1)"
    else
        echo "Too old (found version $1)!"
        DIE=1
    fi
}

echo -n "checking for autoconf >= $AUTOCONF_REQUIRED_VERSION ... "
if (autoconf --version) < /dev/null > /dev/null 2>&1; then
    VER=`autoconf --version | grep -iw autoconf | sed "s/.* \([0-9.]*\)[-a-z0-9]*$/\1/"`
    check_version $VER $AUTOCONF_REQUIRED_VERSION
else
    echo
    echo "  autoconf version >= $VER $AUTOCONF_REQUIRED_VERSION must bei nstalled "
    echo "  to compile aeskulap."
    echo "  Download the appropriate package for your distribution,"
    echo "  or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
    exit 1
fi

echo -n "checking for automake >= $AUTOMAKE_REQUIRED_VERSION ... "
if (automake --version) < /dev/null > /dev/null 2>&1; then
    VER=`automake --version | grep -iw automake | sed "s/.* \([0-9.]*\)[-a-z0-9]*$/\1/"`
    check_version $VER $AUTOMAKE_REQUIRED_VERSION
else
    echo
    echo "  You must have automake 1.8 or newer installed to compile aeskulap."
    echo "  Get ftp://ftp.gnu.org/pub/gnu/automake/automake-1.8.5.tar.gz"
    echo "  (or a newer version of 1.8 if it is available; note that 1.9 is buggy)"
    exit 1
fi

# process dcmtk

echo "Preparing dcmtk ..."

cd dcmtk/config
sh ./autoall
sh ./rootconf

cd $TOPDIR


echo "Generating build information ..."
aclocalinclude="$ACLOCAL_FLAGS"

echo "Running aclocal $aclocalinclude ..."
aclocal $aclocalinclude || {
    echo
    echo "**ERROR**: aclocal failed. This may mean that you have not"
    echo "installed all of the packages you need, or you may need to"
    echo "set ACLOCAL_FLAGS to include \"-I \$prefix/share/aclocal\""
    echo "for the prefix where you installed the packages whose"
    echo "macros were not found"
    exit 1
}

echo "Running autoheader ..."
autoheader || {
    echo "***ERROR*** autoheader failed."
    exit 1
}

echo "Running libtoolize ..."
libtoolize --automake -c || {
    echo
    echo "**ERROR**: intltoolize failed. This may mean that you have not"
    echo "installed all of the packages you need. Please install the"
    echo "'libtool' package."
    exit 1
}

echo "Running automake ..."
automake -c --foreign --add-missing || {
    echo "***ERROR*** automake failed."
    exit 1
}

echo "Running autoconf ..."
autoconf || {
    echo "***ERROR*** autoconf failed."
    exit 1
}

echo "Running glib-gettextize ..."
glib-gettextize --copy --force > /dev/null || {
    echo
    echo "***ERROR*** glib-gettextize failed."
    exit 1
}

echo "Running intltoolize ..."
intltoolize -c -f --automake || {
    echo
    echo "***ERROR* intltoolize failed."
    exit 1
}

chmod +x `find . -name configure`
chmod +x `find . -name mkinstalldirs`

echo
echo "Please run ./configure now."
