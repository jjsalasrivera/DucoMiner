#include <iostream>
#include "Logger.h"
#include "DucoMiner.h"

using namespace std;

int main(int argc, const char * argv[]) {
	//const auto processor_count = std::thread::hardware_concurrency();

	//std::cout << "Number of processor: " << processor_count;

	DucoMiner ducoMiner;
	
	if(ducoMiner.Init())
	{
		ducoMiner.Start();
	}
	
	return 0;
}
