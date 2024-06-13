#!/bin/bash
# New way to compile. This file was created 26 june 2020.
#
cd /home/samuel.flores/github/homologyScanner/build
#
/usr/bin/g++  /home/samuel.flores/github/homologyScanner/src/PrimaryJobData.cpp -o PrimaryJobData.o  /home/samuel.flores/github/homologyScanner/src/main.cpp -o /home/samuel.flores/github/homologyScanner/build/homologyScanner -std=gnu++14 -I/home/samuel.flores/github/homologyScanner/include -I/usr/local//include/ -isystem /usr/local//include/simbody -I /usr/local//include/  -I/usr/local//include/openmm   -I/usr/local//include/openmm/reference   -I  /usr/local/seqan-library-2.2.0/include/ -I /home/samuel.flores/github/breeder/include -I /home/samuel.flores/github/MMB/include/  -I    /usr/include/mysql      -L/usr/local//lib/ -lSimTKmolmodel   -l SimTKsimbody -l SimTKcommon    -l MMB  -l OpenMM  -l Breeder -L  /home/samuel.flores/mysql-5.7.13-linux-glibc2.5-x86_64/lib -l:libmysqlclient.so.20 -L  /usr/lib64  -l sqlite3  -l curl -D ENABLE_MYSQL -D USE_OPENMM -D USE_OPENMM_REALVEC
sudo mv homologyScanner /usr/local//bin/homologyScanner
