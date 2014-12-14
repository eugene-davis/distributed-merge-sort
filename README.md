distributed-merge-sort
======================
Project from CPE 434 at University of Alabama in Hunstville, this software breaks up a set of randomly generated integers into twenty equal pieces, and distributes each of the pieces to one of twenty clients. These clients then perform a parallized merge sort using pthreads, and return their sorted partition to the server.


# Usage Notes
Brief notes on how to use programs and some of their support scripts.
Also note that by turning on or off the preprocessor define statement #define outputFileOption, file outputting for both serial and distributed versions can be turned on or off.
##random_generator
Provides a random number generator, which saves out the specified number of random integers to a file. Run as:

./random_generator num_ints filename
The integers must be between 50 000 000 and 140 000 000.
As an example, to get 50 000 000 integers saved into data.dat:

./random_generator 50000000 data.dat
##serial_merge
Performs a purely serial merge on the data in the file specified. Run as:
./serial_merge filename
For example, to sort data.dat:

./serial_merge data.dat
##Parallel/Distributed
The clients must be deployed before the server is run. Their hostnames must be specified in the configuration file ClientList.conf, which must reside in the same directory as the server binary.
##Client
Performs merge sort on the data it receives from the server. All clients must be running before the server is running. Run as:

./client
##Server
Distributes data to clients, and then merges the sorted lists it receives. Clients must be up before server is run. Takes the data filename as an argument.

./server filename
For example, to sort data.dat

./server data.dat
##Support Scripts
There are a number of support scripts provided to ease the use of the distributed platform. Some of the scripts, timedRun.sh and runTest.sh will deploy both the distributed and serial version for comparisions. The basics of what they do and how to use them are listed below.
###deploy.sh
Deploy.sh reads ClientList.conf, and launches the clients on all of the hosts specified therein. Note that if you have extra hosts in your ClientLists.conf, it will launch these but they will not be contacted by the server, and thus will reside in memory until they are killed. Run as:

bash deploy.sh
kill_clients.sh
The inverse of the deploy.sh script, kill_clients.sh will kill all of the clients in ClientList.conf. It can be used to insure that all clients deploy have shutdown after the server ended (for instance, if extra hostnames were in the ClientList.conf). Run as:

bash kill_clients.sh
###runTest.sh
RunTest.sh will generate a new data set of the size set given to it as an argument, with the name in dataSetName. It will also generate a sorted version of the data via sort -n to compare the results of both the serial and the distributed merge against. Note that the default setting for the variable projectDir must be updated to whatever location the project is stored in.
Additionally, runTest.sh expects the following directories to exist in the project directory:
A directory named gen_data, in which the random_generator must reside. If run in the Linux lab, this must be linked from a local partition (such as /tmp) to avoid running into the block quota for larger files.
RunTest.sh will use the output of sort -n on the data file to confirm the correctness of the resulting sorted data files for both serial and distributed merges, by diffing the files.
To run runTest.sh:

bash runTest.sh num_ints
As an example, to get a data set of size 50 000 000:
bash runTest 50000000
Note that generally rm will have an error saying it cannot remove a file. This is normal behavior, and done to remove files in case of aborted runs.
###timedRun.sh
TimedRun.sh runs the serial and distributed merges against the file ranges listed in the outer for loop. It runs both serial and distributed against five different generated files for each size, measuring their time and storing the time taken to the files distributed_times and serial_times. It is intended to allow automated gathering of timing data. It is run as:

bash timedRun.sh
###Notes on Canceled Runs
Canceled runs will generally require killing all the clients with kill_clients.sh. A few minutes will be needed to allow TCP ports to be properly released after this. Additionally, due to delays in the network, kill_clients.sh may have to be run multiple times to hit all clients – sometimes deploying the clients has a several minute delay.
