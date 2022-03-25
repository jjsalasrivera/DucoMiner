#include "DucoMiner.h"
#include <iostream>
#include "Logger.h"
#include <fstream>
#include "PickAndShowel.h"

using namespace std;

bool DucoMiner::Init()
{
	bool res = false;
	Logger::White( "Initializing Duco Miner..." );

	try
	{
		// Read Configuration
		ifstream i("Settings.json");
		
		if( i.is_open() )
		{
			i >> _configuration;
			i.close();
			
			Logger::Yellow("Configuration: ");
			Logger::Yellow(_configuration.dump(4).c_str());
			
			// Check number of threads
			const auto processor_count = std::thread::hardware_concurrency();
			if( processor_count != _configuration["Threads"])
			{
				char message[200];
				sprintf( message, "Number of threads configured is %d, we recomend to use %d", _configuration["Threads"].get<int>(), processor_count);
				Logger::Red(message);
			}
			res = true;
		}
		else
			res = false;
	}
	catch( const exception& ex )
	{
		char message[200];
		sprintf( message, "Exception loading configuration: %s", ex.what() );
		Logger::Red( message);
		res = false;
	}
	catch( ... )
	{
		Logger::Red("There was an error configuring the miner");
	}
	
	return res;
}

void DucoMiner::Start()
{
	PickAndShowel::run = true;
	
	for(int i = 0; i < _configuration["Threads"].get<int>(); ++i)
	{
		_threads.push_back( thread(PickAndShowel(), i, _configuration) );
	}
	
	for( auto &th : _threads )
	{
		th.join();
	}
	
}

void DucoMiner::Stop()
{
	// Stop miners
	PickAndShowel::run = false;
}

