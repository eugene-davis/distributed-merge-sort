#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <fstream>

using namespace std;

// Random, high port to reduce likelihood of colliding
#define PORT 6862

int main()
{
	char localHostname[100];
	struct sockaddr_in socket_addr, serverAddress;
	int socket1;
	int bindStatus;
	int listenStatus;
	int socketConnection;
	int rcvStatus;
	int totalNumbers = 0; // Number of integers to be recieved to sort
	socklen_t serverAddrLen;
	
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

	totalNumbers = ntohl(totalNumbers);
	
	cout << "Recieved number as " << totalNumbers << endl;

	close(socketConnection);

	return 0;
}
