#include <iostream>
#include <fstream>

using namespace std;

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

	// Get size of data (from first line of file)
	dataFile >> dataSize;

	cout << dataSize << endl;
}
