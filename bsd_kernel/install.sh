#!/bin/sh

ROOT=$PWD

# symlink files into system where they belong

# "ALLVM" kernel configuration
sudo ln -sf $ROOT/ALLVM /usr/src/sys/amd64/conf/

# /etc files:
sudo ln -sf $ROOT/etc/* /etc/

# Kernel build files:
cd $ROOT/sys
find . -type file -print0|xargs -0 -I% -n1 sudo ln -sf $ROOT/sys/% /usr/src/sys/%
cd $ROOT

sudo ln -sf $PWD/sys/conf/* /usr/src/sys/conf/

# Misc other kernel build files:
sudo ln -sf $PWD/sys/linux/modules/* /usr/src/

# Helper scripts for building kernel/etc
sudo ln -sf $PWD/misc/build.sh /usr/src/
sudo ln -sf $PWD/misc/clean.sh /usr/src/

