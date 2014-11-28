#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// Prototypes for merge sort functions
void mergeSplit(int destArray[], int originArray[], int begin, int end);
void merge(int destArray[], int originArray[], int begin, int middle, int end);
void copy(int destArray[], int originArray[], int begin, int end);

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


	// Merge sort
	
	// Create temp array that will be used during sorting process
	// actual results will be returned back into A
	int* tempArray = new int[dataSize];

	mergeSplit(tempArray, data, 0, dataSize);

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
