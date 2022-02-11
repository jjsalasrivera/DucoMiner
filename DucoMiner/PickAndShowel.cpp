#include "PickAndShowel.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include "Logger.h"
#include <thread>
#include <openssl/sha.h>
#include <ctime>

using namespace std;

void PickAndShowel::operator()(int threadId, json configuration )
{
	while( run )
	{
		// Connect to IP
		_checkConnection(configuration["IP"].get<string>().c_str(), configuration["Port"].get<int>() );
		
		// Send MOTD (if neccesary)
		if( !motdSended )
		{
			motdSended = true;
			_getMOTD();
		}
		
		// ask for job
		JobTokens jobTokens;
		jobTokens.expectedHash = NULL;
		jobTokens.lastHash = NULL;
		_askJob(jobTokens, configuration["UserName"].get<string>().c_str(), configuration["Start_Diff"].get<string>().c_str());
		
		if( jobTokens.expectedHash != NULL && jobTokens.lastHash != NULL)
		{
			// calculate hash
			auto start = chrono::high_resolution_clock::now();
			
			int res = _searchResult( jobTokens );
			
			auto stop = chrono::high_resolution_clock::now();
			float endMilis = (float) chrono::duration_cast<chrono::milliseconds>( stop - start ).count();
			
			float hashRate = ((float)res / (endMilis / 1000.0 ));
			// send result
			_sendResult(res, hashRate, configuration["MinerIdentifier"].get<string>().c_str(), threadId, jobTokens.diff, endMilis/1000.0 );
			
			delete[] jobTokens.expectedHash;
			delete[] jobTokens.lastHash;
		}
		
		int intensity = configuration["Intensity"].get<int>();
		int milisToSleep( 10 );
		
		if( intensity >= 90 && intensity < 95 )
			milisToSleep = 100;
		else if(  intensity >= 75 && intensity < 90 )
			milisToSleep = 300;
		else if(  intensity >= 50 && intensity < 75 )
			milisToSleep = 600;
		else if(  intensity >= 0 && intensity < 50 )
			milisToSleep = 800;
		
		this_thread::sleep_for(chrono::milliseconds(milisToSleep));
	}
}

void PickAndShowel::_checkConnection( const char* IP, int Port )
{
	struct sockaddr_in serv_addr;
	
	if( _socket == 0 || !_isConnected )
	{
		while ((_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			Logger::Red("Socket creation error, retry in 3 seconds");
			this_thread::sleep_for(std::chrono::seconds(3));
		}
		
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(Port);
		
		if( inet_pton(AF_INET, IP, &serv_addr.sin_addr) <= 0 )
		{
			Logger::Red("\nInvalid address or Address not supported");
		}
		
		while( connect(_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 )
		{
			Logger::Red("Connection Failed");
			this_thread::sleep_for(std::chrono::seconds(3));
		}
		
		char buffer[8] = {0};
		recv( _socket, buffer, 8, NULL);
		
		char message[200];
		sprintf( message, "Connected to: %s:%d", IP, Port );
		Logger::White(message);
		Logger::White(buffer);
		_isConnected = true;
	}
}

void PickAndShowel::_getMOTD()
{
	char buffer[512] = {0};
	
	_sendAndReceive("MOTD\n", buffer, 512);
	
	Logger::White("MOTD: ");
	Logger::White(buffer);
}

bool PickAndShowel::_askJob( JobTokens& Tokens, const char* userName, const char* diff )
{
	bool res = true;
	
	char jobRequest[128];
	char buffer[128];
	
	sprintf(jobRequest, "JOB,%s,%s\n", userName, diff);
	
	_sendAndReceive( jobRequest, buffer, 128 );
	
	// Tokenize
	char* tokens =  strtok( buffer, "," );
	
	int i = 0;
	while( tokens != NULL && i < 3 )
	{
		if( i == 0 )
		{
			Tokens.lastHash = new char[strlen(tokens)+1]();
			strcpy(Tokens.lastHash, tokens);	//memcpy( Tokens.lastHash, tokens, strlen(tokens)+1);
			//Tokens.lastHash = tokens;
		}
		else if( i == 1 )
		{
			Tokens.expectedHash = new char[strlen(tokens)+1]();
			strcpy(Tokens.expectedHash, tokens);
		}
		else if( i == 2 )
			Tokens.diff = atoi( tokens );
		
		tokens = strtok( NULL, "," );
		++i;
	}
	
	if( i != 3)
		res = false;
	
	return res;
}

void PickAndShowel::_sendAndReceive( const char* message, char* response, int length )
{
	try
	{
		send( _socket, message, strlen(message), 0 );
		recv( _socket, response, length, NULL);
	}
	catch( exception& ex)
	{
		Logger::Red( ex.what() );
		_isConnected = false;
	}
}

int PickAndShowel::_searchResult( JobTokens& job )
{
	int res( 0 );
	unsigned char hash[SHA_DIGEST_LENGTH]; // == 20
	unsigned char expected_hash_byte[SHA_DIGEST_LENGTH];
	
	unsigned char charNumber[10];
	size_t sizeOfHash = strlen( job.lastHash );
	
	for(int j = 0; j < SHA_DIGEST_LENGTH; ++j)
	{
		unsigned char e = ( _fromASCII( job.expectedHash[ j * 2 ] ) * 16 ) + _fromASCII( job.expectedHash[( j * 2 ) + 1] );
		expected_hash_byte[j] = e;
	}
	
	char tmp[60];
	
	for( int i = 0; i <= job.diff * 100; ++i)
	{
		std::strcpy( tmp, job.lastHash );
		std::sprintf((char*)charNumber, "%d", i);
		std::strcat(tmp, (const char*)charNumber);
		
		SHA1((unsigned const char*)tmp, sizeOfHash + _getNumberOfbytes(i), hash);
		
		if( _equals( hash, expected_hash_byte, SHA_DIGEST_LENGTH ))
		{
			res = i;
			break;
		}
	}
	
	return res;
}

void PickAndShowel::_sendResult( int result, float hashRate, const char* identifier, int threadId, int difficult, float seconds )
{
	char message[128];
	sprintf( message, "%d,%f,DucoMiner V0.1,%s,,%d\n", result, hashRate, identifier, threadId );
	
	char response[128];
	_sendAndReceive( message, response, 128 );
	
	const char* dateTime = _getTime();
	
	char log[128];
	
	if( strcmp( response, "GOOD\n" ) == 0)
	{
		sprintf(log,"Core(%d) - %s - Accepted with difficult %d. Hash rate: %f H/s. Time: %f s", threadId, dateTime, difficult, hashRate, seconds );
		Logger::Green(log);
	}
	else
	{
		sprintf(log,"Core(%d) - %s - %s - difficult: %d - hashRate: %f H/s, Time: %f s", threadId, dateTime, response, difficult, hashRate, seconds );
		Logger::Yellow(log);
	}
	
	delete [] dateTime;
}

const char* PickAndShowel::_getTime()
{
	time_t now;
	struct tm* timeinfo;
	
	time( &now );
	timeinfo = localtime( &now );
	
	char* strTime = new char[20];
	sprintf(strTime, "%d-%02d-%02d %02d:%02d:%02d", timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	
	return strTime;
}

bool PickAndShowel::_equals( const unsigned char* hash, const unsigned char* expected, int length )
{
	for( int i = 0; i < length; ++i )
	{
		if(hash[i] != expected[i])
			return false;
	}
	
	return true;
}

unsigned char PickAndShowel::_fromASCII( const char c )
{
	unsigned char r( '0' );
	
	if( c >= '0' && c <= '9' )
		r = c - 48;
	else if( c >= 'A' && c <= 'X' )
		r = c - 55;
	else
		r = c - 87;
	
	return r;
}

size_t PickAndShowel::_getNumberOfbytes( int n )
{
	size_t r = 1;
	
	if( n < 10)
		r = 1;
	else if( n < 100 )
		r = 2;
	else if ( n < 1000 )
		r = 3;
	else if ( n < 10000 )
		r = 4;
	else if ( n < 100000 )
		r = 5;
	else if ( n < 1000000 )
		r = 6;
	else if ( n < 10000000 )
		r = 7;
	else if ( n < 100000000 )
		r = 8;
	else if ( n < 1000000000 )
		r = 9;
	
	return r;
}
