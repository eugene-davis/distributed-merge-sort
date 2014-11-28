#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <fstream>
#include <string.h>

using namespace std;

// Random, high port to reduce likelihood of colliding
#define PORT 6862

#define debug

int main()
{
	char localHostname[100];
	struct sockaddr_in socket_addr, serverAddress;
	int socket1;
	int bindStatus;
	int listenStatus;
	int socketConnection;
	int rcvStatus, sendStatus;
	int totalNumbers = 0; // Number of integers to be recieved to sort
	socklen_t serverAddrLen;
	int *data;
	int flags = 0;
	
	// Get hostname
	gethostname(localHostname, sizeof(localHostname));

	// Check that hostname was successfully returned
	if (localHostname == NULL)
	{
		cout << "Local hostname appears to be misconfigured or too long" << endl;
		return 1;
	}

	// Create socket to listen on
	socket1 = socket(AF_INET, SOCK_STREAM, 0);

	if (socket1 <0)
	{
		perror("Error opening socket");
		return 1;
	}

	// Bind socket
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_port = htons(PORT);
	socket_addr.sin_addr.s_addr = INADDR_ANY;

	bindStatus = bind(socket1, (struct sockaddr *) &socket_addr, sizeof(struct sockaddr_in));

	if (bindStatus < 0)
	{
		perror("Error binding on socket");
		return 1;
	}	

	// Setup listening
	listenStatus = listen(socket1, 5);

	if (listenStatus < 0)
	{
		perror("Error on listen for socket");
		return 1;
	}

	// wait until connection has been made
	socketConnection = accept(socket1, (struct sockaddr *)&serverAddress, &serverAddrLen);
	if (socketConnection < 0)
	{
		perror("Error on accept for socket");
		return 1;
	}		

	// Recieve the total number of ints which will be sent here, recieve into an int (thus cast to char)
	rcvStatus = recv(socketConnection, (char *) &totalNumbers, sizeof(int), 0);
	if (rcvStatus < 0)
	{
		perror("Error recieving data");
		return -1;
	}

	totalNumbers = ntohl(totalNumbers);

	data = new int[totalNumbers];

	int currentData = 0;
	for (int i = 0; i < totalNumbers; i++)
	{
		rcvStatus = recv(socketConnection, (char *) &currentData, sizeof(int), 0);
		data[i] = ntohl(currentData);
	}


	#ifdef debug
	// debug by writing to file what is being recieved, so it can be compared to client
	ofstream testFile;
	testFile.open("testClientList", ofstream::out | ofstream::trunc);
	for (int i = 0; i < totalNumbers; i++)
	{
		testFile << data[i] << endl;
	}
	#endif

	// Perform merge sort


	// Send sorted list back to server
	currentData = 0;
	for (int i = 0; i < totalNumbers; i++)
	{
		currentData = htonl(data[i]);
		sendStatus = send(socketConnection, (char *) &currentData, sizeof(int), flags);

		if (sendStatus < 0)
		{
			perror("Error sending data");
			return false;
		}
	}

	return 0;
}
