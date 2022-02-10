#pragma once

class AbstractMiner
{
public:
	virtual void Start() = 0;
	virtual bool Init() = 0;
	virtual void Stop() = 0;
};
