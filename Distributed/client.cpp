// Eugene Davis
// CPE 435 Project
// Distributed Portion - Client
// To run, simply run the client. Since the server
// is push, it will listed for a connection to it from
// the server, and work with it once connected

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
#include <stdlib.h>

using namespace std;

// Prototypes for merge sort functions
void* mergeSplit(void* argsP);
void merge(int destArray[], int originArray[], int begin, int middle, int end);
void copy(int destArray[], int originArray[], int begin, int end);

// Random, high port to reduce likelihood of colliding
#define PORT 68015

//#define debug

// If defined, turns on extra messages
//#define chatty

// Arguments for threads in merge sort
struct arguments
{
	
	int *destArray;
	int *originArray;
	int begin;
	int end;
	int rLevel;
};

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
	string outputFileName = localHostname;
	outputFileName = outputFileName + "RecievedList";
	testFile.open(outputFileName.c_str(), ofstream::out | ofstream::trunc);
	for (int i = 0; i < totalNumbers; i++)
	{
		testFile << data[i] << endl;
	}
	#endif

	// Perform merge sort
	int* tempArray = new int[totalNumbers];

	arguments* args = new arguments;
	args->destArray = tempArray;
	args->originArray = data;
	args->begin = 0;
	args->end = totalNumbers;
	args->rLevel = 0;
	

	#ifdef chatty
	cout << "Beginning merge sort" << endl;
	#endif

	mergeSplit(args);

	#ifdef chatty
	cout << "Completed merge sort on partition. Sending data back." << endl;
	#endif
	


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
	}
	
	// debug by writing to file what the
	// sorted list from the client is
	#ifdef debug
	outputFileName = localHostname;
	outputFileName = outputFileName + "sortedList";
	testFile.open(outputFileName.c_str(), ofstream::out | ofstream::trunc);
	for (int i = 0; i < totalNumbers; i++)
	{
		testFile << data[i] << endl;
	}
	#endif

	#ifdef chatty
	cout << "Completed sending of data." << endl;
	#endif

	close(socketConnection);

	return 0;
}

// The basic portion of the merge sort, recursive function which will split
// until it hits an array of size one, then call merge as it goes back up
void* mergeSplit(void *argsP)
{
	arguments args = *((arguments*)argsP);

	// Set variables back up (parse from arguments struct)
	int *destArray;
	int *originArray;
	int begin, end, rLevel;

	destArray = args.destArray;	
	originArray = args.originArray;
	begin = args.begin;
	end = args.end;
	rLevel = args.rLevel;

	rLevel++;

	// Check if split down to 1, in which case "sorted"
	if (end - begin <= 1)
	{
		// Must be allowed to exit without going further, and return can't be used since it is void*
	}
	else
	{
		// Split recursion
		int middle = (end + begin) / 2;

		// Arguments for first child
		arguments* args1 = new arguments;
		args1->destArray = destArray;
		args1->originArray = originArray;
		args1->begin = begin;
		args1->end = middle;
		args1->rLevel = rLevel;

		// Arguments for second child
		arguments* args2 = new arguments;
		args2->destArray = destArray;
		args2->originArray = originArray;
		args2->begin = middle;
		args2->end = end;
		args2->rLevel = rLevel;
	
		// Split off new threads until the third level of recursion - this will leave 8 active threads, with the rest waiting (with join) for the
		// child threads to complete, allowing everything to run in parallel
		if (rLevel <= 3)
		{
			pthread_t mergeSplitThreads[2];

			// Create the threads
			pthread_create(&mergeSplitThreads[0], NULL, mergeSplit, args1);
			pthread_create(&mergeSplitThreads[1], NULL, mergeSplit, args2);

			// Now join on the children
			pthread_join(mergeSplitThreads[0], NULL);
			pthread_join(mergeSplitThreads[1], NULL);
		}
		// Once more than 8 threads are working at the same time (not counting those waiting at join), don't split off new threads
		else
		{
			mergeSplit(args1);
			mergeSplit(args2);
		}

		// Merge
		merge(destArray, originArray, begin, middle, end);
	
		// Copy results back into origin array
		copy(destArray, originArray, begin, end);
	}
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
