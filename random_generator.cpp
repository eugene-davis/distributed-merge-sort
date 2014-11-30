/** Eugene Davis
    CPE 435
    Project
    Random Number Generator
**/

#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <fstream>

using namespace std;

int main(int argc, char *argv[])
{
	int dataSize = 0;

	// Handle command line arguments
	if (argc != 3)
	{
		cout << "Incorrect usage. Correct invocation is: " << endl;
		cout << "random_generator num_data_points output_file_name" << endl;
		cout << "The number of data points must be between 50 000 000 and 140 000 000." << endl;
		return 0;
	}
	
	// Get argument for data size
	dataSize = atoi(argv[1]);
	// Check that it was a valid data size
	if (dataSize < 50000000 || dataSize > 140000000)
	{
		cout << "Requested number of data points not within range." << endl;
		return 0;
	}

	// Setup file operations
	ofstream dataFile;
	// Open file for writing, and truncate it so any previous content is removed
	dataFile.open(argv[2], ofstream::out | ofstream::trunc | ofstream::binary);

	if (!dataFile.is_open())
	{
		cout << "Failed to open data file for writing." << endl;
		return 0;
	}

	// Open descriptor file (holds size of array)
	ofstream descriptFile;
    string descriptFileName(argv[2]);
    descriptFileName = "descrip_" + descriptFileName;
	descriptFile.open(descriptFileName.c_str(), ofstream::out | ofstream::trunc);

	if (!descriptFile.is_open())
	{
		cout << "Failed to open descriptive file for writing." << endl;
		return 0;
	}

	// Write size of data set to descriptive file
	descriptFile << dataSize << endl;

	descriptFile.close();

	// Seed random number generator
	srand(time(NULL));
	
	// Generate random numbers based on data size
	for (int i = 0; i < dataSize; i++)
	{
		// Write out the number, each number is on a new line
		dataFile << rand() << endl;
	}	

	dataFile.close();
}
