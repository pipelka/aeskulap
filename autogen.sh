#!/bin/sh

cd `dirname $0`
TOPDIR=`pwd`

echo "Preparing dcmtk ..."

cd dcmtk/config
sh ./autoall
sh ./rootconf

cd $TOPDIR

echo "Generating build information ..."
aclocalinclude="$ACLOCAL_FLAGS"

echo "Running gettextize .."
gettextize --copy --intl

echo "Running intltoolize ..."
intltoolize -c -f --automake

echo "Running libtoolize ..."
libtoolize -f -c

echo "Running aclocal $aclocalinclude ..."
aclocal -I m4 $aclocalinclude || {
    echo
    echo "**Error**: aclocal failed. This may mean that you have not"
    echo "installed all of the packages you need, or you may need to"
    echo "set ACLOCAL_FLAGS to include \"-I \$prefix/share/aclocal\""
    echo "for the prefix where you installed the packages whose"
    echo "macros were not found"
    exit 1
}

echo "Running autoheader ..."
autoheader || ( echo "***ERROR*** autoheader failed." ; exit 1 )

echo "Running automake ..."
automake -c -a --foreign || ( echo "***ERROR*** automake failed." ; exit 1 )

echo "Running autoconf ..."
autoconf || ( echo "***ERROR*** autoconf failed." ; exit 1 )

chmod +x `find . -name configure`

echo
echo "Please run ./configure now."
