#pragma once

/*
	Copyright 2020, Won Seong-Yeon. All Rights Reserved.
		KoreaGameMaker@gmail.com
		github.com/GameForPeople
*/

#include <string>
#include <shared_mutex>

#include <thread>
#include <vector>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>

static std::shared_mutex lock;
/*
	멍청하지만 안전한 스트링.... 문자열?
		- 죽지는 않지만 이상한 값들을 내뱉을 수 있음.
*/
//template < int MAX_SIZE >
class MultiThreadSafeString
{
	static constexpr int  MAX_SIZE  = 10;
	static constexpr char NULL_CHAR = '\0';

	                 int  size = 0;
	                 char charArr[ MAX_SIZE + 1 ];

public:
	MultiThreadSafeString()
	{
		charArr[ MAX_SIZE ] = NULL_CHAR;
		charArr[ 0 ]        = NULL_CHAR;
	}

	MultiThreadSafeString( const std::string& string )
	{
		Set ( string );
	}

public:
	std::string Get()
	{
		START_:
		char tempCharArr[ MAX_SIZE + 1 ];
		std::memcpy( tempCharArr, charArr, size );

		for ( int i = 0; i < size; ++i )
		{
			if ( tempCharArr[ i ] == NULL_CHAR )
			{
				goto START_;
				return std::string( tempCharArr, i );
			}
		}

		return std::string( tempCharArr, size );
	}

	void Set( const std::string& newString )
	{
		auto tempSize = newString.size() > MAX_SIZE
			? MAX_SIZE
			: newString.size();

		charArr[ tempSize ] = NULL_CHAR;
		std::memcpy( charArr, newString.c_str(), tempSize );
		size = tempSize;
	}

public:

};

template < int MAX_SIZE >
class T_MultiThreadSafeString
{
	static constexpr char NULL_CHAR = '\0';

	                 int  size = 0;
	                 char charArr[ MAX_SIZE + 1 ];

public:
	T_MultiThreadSafeString()
	{
		charArr[ MAX_SIZE ] = NULL_CHAR;
		charArr[ 0 ]        = NULL_CHAR;
	}

	T_MultiThreadSafeString( const std::string& string )
	{
		Set ( string );
	}

public:
	std::string Get()
	{
		START_:
		char tempCharArr[ MAX_SIZE + 1 ];
		std::memcpy( tempCharArr, charArr, size );

		for ( int i = 0; i < size; ++i )
		{
			if ( tempCharArr[ i ] == NULL_CHAR )
				goto START_;
		}

		return std::string( tempCharArr, size );
	}

	void Set( const std::string& newString )
	{
		auto tempSize = newString.size() > MAX_SIZE
			? MAX_SIZE
			: newString.size();

		charArr[ tempSize ] = NULL_CHAR;
		std::memcpy( charArr, newString.c_str(), tempSize );
		size = tempSize;
	}

public:

};


class LockPtrString
{
	std::string*      m_stringPtr;
	std::shared_mutex m_lock;

public:
	LockPtrString()
		: m_stringPtr( nullptr )
		, m_lock     (         )
	{
	}

public:
	std::string Get()
	{
		std::shared_lock< std::shared_mutex > lock ( m_lock );
		return *m_stringPtr;
	}

	void Set( const std::string& newString )
	{
		std::string* newStringPtr = new std::string( newString );
		std::string* oldStringPtr = nullptr;

		{
			std::unique_lock< std::shared_mutex > lock ( m_lock );
			oldStringPtr = m_stringPtr;
			m_stringPtr = newStringPtr;
		}

		delete oldStringPtr;
	}

public:

};

class LockString
{
	std::string       m_string;
	std::shared_mutex m_lock;

public:
	LockString()
		: m_string()
		, m_lock  ()
	{
	}

public:
	std::string Get()
	{
		std::shared_lock< std::shared_mutex > lock ( m_lock );
		return m_string;
	}

	void Set( const std::string& newString )
	{
		std::unique_lock< std::shared_mutex > lock ( m_lock );
		m_string = newString;
	}

public:

};

class String
{
	std::string m_string;

public:
	String()
		: m_string()
	{
	}

public:
	std::string Get()
	{
		return m_string;
	}

	void Set( const std::string& newString )
	{
		m_string = newString;
	}

public:
};

int main()
{
	using namespace std::chrono_literals;

	auto testFunc = []( auto& testString )
	{
		auto startTime = std::chrono::high_resolution_clock::now();

		std::vector< std::thread > readThreadCont;
		std::vector< std::thread > writeThreadCont;
		
		for ( int i = 0; i < 6; ++i )
		{
			readThreadCont.emplace_back(
				[ &testString ]()
				{	
					for ( int i = 0 ; i < 3; ++i )
					{
						auto string = testString.Get();
						{
							std::unique_lock< std::shared_mutex > localLock( lock );
							std::cout << string.size() << " : " << string << std::endl;
						}
						std::this_thread::sleep_for( 1s );
					}
				} );
		}

		for ( int i = 0; i < 3; ++i )
		{
			writeThreadCont.emplace_back(
				[ &testString ]()
				{
					for ( int i = 0 ; i < 100000000; ++i )
					{
						const int randomSize = rand() % 10 + 1;
						std::string randomString ( randomSize, 'b' );

						testString.Set( randomString );
					}
				} );
		}

		for( auto& th : readThreadCont )  { th.join(); }
		for( auto& th : writeThreadCont ) { th.join(); }

		auto endTime = std::chrono::high_resolution_clock::now() - startTime;
		std::cout << "성능은? " << duration_cast<std::chrono::milliseconds>(endTime).count() << " msecs\n";
	};

	{
		LockString string;
		testFunc( string );
	}
	{
		MultiThreadSafeString string;
		testFunc( string );
	}
	{
		T_MultiThreadSafeString< 10 > string;
		testFunc( string );
	}

	std::system("pAuSe");
}