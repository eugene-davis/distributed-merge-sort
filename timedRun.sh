#!/bin/bash

# Must be run on eb24621, from ~/Live_Project/

dataSetSize=50000000
dataSetName="data.dat"
projectDir="/home/student/emd0003/Live_Project"

#Deploy the clients first thing so they have time to launch
echo "Deploying clients for distributed run"
cd $projectDir/Distributed/
bash deploy.sh >/dev/null

# Generated new data set
echo "Generating random data set of size $dataSetSize"
cd $projectDir/gen_data
./random_generator $dataSetSize $dataSetName

# Timed run for serial version
echo "Running serial version"
cd $projectDir/Serial/
time ./serial_merge $dataSetName

# Time run for distributed version
echo "Running distributed server"
cd $projectDir/Distributed/
time ./server_emd0003 $dataSetName

# Cleanup
echo "Cleaning up data set files"
cd $projectDir/gen_data
rm *dat
