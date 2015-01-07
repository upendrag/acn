export LD_LIBRARY_PATH=lib/

./bin/controller &
./bin/brouter 01 01  01 01 04 01 &
./bin/brouter 02 01  01 02 02 01 &

./bin/brouter 03 02  02 02 03 01 &
./bin/brouter 04 02  03 02 05 01 &

./bin/brouter 05 03  04 02 06 01 &
./bin/brouter 06 03  06 02 07 01 09 01 &

./bin/brouter 07 04  07 02 08 01 &
./bin/brouter 08 04  08 02 10 01 05 02 &

./bin/irouter 09 11 01 &
./bin/brouter 10 05 09 02 11 02 &

./bin/irouter 11 12 01 &
./bin/brouter 12 06 12 02 10 02 &
