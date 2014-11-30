#!/bin/bash

# Must be run on eb24621, from ~/Live_Project/

#dataSetSize=60000000
dataSetName="data.dat"
projectDir="/home/student/emd0003/Live_Project"

distributedTimes=$projectDir"/distributed_times"
serialTimes=$projectDir"/serial_times"

for dataSetSize {80000000..10000000..140000000}

	for i in 1 2 3 4 5
	do

		#Deploy the clients first thing so they have time to launch
		echo "Deploying clients for distributed run"
		cd $projectDir/Distributed/
		bash deploy.sh 

		# Generated new data set
		echo "Generating random data set of size $dataSetSize"
		cd $projectDir/gen_data
		./random_generator $dataSetSize $dataSetName

		# Time run for distributed version
		echo "Running distributed server"
		cd $projectDir/Distributed/
		time ./server_emd0003 $dataSetName >> $distributedTimes

		# Timed run for serial version
		echo "Running serial version"
		cd $projectDir/Serial/
		time ./serial_merge $dataSetName >> $serialTimes

		# Cleanup
		echo "Cleaning up data set files"
		cd $projectDir/gen_data
		rm *dat

		echo "Sleeping for 5 minutes to allow time for clients to release socket"
		sleep 5m

	done
done
