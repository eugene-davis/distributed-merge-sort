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

// Prototypes for merge sort functions
void mergeSplit(int destArray[], int originArray[], int begin, int end);
void merge(int destArray[], int originArray[], int begin, int middle, int end);
void copy(int destArray[], int originArray[], int begin, int end);

// Random, high port to reduce likelihood of colliding
#define PORT 68015

//#define debug

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

	cout << "Client on " << localHostname << " has started." << endl;

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
	int* tempArray = new int[totalNumbers];

	cout << "Beginning merge sort" << endl;
	mergeSplit(tempArray, data, 0, totalNumbers);
	cout << "Completed merge sort on partition. Sending data back." << endl;
	


	// Send sorted list back to server
	currentData = 0;
	for (int i = 0; i < totalNumbers; i++)
	{
		currentData = htonl(data[i]);
		sendStatus = send(socketConnection, (char *) &currentData, sizeof(int), flags);

		if (sendStatus < 0)
		{
			perror("Error sending data (client)");
			return false;
		}
		usleep(1); // Short delay to avoid causing overload of server
	}

	return 0;
}

// The basic portion of the merge sort, recursive function which will split
// until it hits an array of size one, then call merge as it goes back up
void mergeSplit(int destArray[], int originArray[], int begin, int end)
{
	// Check if split down to 1, in which case "sorted"
	if (end - begin <= 1)
	{
		return;
	}

	// Split recursion
	int middle = (end + begin) / 2;
	mergeSplit(destArray, originArray, begin, middle);
	mergeSplit(destArray, originArray, middle, end);

	// Merge
	merge(destArray, originArray, begin, middle, end);
	
	// Copy results back into origin array
	copy(destArray, originArray, begin, end);
}

void merge(int destArray[], int originArray[], int begin, int middle, int end)
{
	int beginTemp = begin;
	int middleTemp = middle;

	// Go while there are elements left to go over
	for (int i = begin; i < end; i++)
	{
		// If left array's current element is greater than or equal to rights current element, copy into appropriate location
		if (beginTemp < middle && (middleTemp >= end || originArray[beginTemp] <= originArray[middleTemp]))
		{
			destArray[i] = originArray[beginTemp];
			beginTemp++;
		}
		// If it wasn't larger, copy in the right one
		else
		{
			destArray[i] = originArray[middleTemp];
			middleTemp++;
		}
	}
}

// Moves the destArray back into the originArray now that it is sorted
void copy(int destArray[], int originArray[], int begin, int end)
{
	for (int i = begin; i < end; i++)
	{
		originArray[i] = destArray[i];
	}
}
