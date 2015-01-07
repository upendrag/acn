export LD_LIBRARY_PATH=lib/

./bin/controller &
./bin/brouter 01 01 01 01 03 01 &
./bin/brouter 02 01 01 02 04 01 &
./bin/brouter 03 02 04 02 02 01 &
./bin/brouter 04 02 02 02 05 01 &
./bin/brouter 05 03 03 02 06 01 &
./bin/brouter 06 03 06 02 05 02 &
