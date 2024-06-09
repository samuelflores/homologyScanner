#Download base image. Consider a named distribution of MMB instead.

FROM samuelflores/mmb-ubuntu as MMBImage

# Need mysql, sqlite3, and curl:
RUN apt-get update && apt-get install -y    libmysqld-dev libsqlite3-dev libcurl4-openssl-dev faketime

#not needed for compile, but for execution:
RUN apt-get install mysql-server
RUN service mysql start

   

RUN mkdir  /svn
WORKDIR /svn
# Anonymous access should now be possible:
RUN svn  checkout https://simtk.org/svn/breeder 
RUN svn  checkout https://simtk.org/svn/homologyscanner 
# Build breeder:
RUN g++ ../src/BreederParameterReader.cpp ../src/Breed.cpp ../src/Chromosome.cpp ../src/DBManager.cpp ../src/MysqlConnection.cpp ../src/SQLite.cpp ../src/main.cpp -fPIC -shared -o ./libBreeder.so -I///svn/breeder/include -I/usr/local//include/ -isystem /usr/local//include/simbody -I/usr/local/openmm/include -I/usr/local/openmm/include/openmm -I/usr/local/openmm/include/openmm/reference -I /github/seqan/include/ -I //github/MMB/include/ -I /usr/include/mysql -I /usr/include -std=c++14 -D ENABLE_MYSQL -D USE_OPENMM -D USE_OPENMM_REALVEC
RUN mv libBreeder.so /usr/local//lib/
RUN g++ -std=c++14 -o breeder -Wl,-rpath,/home/sflores/svn/RNAToolbox/trunk/build/ -L/usr/local/MMB/lib/ -lSimTKmolmodel -L /usr/local/MMB/lib64/ -l SimTKsimbody -L //usr/lib/mysql/lib/ -L /usr/lib64 -L /home/sam/svn/breeder/build -l Breeder -l MMBlib -l OpenMM -l sqlite3 -l mysqlclient -D USE_OPENMM -D USE_OPENMM_REALVEC
RUN mv breeder /usr/local//bin/breeder 

#Build homologyScanner :
RUN /usr/bin/g++  /svn/homologyscanner/src/PrimaryJobData.cpp -o PrimaryJobData.o /svn/homologyscanner/src/main.cpp -o /svn/homologyscanner/build/homologyScanner -std=gnu++14 -I/svn/homologyscanner/include -I/github/MMB//include -I/usr/local/MMB/include/ -isystem /usr/local/MMB/include/simbody -I /github/seqan/include -I /svn/breeder/include -I /usr/include/mysql -I /usr/local/include/MMB -I /usr/local/include/simbody/ -I /usr/local/openmm/include/openmm/reference/ -I /usr/local/openmm/include/openmm/ -I /usr/local/openmm/include -I /usr/local/openmm/include/openmm/internal/ -Wl,-rpath,/svn/RNAToolbox//build/ -L/svn/RNAToolbox//build -L/usr/local/MMB/lib/ -lSimTKmolmodel -L /usr/local/MMB/lib64/ -l SimTKsimbody -l SimTKcommon -L /svn/RNAToolbox//build/ -l MMBlib -L /usr/local/openmm/lib/ -l OpenMM -L /svn/breeder/build/ -l Breeder -L /mysql-5.7.13-linux-glibc2.5-x86_64/lib -l:libmysqlclient.so.20 -L /usr/lib64 -l sqlite3 -l curl -D ENABLE_MYSQL -D USE_OPENMM -D USE_OPENMM_REALVEC
RUN mv homologyScanner /usr/local/bin/

RUN mkdir /usr/local/foldx
WORKDIR   /usr/local/foldx
RUN wget   http://pe1.scilifelab.se/MMB-annex/homologyScanner/foldx        
RUN wget   http://pe1.scilifelab.se/MMB-annex/homologyScanner///foldx.original 
RUN wget   http://pe1.scilifelab.se/MMB-annex/homologyScanner/rotabase.txt 


WORKDIR /work                  
