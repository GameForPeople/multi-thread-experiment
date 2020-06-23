#include "stdafx.h"

#include <iostream>
#include <string>

#include <thread>
#include <atomic>

#define _ERROR 0

template <class TYPE> inline auto T_CAS(volatile TYPE* addr, TYPE& oldValue, TYPE newValue) noexcept -> bool
{
	return std::atomic_compare_exchange_strong(reinterpret_cast<volatile std::atomic<TYPE>*>(addr), &oldValue, newValue);
};

struct AccessMemory
{
	int				dataA;
	char			dataB;
	std::string		dataC;
	AccessMemory*	dataD;
};

int main()
{
	AccessMemory*				pAccessMemory{ nullptr };
	std::vector< std::thread > 	readThreadCont;
	std::thread					writeThreaed;

	pAccessMemory = new AccessMemory( { 1, 'A', "ABC", new AccessMemory{} } );

	//  Read Thread
	for( int i = 0; i < 8; ++i )
	{
		readThreadCont.emplace_back(
			static_cast< std::thread >(
			[&]() noexcept -> auto
			{
				while( 7 )
				{
					switch( rand() % 4 )
					{
					case 0:	{ if ( pAccessMemory->dataA != 1 && pAccessMemory->dataA != 2 )			{ throw _ERROR; } } break;
					case 1: { if ( pAccessMemory->dataB != 'A' && pAccessMemory->dataB != 'B' )		{ throw _ERROR; } } break;
					case 2: { if ( pAccessMemory->dataC != "ABC" && pAccessMemory->dataC != "CBA" ) { throw _ERROR; } } break;
					case 3: { if ( pAccessMemory->dataD == nullptr )								{ throw _ERROR; } } break;
					default: break;
					}
				}
			} ) );
	}

	// Write Thread
	writeThreaed = static_cast< std::thread >( 
		[&]() noexcept -> auto
		{ 
			int				localDataA{ 1	  };
			char			localDataB{ 'A'	  };
			std::string		localDataC{ "ABC" };
			AccessMemory*	localDataD = pAccessMemory->dataD;

			bool	retVal		  { true };
			int		printLoopCount{ 0	 };  
			while ( 7 )
			{
				localDataA = ( localDataA == 1		) ? 2 : 1;
				localDataB = ( localDataB == 'A'	) ? 'B' : 'A';	
				localDataC = ( localDataC == "ABC"	) ? "CBA" : "ABC";	

				AccessMemory*	oldAccessMemory		= pAccessMemory;
				AccessMemory*	copyOldAccessMemory = pAccessMemory;
				long			oldPointerAddr		= reinterpret_cast< long >( pAccessMemory );
				
				AccessMemory*	newAccessMemory = new AccessMemory( { localDataA, localDataB, localDataC, localDataD } );
				long			newPointerAddr	= reinterpret_cast< long >( newAccessMemory );

				retVal = T_CAS( &pAccessMemory, oldAccessMemory, newAccessMemory );
				//pAccessMemory = newAccessMemory;
				// memory Leak!

				if ( !retVal )															 { break; }
				if ( newPointerAddr != reinterpret_cast< long >( pAccessMemory       ) ) { break; }
				if ( newPointerAddr != reinterpret_cast< long >( newAccessMemory     ) ) { break; }
				if ( oldPointerAddr != reinterpret_cast< long >( oldAccessMemory     ) ) { break; }
				if ( oldPointerAddr != reinterpret_cast< long >( copyOldAccessMemory ) ) { break; }

				if ( printLoopCount >= 1000000 )
				{
					printLoopCount = 0;
					std::cout 
						<< " pAccessMemory :" << reinterpret_cast< long >( pAccessMemory )
						<< ",   newAccessMemory :" << reinterpret_cast< long >( newAccessMemory )
						<< ",   oldAccessMemory :" << reinterpret_cast< long >( oldAccessMemory )
						<< ",   newPointerAddr :" << newPointerAddr  
						<< ",   oldPointerAddr :" << oldPointerAddr
						<< "\n";
				}

				++printLoopCount;
			}

			std::cout << "À¸¾Ç WRITE!" << std::endl;
			throw _ERROR;
		} );

	// wait
	for( auto& thread : readThreadCont ) { thread.join(); }
	writeThreaed.join();
}