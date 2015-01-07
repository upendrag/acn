export LD_LIBRARY_PATH=lib/

./bin/controller &
./bin/brouter 01 01 01 01 04 01 &
./bin/irouter 02 01 02 02 01 &
./bin/irouter 03 02 02 03 01 &
./bin/brouter 04 01 03 02 04 04 &

./bin/brouter 05 02 04 02 05 01 &
./bin/brouter 06 02 04 03 07 01 &
./bin/irouter 07 05 02 06 01 &
./bin/irouter 08 06 02 07 02 &
