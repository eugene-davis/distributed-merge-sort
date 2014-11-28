#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// Prototypes
bool distribute(int data[], int dataSize);

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


	return true;
}
