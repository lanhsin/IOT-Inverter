# IOT-Inverter

Monitor simulation Inverters.  
Read configuration file of inverters.  
Read data from inverters. Write data to database.

#### Build executable file

1. mkdir build
2. cd build
3. Release : cmake -DCMAKE_BUILD_TYPE=Release ..
4. Debug :   cmake -DCMAKE_BUILD_TYPE=Debug ..
5. make
6. cpack

#### Execution

1. cd build
2. cp ../docs/solar.conf  .
3. ./invIOT

#### Trace Mode

1. cd build
2. cp ../docs/solar.conf  .
3. ./invIOT -T

#### Configuration File

If change configuration file, restart the execution file. 
