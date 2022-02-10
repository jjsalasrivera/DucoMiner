#include "Logger.h"
#include <iostream>

using namespace std;

/*
 			foreground background
 black        30         40
 red          31         41
 green        32         42
 yellow       33         43
 blue         34         44
 magenta      35         45
 cyan         36         46
 white        37         47
 
 reset             0  (everything back to normal)
 bold/bright       1  (often a brighter shade of the same colour)
 underline         4
 inverse           7  (swap foreground and background colours)
 bold/bright off  21
 underline off    24
 inverse off      27
 
 cout << "\033[1;31mbold red text\033[0m\n";

*/

void Logger::White( const char* message )
{
	cout << "\033[37m" << message << "\033[0m\n";
}

void Logger::Yellow( const char* message )
{
	cout << "\033[33m" << message << "\033[0m\n";
}

void Logger::Red( const char* message )
{
	cout << "\033[31m" << message << "\033[0m\n";
}

void Logger::Green( const char* message )
{
	cout << "\33[32m" << message << "\033[0m\n";
}
