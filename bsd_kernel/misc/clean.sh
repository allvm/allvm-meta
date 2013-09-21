#!/bin/sh

sudo chflags -R noschg /usr/obj/usr
sudo rm -rf /usr/obj/usr

cd /usr/src
sudo make cleandir -j4
sudo make cleandir -j4
