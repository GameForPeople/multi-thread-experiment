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

	                 int  size    = 0;
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

		const int oldSize = size;
		std::memcpy( tempCharArr, charArr, oldSize );

		for ( int i = 0; i < oldSize; ++i )
		{
			if ( tempCharArr[ i ] == NULL_CHAR )
			{
				goto START_;
				//return std::string( tempCharArr, i );
			}
		}

		if ( oldSize != size )
			goto START_;

		return std::string( tempCharArr, oldSize );
	}

	void Set( const std::string& newString )
	{
		auto tempSize = newString.size() > MAX_SIZE
			? MAX_SIZE
			: newString.size();

		charArr[ tempSize ] = NULL_CHAR;
		std::memcpy( charArr, newString.c_str(), tempSize );
		size = tempSize;

		// 감소할 때, dddd -> ccc
		//	dddd 4 --> 적법
		//	ddd0 4 --> 부적법 ( 루프를 다시 돌아주지 않을 까? 아닌가벼 )
		//	ccc0 4 --> 부적법 ( 루프를 다시 돌아주지 않을 까? 아닌가벼 )
		//	ccc0 3 --> 적법

		// 증가할 때, ccc -> dddd
		//	ccc 3 --> 적법
		//	cccX0 3 --> 적법
		//	DDDD0 3 --> 부적법 ( 문제 상황 )
		//	DDDD0 4 --> 적법

		// 동일할 때, aaa -> bbb
		// aaa0 3 --> 적법
		// aaa0 3 --> 적법
		// bbb0 3 --> 적법
		// bbb0 3 --> 적법
	}

public:
	friend std::ostream& operator<<( std::ostream& os, const MultiThreadSafeString& dt );
};

std::ostream& operator<<( std::ostream& os, MultiThreadSafeString& dt)
{
    os << dt.Get();
    return os;
}

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

class StringWithSpinLock
{
	std::string      m_string;
	std::atomic_bool m_spinLock;

public:
	StringWithSpinLock()
		: m_string()
		, m_spinLock()
	{
	}

public:
	std::string Get()
	{
		bool expected = false;
		while ( !m_spinLock.compare_exchange_strong( expected, true ) )
		{
		}
		
		const auto retString = m_string;
		m_spinLock = false;

		return retString;
	}

	void Set( const std::string& newString )
	{
		bool expected = false;
		while ( !m_spinLock.compare_exchange_strong( expected, true ) )
		{
		}

		m_string = newString;

		m_spinLock = false;
	}
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
					for ( int i = 0 ; i < 5; ++i )
					{
						auto string = testString.Get();
						{
							std::unique_lock< std::shared_mutex > localLock( lock );
							std::cout << string.size() << " : " << string << std::endl;
						}
						std::this_thread::sleep_for( 0.3s );
					}
				} );
		}

		for ( int i = 0; i < 1; ++i )
		{
			writeThreadCont.emplace_back(
				[ &testString ]()
				{
					for ( int i = 0 ; i < 100000000; ++i )
					{
						//const int randomSize = rand() % 10 + 1;
						//std::string randomString ( randomSize, 'b' );
						
						std::string randomString;
						switch ( rand() % 6 )
						{
							case 0:
								randomString = "A";
								break;
							case 1:
								randomString = "BB";
								break;
							case 2:
								randomString = "CCC";
								break;
							case 3:
								randomString = "DDDD";
								break;
							case 4:
								randomString = "EEEEE";
								break;
							case 5:
								randomString = "FFFFFF";
								break;
							default:
							break;
						}

						testString.Set( randomString );
					}
				} );
		}

		for( auto& th : readThreadCont )  { th.join(); }
		for( auto& th : writeThreadCont ) { th.join(); }

		auto endTime = std::chrono::high_resolution_clock::now() - startTime;
		std::cout << "성능은? " << duration_cast<std::chrono::milliseconds>(endTime).count() << " msecs\n";
	};

	//{
	//	LockString string;
	//	testFunc( string );
	//}
	//{
	//	StringWithSpinLock string;
	//	testFunc( string );
	//}
	{
		MultiThreadSafeString string;
		testFunc( string );
	}

	std::system("pAuSe");
}