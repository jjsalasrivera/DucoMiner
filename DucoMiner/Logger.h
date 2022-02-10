#pragma once

class Logger
{
public:
	static void White ( const char* message );
	static void Yellow( const char* message );
	static void Red( const char* message );
	static void Green( const char* message );

private:
	Logger() {}
};
