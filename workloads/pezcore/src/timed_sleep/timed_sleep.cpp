#include <thread>
#include <chrono>
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char**argv)
{
	// First arg will be the module name with full path
	// Second arg needs to be a positive integer to denote how many seconds to wait
	// There should be no other arguments
	if (argc < 2)
	{
		cout << "Function syntax: timed_sleep N" << endl;
		cout << "where N is a number representing the number of seconds to sleep." << endl;
		return -1;
	}
	
	// The second argument should be interpetable as an unsigned integer
	std::string sSecs = argv[1];
	unsigned int uiSecs = stoul(sSecs, nullptr, 0);

	if (uiSecs < 1)
	{
		cout << "Parameter passed could not be interpreted as a positive integer." << endl;
		cout << "Try again passing a positive integer such as 5 or 10 or 42." << endl;
		return -1;
	}

	cout << "Timed_Sleep was passed a valid argument of " << uiSecs << endl;
	cout << "Timed_Sleep will now sleep." << endl;

	std::chrono::seconds sleepDuration(uiSecs);
	std::this_thread::sleep_for(sleepDuration);

	cout << "Timed_Sleep has awoken!" << endl;
	cout << "Timed_Sleep took " << uiSecs <<" seconds." << endl;
	cout << "Exiting." << endl;

	return 0;
}