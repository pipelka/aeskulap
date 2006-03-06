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

#echo "Running autopoint ..."
#autopoint -f || ( echo "***ERROR*** autopoint failed." ; exit 1 )


echo "Running libtoolize ..."
libtoolize -f -c || {
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
glib-gettextize --copy --force || {
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
