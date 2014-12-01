#!/bin/bash

# Must be run on eb24621, from ~/Live_Project/

dataSetSize=50000000
dataSetName="data.dat"
controlSort="sorted_control_$dataSetName"
projectDir="/home/student/emd0003/Live_Project"

#Deploy the clients first thing so they have time to launch
echo "Deploying clients for distributed run"
cd $projectDir/Distributed/
bash deploy.sh 

# Generated new data set
echo "Generating random data set of size $dataSetSize"
cd $projectDir/gen_data
./random_generator $dataSetSize $dataSetName

# Generate a control sort on the new data set
sort -n $dataSetName >> $controlSort

# Run for distributed version
echo "Running distributed server"
cd $projectDir/Distributed/
./server_emd0003 $dataSetName

# Run for serial version
echo "Running serial version"
cd $projectDir/Serial/
./serial_merge $dataSetName

# Diff the control and my sorts
cd $projectDir/gen_data
diff $controlSort $projectDir/Serial/sorted_$dataSetName
diff $controlSort $projectDir/Distributed/sorted_$dataSetName

# Cleanup
echo "Cleaning up data set files"
cd $projectDir/gen_data
rm *dat

echo "Sleeping for 2 minutes to allow time for clients to release socket"
sleep 2m
