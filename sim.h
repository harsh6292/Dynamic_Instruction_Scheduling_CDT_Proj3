#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<limits.h>

class cache
{
	public:
		unsigned int c_size;
		unsigned int c_assoc;
		unsigned int c_blocksize;
		unsigned int c_numOfSets;
		int			 level;
		unsigned int* c_tagArray;
		unsigned int* valid_in_bit;
		int* LRUCounter;
		
		unsigned int readCounter;
		unsigned int readMissCounter;
	
		class cache* nextLevel;

};


extern class cache l1Cache;
extern class cache l2Cache;
extern class cache victimCache;

extern int powerOfTwo(int  num);
extern void extractAddressParams(unsigned int addressInInt, class cache* l1Cache, unsigned int* indexLocation, unsigned int* tagAddress);
extern int readFromAddress(class cache* cache_ds, unsigned int addressInInt);

extern void LRUForHit(class cache* l1Cache, unsigned int indexLocation, unsigned int tagLocation);
extern void LRUForMiss(class cache* l1Cache, unsigned int indexLocation, unsigned int* tagLocation);
