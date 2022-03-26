#include "PickAndShowel.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include "Logger.h"
#include <thread>
#include <openssl/sha.h>
#include <ctime>
#include <string>
#include "fmt/format.h"

using namespace std;

void PickAndShowel::operator()(int threadId, json configuration )
{
	while( run )
	{
		// Connect to IP
		_checkConnection(configuration["IP"].get<string>().c_str(), configuration["Port"].get<int>(), threadId );
		
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
		bool job_res = _askJob(jobTokens, configuration["UserName"].get<string>().c_str(), configuration["Start_Diff"].get<string>().c_str());
		
		if( job_res && jobTokens.expectedHash != NULL && jobTokens.lastHash != NULL)
		{
			// calculate hash
			auto start = chrono::high_resolution_clock::now();
			
			int res = _searchResult( jobTokens );
			
			auto stop = chrono::high_resolution_clock::now();
			float endMicros = (float) chrono::duration_cast<chrono::microseconds>( stop - start ).count();
			
			float hashRate = ((float)res / (endMicros / 1000000.0 ));
			// send result
			_sendResult(res, hashRate, configuration["MinerIdentifier"].get<string>().c_str(), threadId, jobTokens.diff, endMicros/1000000.0 );
			
			delete[] jobTokens.expectedHash;
			delete[] jobTokens.lastHash;
		}
		
		int intensity = configuration["Intensity"].get<int>();
		int milisToSleep( 10 );
		
		if( intensity >= 90 && intensity < 95)
			milisToSleep = 50;
		else if(  intensity >= 75 && intensity < 90 )
			milisToSleep = 200;
		else if(  intensity >= 50 && intensity < 75 )
			milisToSleep = 400;
		else if(  intensity >= 0 && intensity < 50 )
			milisToSleep = 600;
		
		this_thread::sleep_for(chrono::milliseconds(milisToSleep));
	}
}

void PickAndShowel::_checkConnection( const char* IP, int Port, int threadId )
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
		sprintf( message, "Core(%d) - Connected to: %s:%d", threadId, IP, Port );
		Logger::White(message);
		Logger::White(buffer);
		_isConnected = true;
	}
}

void PickAndShowel::_getMOTD()
{
	char buffer[512] = {0};
	
	ssize_t l = _sendAndReceive("MOTD\n", buffer, 512);
	
	if( l > 0)
	{
		Logger::White("MOTD: ");
		Logger::White(buffer);
	}
}

bool PickAndShowel::_askJob( JobTokens& Tokens, const char* userName, const char* diff )
{
	bool res = true;
	
	char jobRequest[128];
	char buffer[256];
	
	sprintf(jobRequest, "JOB,%s,%s\n", userName, diff);
	
	ssize_t l = _sendAndReceive( jobRequest, buffer, 256 );
	int i = 0;

	if( l > 0 )
	{
		// Tokenize
		char* tokens =  strtok( buffer, "," );
		
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
	}
	
	if( i != 3 || l <= 0)
		res = false;
	
	return res;
}

ssize_t PickAndShowel::_sendAndReceive( const char* message, char* response, int length )
{
	ssize_t l = -1;
	try
	{
		send( _socket, message, strlen(message), 0 );
		l = recv( _socket, response, length, NULL);
	}
	catch( exception& ex)
	{
		Logger::Red( ex.what() );
		_isConnected = false;
	}
	
	return l;
}

inline int PickAndShowel::_searchResult( JobTokens& job ) const throw()
{
	int res( 0 );
	unsigned char hash[SHA_DIGEST_LENGTH];
	unsigned char expected_hash_byte[SHA_DIGEST_LENGTH];
	
	size_t sizeOfHash = strlen( job.lastHash );
	
	for(int j = 0; j < SHA_DIGEST_LENGTH; ++j)
	{
		unsigned char e = ( _fromASCII( job.expectedHash[ j * 2 ] ) * 16 ) + _fromASCII( job.expectedHash[( j * 2 ) + 1] );
		expected_hash_byte[j] = e;
	}
	
	SHA_CTX ctx;
	SHA1_Init(&ctx);
	SHA1_Update(&ctx, job.lastHash, sizeOfHash);
	SHA_CTX ctx_copy;

	unsigned long endNanos(0);
	for( int i = 0; i <= job.diff * 100; ++i)
	{
		ctx_copy = ctx;

		//sprintf(charNumber, "%d", i);
		auto start = chrono::high_resolution_clock::now();
		auto f = fmt::format_int( i );
		auto stop = chrono::high_resolution_clock::now();
		endNanos += chrono::duration_cast<chrono::nanoseconds>( stop - start ).count();
		
		SHA1_Update(&ctx_copy, (const char*)f.data(), f.size() );
		SHA1_Final(hash, &ctx_copy);
		
		if( _equals( hash, expected_hash_byte, SHA_DIGEST_LENGTH))
		{
			char log[64];
			sprintf(log, "Conversion in: %ld nanos\n", endNanos/i);
			Logger::Yellow(log);

			res = i;
			break;
		}
	}
	
	return res;
}

void PickAndShowel::_sendResult( int result, float hashRate, const char* identifier, int threadId, int difficult, float seconds )
{
	char message[128];
	sprintf( message, "%d,%f,DucoMiner V0.1,%s,,%d\n", result, hashRate, identifier, MINER_ID );
	
	char response[256];
	ssize_t response_length = _sendAndReceive( message, response, 256 );
	
	const char* dateTime = _getTime();
	
	char log[256];
	
	if( response_length > 0 )
	{
		if( strstr( response, "GOOD\n" ) != NULL )
		{
			sprintf(log,"Core(%d) - %s - Accepted with difficult %d. Hash rate: %f H/s. Time: %f s", threadId, dateTime, difficult, hashRate, seconds );
			Logger::Green(log);
		}
		else
		{
			sprintf(log,"Core(%d) - %s - %s - difficult: %d - hashRate: %f H/s, Time: %f s", threadId, dateTime, response, difficult, hashRate, seconds );
			Logger::Yellow(log);
		}
	}
	
	delete [] dateTime;
}

const char* PickAndShowel::_getTime() const
{
	time_t now;
	struct tm* timeinfo;
	
	time( &now );
	timeinfo = localtime( &now );
	
	char* strTime = new char[20];
	sprintf(strTime, "%d-%02d-%02d %02d:%02d:%02d", timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	
	return strTime;
}

inline bool PickAndShowel::_equals( const unsigned char* hash, const unsigned char* expected, int length ) const throw()
{
	const unsigned char* h = hash;
	const unsigned char* e = expected;
	
	while( length-- )
	{
		if( *h++ != *e++ )
			return false;
	}
	
	return true;
}

inline unsigned char PickAndShowel::_fromASCII( const char c ) const
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
