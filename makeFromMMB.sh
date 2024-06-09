#!/bin/bash
cd ~/github/MMB/build
make -j4
sudo make install
cd ~/svn/breeder/build
g++ ../src/BreederParameterReader.cpp ../src/Breed.cpp ../src/Chromosome.cpp ../src/DBManager.cpp ../src/MysqlConnection.cpp ../src/SQLite.cpp ../src/main.cpp -fPIC -shared -o ./libBreeder.so -I/home/sam/svn/breeder/include  -I/usr/local//include/ -isystem /usr/local//include/simbody -I/usr/local/openmm/include -I/usr/local/openmm/include/openmm -I/usr/local/openmm/include/openmm/reference -I /usr/local//seqan-library-2.2.0/include/ -I /home/sam/github/MMB/include/ -I /usr/include/mysql -I /usr/include -std=c++14 -D ENABLE_MYSQL -D USE_OPENMM -D USE_OPENMM_REALVEC
sudo mv libBreeder.so /usr/local//lib
g++ -std=c++14 -o breeder -Wl,-rpath,/home/sflores/svn/RNAToolbox/trunk/build/ -L/usr/local/MMB/lib/ -lSimTKmolmodel -L /usr/local/MMB/lib64/ -l SimTKsimbody -L //usr/lib/mysql/lib/ -L /usr/lib64 -L /home/sam/svn/breeder/build -l Breeder -l MMBlib -l OpenMM -l sqlite3 -l mysqlclient -D USE_OPENMM -D USE_OPENMM_REALVEC

sudo mv breeder /usr/local//bin/breeder
cd ~/svn/homologyScanner/build
/usr/bin/g++ /home/sam/svn/homologyScanner/src/PrimaryJobData.cpp -o PrimaryJobData.o /home/sam/svn/homologyScanner/src/main.cpp -o /home/sam/svn/homologyScanner/build/homologyScanner -std=gnu++14 -I/home/sam/svn/homologyScanner/include -I/home/sam/svn/RNAToolbox//include -I/usr/local/MMB/include/ -isystem /usr/local/MMB/include/simbody  -I /usr/local/seqan-library-2.2.0/include/ -I /home/sam/svn/breeder/include -I /usr/include/mysql -I /usr/local/include/MMB -I /usr/local/include/simbody/ -I /usr/local/openmm/include/openmm/reference/ -I /usr/local/openmm/include/openmm/ -I /usr/local/openmm/include -I /usr/local/openmm/include/openmm/internal/ -Wl,-rpath,/home/sam/svn/RNAToolbox//build/ -L/home/sam/svn/RNAToolbox//build -L/usr/local/MMB/lib/ -lSimTKmolmodel -L /usr/local/MMB/lib64/ -l SimTKsimbody -l SimTKcommon -L /home/sam/svn/RNAToolbox//build/ -l MMBlib -L /usr/local/openmm/lib/ -l OpenMM -L /home/sam/svn/breeder/build/ -l Breeder -L /home/sam/mysql-5.7.13-linux-glibc2.5-x86_64/lib -l:libmysqlclient.so.20 -L /usr/lib64 -l sqlite3 -l curl -D ENABLE_MYSQL -D USE_OPENMM -D USE_OPENMM_REALVEC 

sudo mv homologyScanner /usr/local/bin/

