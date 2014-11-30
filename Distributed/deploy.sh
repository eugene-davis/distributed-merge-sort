#!/bin/bash
# Script to rapidly deploy distributed merge sort clients and then run server

clientList="ClientList.conf"

while read host 
do
	echo "Deploying client to $host"
	ssh emd0003@$host ~/Live_Project/Distributed/client_emd0003
done < $clientList
