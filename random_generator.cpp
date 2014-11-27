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

int main(void)
{
	int dataSize = 0;
	while (dataSize < 50000000 || dataSize > 140000000)
	{
		cout << "Enter a data size between 50 000 000 and 140 000 000." << endl;
	
		cin >> dataSize;
		if (dataSize < 50000000 && dataSize > 140000000)
		{
			cout << "Number not in range, try again." << endl;
		}
	}

	// Setup file operations
	ofstream dataFile;
	dataFile.open("rand_nums.dat");

	// Seed random number generator
	srand(time(NULL));
	
	// Generate random numbers based on data size
	for (int i = 0; i < dataSize; i++)
	{
		// Write out the number, each number is on a new line
		dataFile << rand() << endl;
	}	
}
