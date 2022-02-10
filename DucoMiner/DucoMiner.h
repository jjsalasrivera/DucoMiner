#pragma once

#include "AbstractMiner.h"
#include "json.h"
#include <thread>
#include <vector>

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
};

