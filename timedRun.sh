#!/bin/bash

# Must be run on eb24621, from ~/Live_Project/

dataSetSize=50000000
projectDir="/home/student/emd0003/Live_Project"

cd $projectDir/gen_data
./random_generator $dataSetSize data.dat

cd $projectDir/Distributed/
bash deploy.sh

cd $projectDir/Serial/
time ./serial_merge data.dat

cd $projectDir/Distributed/
time ./server_emd0003 data.dat
