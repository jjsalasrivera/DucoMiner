#pragma once

#include "AbstractMiner.h"
#include "json.h"
#include <thread>
#include <vector>

#define LOWDIFF 25000000
#define MEDIUMDIFF 250000000
#define HIGHDIFF 1000000000

using namespace std;
using namespace nlohmann;

class DucoMiner : AbstractMiner
{
public:
	bool Init();
	void Start();
	void Stop();
	
private:
	json _configuration;
	vector<thread> _threads;
	
	void InitializeNumbers( string Difficult );
};

