#!/bin/sh
set -e

LIBS="runnable-ld.so libpthread.so.e59bca84 libppapi_cpp.so libppapi_gles2.so libstdc++.so.6 libm.so.e59bca84 libgcc_s.so.1 libc.so.e59bca84 libdl.so.e59bca84"

if [ ! -d $NACL_SDK_ROOT ]
then
	echo NACL_SDK_ROOT must be set
	exit 1
fi

mkdir -p build-nacl
cd build-nacl

for HOST in i686-nacl x86_64-nacl
do
	if [[ $1 = clean ]]
	then
		rm -rf $HOST
	fi
	mkdir -p $HOST
	cd $HOST
	if [[ $1 = clean ]]
	then
		../../configure --host $HOST
	fi
	make -j8
	cd ..
done

echo creating app store zip
zip -j oort.zip ../chrome-app/*

mkdir -p www
cd www

echo symlinking Oort binaries
ln -sf ../i686-nacl/oort_nacl oort-i686-nacl.nexe
ln -sf ../x86_64-nacl/oort_nacl oort-x86_64-nacl.nexe

echo symlinking Oort HTML
ln -sf ../../ui/nacl/app.yaml
ln -sf ../../ui/nacl/oort.html
ln -sf ../../ui/nacl/oort.js
ln -sf ../../ui/nacl/oort.css

rm -rf lib32 lib64
mkdir lib32 lib64
for LIB in $LIBS
do
	echo hardlinking lib $LIB
	for BITS in 32 64
	do
		ln -f $NACL_SDK_ROOT/toolchain/linux_x86/x86_64-nacl/lib$BITS/$LIB lib$BITS/
	done
done

exec 3> oort.nmf

cat >&3 <<EOS
{
  "program": {
    "x86-64": {"url": "lib64/runnable-ld.so"},
    "x86-32": {"url": "lib32/runnable-ld.so"}
  },
  "files": {
EOS

for LIB in $LIBS
do
	cat >&3 <<-EOS
    "lib/$LIB" : {
      "x86-64": {"url": "lib64/$LIB"},
      "x86-32": {"url": "lib32/$LIB"}
    },
	EOS
done

cat >&3 <<EOS
    "main.nexe" : {
      "x86-64": {"url": "oort-x86_64-nacl.nexe"},
      "x86-32": {"url": "oort-i686-nacl.nexe"}
    }
  }
}
EOS

exec 3>&-
