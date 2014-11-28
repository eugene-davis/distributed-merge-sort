#/bin/bash
# Script to take down clients if problems have occurred

clientList="ClientList.conf"

while read host 
do
	echo "Killing clients on $host"
        ssh emd0003@$host pkill client_emd0003 &
done < $clientList

