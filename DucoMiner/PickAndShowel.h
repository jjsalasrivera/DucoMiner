#pragma once

#include "json.h"

#define MINER_ID 1110 	// A random number to identify miners

using namespace nlohmann;

typedef struct
{
	char* lastHash;
	char* expectedHash;
	int diff;
} JobTokens;

class PickAndShowel
{
public:
	inline static bool run = false;
	inline static bool motdSended = false;
	
	void operator() (int threadId, json configuration);
	
private:
	int _socket = 0;
	bool _isConnected = false;
	
	void _checkConnection( const char* IP, int Port, int threadId );
	void _getMOTD();
	bool _askJob( JobTokens& tokens, const char* userName, const char* diff );
	void _sendAndReceive( const char* message, char* response, int length );
	int _searchResult( JobTokens& job ) const throw();
	bool _equals( const unsigned char* hash, const unsigned char* expected, int length  ) const throw();
	unsigned char _fromASCII( const char c ) const;
	size_t _getNumberOfbytes( int n ) const;
	void _sendResult( int result, float hashRate, const char* identifier, int threadId, int difficult, float seconds );
	const char* _getTime() const;
};
