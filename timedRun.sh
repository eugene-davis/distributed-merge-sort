#!/bin/bash

# Must be run on eb24621, from ~/Live_Project/

dataSetSize=50000000
dataSetName="data.dat"
projectDir="/home/student/emd0003/Live_Project"

#Deploy the clients first thing so they have time to launch
cd $projectDir/Distributed/
bash deploy.sh

# Generated new data set
cd $projectDir/gen_data
./random_generator $dataSetSize $dataSetName

# Time run for serial version
cd $projectDir/Serial/
time ./serial_merge $dataSetName

# Run distributed version
cd $projectDir/Distributed/
time ./server_emd0003 $dataSetName

# Cleanup
cd $projectDir/gen_data
rm *dat
