#!/bin/bash
# Script to rapidly deploy distributed merge sort clients and then run server

clientList="ClientList.conf"

while read host 
do
	ssh emd0003@$host ~/Project_Live/Distributed/client
done < $clientList
