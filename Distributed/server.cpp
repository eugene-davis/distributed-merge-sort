#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>

using namespace std;

// Random, high port to reduce likelihood of colliding
#define PORT 68015

// Number of clients
#define NUM_MACHINES 20

//#define debug

struct arguments
{
	
	int *data;
	string hostName;
	int start;
	int end;
};

// Prototypes
bool distribute(int data[], int dataSize);
//bool clientConnection(int data[], int start, int end);
void* clientConnection(void *argsP);

pthread_mutex_t mutex;

int main(int argc, char *argv[])
{
	int dataSize = 0;

	// Parse commandline argument
	if (argc != 2)
	{
		cout << "Incorrect usauge. Correct invocation is: " << endl;
		cout << "serial_merge input_file_name" << endl;
		return 0;
	}

	// Open the file
	ifstream dataFile;
	dataFile.open(argv[1]);
	
	// Check if file opened correctly
	if (!dataFile.is_open())
	{
		cout << "Failed to open data file for reading." << endl;
		return 0;
	}

	// Open descriptive file
	ifstream descriptFile;
	string descriptFileName(argv[1]);
	descriptFileName = "descrip_" + descriptFileName;
	descriptFile.open(descriptFileName.c_str());

	if (!descriptFile.is_open())
	{
		cout << "Failed to open descriptive file for reading." << endl;
		return 0;
	}

	// Get size of data (from first line of file)
	descriptFile  >> dataSize;

	// Check that the data size is in range
	if (dataSize < 50000000 || dataSize > 140000000)
	{
		cout << "The data size indicated by the file is out of range." << endl;
		return 0;
	}

	// Setup array to hold data
	int* data = new int[dataSize];

	// Start reading in data
	int currentData;
	for (currentData = 0; currentData < dataSize && !dataFile.eof(); currentData++)
	{
		dataFile >> data[currentData];
	}

	// Progress the dataFile by one more line to hit end of file (assuming valid file)
	dataFile.ignore(10000000000000, '\n');
	dataFile.ignore(10000000000000, '\n');

	// Verify that the data grabbed is the same amount as dataSize indicated
	// and that the end of file has been reached - i.e. make sure the data
	// file given was valid
	if (currentData != dataSize || !dataFile.eof())
	{
		cout << "The data size did not match the size specified by the descriptor, suggesting that the input file was invalid or corrupted." << endl;
		return 0;
	}

	// Distribute data to clients
	if (!distribute(data, dataSize))
	{
		cout << "Problems encountered while distributing data to clients." << endl;
		return 1;
	}
	
	// Final merging. Use insertion sort, since 20 isn't a power of two (thus no clean merge sort option), and insertion sort is relatively effective on partially sorted lists
	int x = 0;
	for (int i = 0; i < dataSize; i++)
	{
		x = data[i];
		int j = i;
		while (j > 0 && data[j - 1] > x)
		{
			data[j] = data[j - 1];
			j = j - 1;
		}
		data[j] = x;
	}

	// Output array
	ofstream outputFile;
	string outputFileName(argv[1]);
	outputFileName = "sorted_" + outputFileName;
	outputFile.open(outputFileName.c_str(), ofstream::out | ofstream::trunc);
	if (!outputFile.is_open())
	{
		cout << "Failed to open " << outputFileName  << " file for writing." << endl;
		return 0;
	}
	
	// Write to file
	for (int i = 0; i < dataSize; i++)
    {
    	outputFile << data[i] << endl;
    }

}

// Distributes packages to clients
bool distribute(int data[], int dataSize)
{
	int numMachines = NUM_MACHINES;
	arguments allArgs[20];
	string hostNames[20];

	// Read in client list from configuration file
	ifstream clientList;
	clientList.open("ClientList.conf");
	
	if (!clientList.is_open())
	{
		cout << "Failed to open client list. Now exiting." << endl;
		return false;
	}
	
	// Read the twenty machine names, error out if too few (ignores any extra)
	int i = 0;
	for ( ; i < numMachines && !clientList.eof(); i++)
	{
		clientList >> hostNames[i];
	}

	// Check if too few in the list
	if (i < 19)
	{
		cout << "Machine list had few than twenty hostnames. Now exiting." << endl;
		return false;
	}

	pthread_t clients[NUM_MACHINES];

	// Calculate partitions for data and kick off transmission
	int partitionSize = dataSize / numMachines;
	arguments* args;
	// Handles all but the final (potentially differently sized) connection
	for (int i = 0; i < numMachines - 1; i++)
	{
		args = &allArgs[i];
		args->data = data;
		args->hostName = hostNames[i];
		args->start = (i * partitionSize);
		args->end = (i + 1) * partitionSize;

		pthread_mutex_lock(&mutex);
		cout << "Launching thread for client " << hostNames[i] << endl;
		pthread_mutex_unlock(&mutex);

		pthread_create(&clients[i], NULL, clientConnection, args);
	}

	// Final connection, may not be as big as the others
	args = &allArgs[19];
	args->data = data;
	args->hostName = hostNames[19];
	args->start = (19 * partitionSize);
	args->end = dataSize;

	pthread_mutex_lock(&mutex);
	cout << "Launching thread for client " << hostNames[19] << endl;
	pthread_mutex_unlock(&mutex);

	pthread_create(&clients[19], NULL, clientConnection, args);
	

	// Wait for clients to complete
	for (int i = 0; i < NUM_MACHINES; i++)
	{
		pthread_join(clients[i], NULL);
	}

	return true;
}

// Client specific function
//bool clientConnection(int data[], int start, int end)
void* clientConnection(void *argsP)
{
	arguments args = *((arguments*)argsP);

	// Socket setup
	int socket1;
	struct sockaddr_in sock1Addr;
	int connectionStatus;

	string errorMsg;

	int dataNumbers = args.end - args.start;
	int start = args.start;
	int end = args.end;
	char ip[100];
	struct hostent *hostDetails; // Details returned by gethostby name
	struct in_addr **addresses; // List of addresses contained in hostDetails
	int flags = 0;
	int sendStatus, rcvStatus;
	int *data;
	data = args.data;

	// Get hostname
	hostDetails = gethostbyname(args.hostName.c_str());

	// Check that hostname was successfully returned
	if (hostDetails == NULL)
	{
		pthread_mutex_lock(&mutex);
		cout << "Unable to retrieve IP address for hostname " << args.hostName << endl;
		pthread_mutex_unlock(&mutex);
	}

	addresses = (struct in_addr **) hostDetails->h_addr_list;
	
	// Get first of the valid addresses
	for (int i = 0; addresses[i] != NULL; i++)
	{
		strcpy(ip, inet_ntoa(*addresses[i]));
	}

	socket1 = socket(AF_INET, SOCK_STREAM, 0);

	if (socket1 < 0)
	{
		pthread_mutex_lock(&mutex);
		perror("Error opening socket");
		pthread_mutex_unlock(&mutex);
	}

	sock1Addr.sin_family = AF_INET;
	sock1Addr.sin_port = htons((u_short) PORT);
	sock1Addr.sin_addr.s_addr = inet_addr(ip);


	connectionStatus = connect(socket1, (struct sockaddr *) &sock1Addr, sizeof(struct sockaddr_in));

	if (connectionStatus < 0)
	{
		errorMsg = "Error connecting to client " + args.hostName;
		pthread_mutex_lock(&mutex);
		perror(errorMsg.c_str());
		pthread_mutex_unlock(&mutex);
	}

	dataNumbers = htonl(dataNumbers);

	pthread_mutex_lock(&mutex);
	cout << "Beginning data transmission to " << args.hostName << endl;
	pthread_mutex_unlock(&mutex);

	// Send size of data
	sendStatus = send(socket1, (char *) &dataNumbers, sizeof(int), flags);

	if (sendStatus < 0)
	{
		errorMsg = "Error sending data size to " + args.hostName;
		pthread_mutex_lock(&mutex);
		perror(errorMsg.c_str());
		pthread_mutex_unlock(&mutex);
	}

	#ifdef debug
	// debug by writing to file what is being sent, so it can be compared to client
	ofstream testFile;
	testFile.open("testServerSendList", ofstream::out | ofstream::trunc);
	for (int i = start; i < end; i++)
	{
		testFile << data[i] << endl;
	}
	testFile.close();
	#endif


	int size = (end - start) * sizeof(int);

	// Send data
	int currentData = 0;
	for (int i = start; i < end; i++)
	{
		currentData = htonl(data[i]);
		sendStatus = send(socket1, (char *) &currentData, sizeof(int), flags);

		if (sendStatus < 0)
		{
			errorMsg = "Error sending data to " + args.hostName;
			perror(errorMsg.c_str());
		}
		//usleep(1); // Short delay to avoid causing overload of client
	}

	pthread_mutex_lock(&mutex);
	cout << "Completed sending data to " + args.hostName << endl << "Waiting for data from client." << endl << endl;
	pthread_mutex_unlock(&mutex);

	// Recieve sorted data
	currentData = 0;
	for (int i = start; i < end; i++)
	{
		rcvStatus = recv(socket1, (char *) &currentData, sizeof(int), 0);
		data[i] = ntohl(currentData);
	}

	#ifdef debug
	// debug by writing to file what is being recieved, so it can be compared to client
	testFile.open("testServerRecieveList", ofstream::out | ofstream::trunc);
	for (int i = start; i < end; i++)
	{
		testFile << data[i] << endl;
	}
	testFile.close();
	#endif

	//close(socket1);

}
