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

using namespace std;

// Random, high port to reduce likelihood of colliding
#define PORT 6862

// Prototypes
bool distribute(int data[], int dataSize);
bool clientConnection();

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
		return 1;
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
	int numMachines = 20;
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

	if (!clientConnection())
	{
		return false;
	}

	return true;
}

// Client specific function
bool clientConnection()
{
	// Socket setup
	int socket1;
	struct sockaddr_in sock1Addr;
	int connectionStatus;
	char hostname[100] = "localhost"; // Temp, will be parameter soon
	int dataNumbers = htonl(8); // Temp, will be parameter soon
	char ip[100];
	struct hostent *hostDetails; // Details returned by gethostby name
	struct in_addr **addresses; // List of addresses contained in hostDetails
	int flags;
	int sendStatus;

	// Get hostname
	hostDetails = gethostbyname(hostname);

	// Check that hostname was successfully returned
	if (hostDetails == NULL)
	{
		cout << "Unable to retrieve IP address for hostname " << hostname << endl;
		return false;
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
		perror("Error opening socket");
		return false;
	}

	sock1Addr.sin_family = AF_INET;
	sock1Addr.sin_port = htons((u_short) PORT);
	sock1Addr.sin_addr.s_addr = inet_addr(ip);


	connectionStatus = connect(socket1, (struct sockaddr *) &sock1Addr, sizeof(struct sockaddr_in));

	if (connectionStatus < 0)
	{
		perror("Error connecting to client");
		return false;
	}

	// Send size of data
	sendStatus = send(socket1, (char *) dataNumbers, sizeof(int), flags);

	return true;
}
