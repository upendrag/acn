export LD_LIBRARY_PATH=lib/

./bin/controller &
./bin/irouter 01 01 01 02 01 &
./bin/irouter 02 01 02 03 01 &
./bin/irouter 03 02 02 04 01 05 01 &
./bin/irouter 04 04 02 03 02 05 02 &
./bin/irouter 05 06 01 05 03 & 
