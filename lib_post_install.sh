#!/bin/sh

UNAME=`uname`
MACHINE=`uname -m`
VERSION="0.6.0"

echo "Creating links for $UNAME $MACHINE $VERSION"

if [ "$UNAME" = "Darwin" ]; then
	LIBDIR=/usr/lib

	SYNTROGUI="$LIBDIR/libSyntroGUI.$VERSION.dylib"
	SYNTROGUI_SONAME="$LIBDIR/libSyntroGUI.0.dylib"
	SYNTROGUI_LINKNAME="$LIBDIR/libSyntroGUI.dylib"
	
	SYNTROLIB="$LIBDIR/libSyntroLib.$VERSION.dylib"
	SYNTROLIB_SONAME="$LIBDIR/libSyntroLib.0.dylib"
	SYNTROLIB_LINKNAME="$LIBDIR/libSyntroLib.dylib"

	SYNTROCONTROLLIB="$LIBDIR/libSyntroControlLib.$VERSION.dylib"
	SYNTROCONTROLLIB_SONAME="$LIBDIR/libSyntroControlLib.0.dylib"
	SYNTROCONTROLLIB_LINKNAME="$LIBDIR/libSyntroControlLib.dylib"

elif [ "$MACHINE" = "armv7l" ]; then
# Our qmake generated Makefile is temporarily broken for arm platforms
# like the Gumstix.
# Fixing what the install should have done here.  

	echo "Doing special armv7l fixup"

	LIBDIR=/usr/lib

	SYNTROGUI="$LIBDIR/libSyntroGUI.so.$VERSION"
	SYNTROGUI_SONAME="$LIBDIR/libSyntroGUI.so.0"
	SYNTROGUI_LINKNAME="$LIBDIR/libSyntroGUI.so"
	
	SYNTROLIB="$LIBDIR/libSyntroLib.so.$VERSION"
	SYNTROLIB_SONAME="$LIBDIR/libSyntroLib.so.0"
	SYNTROLIB_LINKNAME="$LIBDIR/libSyntroLib.so"

	SYNTROCONTROLLIB="$LIBDIR/libSyntroControlLib.so.$VERSION"
	SYNTROCONTROLLIB_SONAME="$LIBDIR/libSyntroControlLib.so.0"
	SYNTROCONTROLLIB_LINKNAME="$LIBDIR/libSyntroControlLib.so"

	SRCGUI="SyntroGUI/libSyntroGUI.so.$VERSION"

	if [ -f $SRCGUI ]; then
		echo "Copying $SRCGUI to $SYNTROGUI" 
		cp $SRCGUI $LIBDIR
		echo "Stripping $SYNTROGUI"
		/usr/bin/strip $SYNTROGUI	
	else
		echo "$SRCGUI not found"
		exit
	fi 

	SRCLIB="SyntroLib/libSyntroLib.so.$VERSION"

	if [ -f $SRCLIB ]; then
		echo "Copying $SRCLIB to $SYNTROLIB" 
		cp $SRCLIB $LIBDIR
		echo "Stripping $SYNTROLIB"
		/usr/bin/strip $SYNTROLIB
	else
		echo "$SRCLIB not found"
		exit
	fi 

	SRCLIB="SyntroControlLib/libSyntroControlLib.so.$VERSION"

	if [ -f $SRCLIB ]; then
		echo "Copying $SRCLIB to $SYNTROCONTROLLIB" 
		cp $SRCLIB $LIBDIR
		echo "Stripping $SYNTROCONTROLLIB"
		/usr/bin/strip $SYNTROCONTROLLIB
	else
		echo "$SRCLIB not found"
		exit
	fi 

elif [ "$UNAME" = "Linux" ]; then
	LIBDIR=/usr/lib

	SYNTROGUI="$LIBDIR/libSyntroGUI.so.$VERSION"
	SYNTROGUI_SONAME="$LIBDIR/libSyntroGUI.so.0"
	SYNTROGUI_LINKNAME="$LIBDIR/libSyntroGUI.so"
	
	SYNTROLIB="$LIBDIR/libSyntroLib.so.$VERSION"
	SYNTROLIB_SONAME="$LIBDIR/libSyntroLib.so.0"
	SYNTROLIB_LINKNAME="$LIBDIR/libSyntroLib.so"

	SYNTROCONTROLLIB="$LIBDIR/libSyntroControlLib.so.$VERSION"
	SYNTROCONTROLLIB_SONAME="$LIBDIR/libSyntroControlLib.so.0"
	SYNTROCONTROLLIB_LINKNAME="$LIBDIR/libSyntroControlLib.so"

else
	echo "Unknown O/S $UNAME"
	exit
fi

# this might not work for Mac
PKGCONFIGDIR="$LIBDIR/pkgconfig"


if [ -d $PKGCONFIGDIR ]; then
    cp syntro.pc $PKGCONFIGDIR
else
    echo "You need to manually copy syntro.pc to the pkgconfig directory!"
fi


if [ ! -f $SYNTROGUI ]; then
	echo "$SYNTROGUI not found"
else
	if [ ! -f $SYNTROGUI_SONAME ]; then
		ln -s $SYNTROGUI $SYNTROGUI_SONAME	
	fi

	if [ ! -f $SYNTROGUI_LINKNAME ]; then
		ln -s $SYNTROGUI $SYNTROGUI_LINKNAME
	fi	
fi	

if [ ! -f $SYNTROLIB ]; then
	echo "$SYNTROLIB not found"
else
	if [ ! -f $SYNTROLIB_SONAME ]; then
		ln -s $SYNTROLIB $SYNTROLIB_SONAME	
	fi

	if [ ! -f $SYNTROLIB_LINKNAME ]; then
		ln -s $SYNTROLIB $SYNTROLIB_LINKNAME 
	fi	
fi	


if [ ! -f $SYNTROCONTROLLIB ]; then
	echo "$SYNTROCONTROLLIB not found"
else
	if [ ! -f $SYNTROCONTROLLIB_SONAME ]; then
		ln -s $SYNTROCONTROLLIB $SYNTROCONTROLLIB_SONAME	
	fi

	if [ ! -f $SYNTROCONTROLLIB_LINKNAME ]; then
		ln -s $SYNTROCONTROLLIB $SYNTROCONTROLLIB_LINKNAME 
	fi	
fi	


