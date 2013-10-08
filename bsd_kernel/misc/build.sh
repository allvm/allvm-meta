#!sudo /bin/sh

mv -f build.log build.old.log 2>&1 > /dev/null || /bin/true

# ./clean.sh

make kernel KERNCONF=ALLVM -j5 2>&1 | tee build.log
