#include "sim.h"

class cache l1Cache;
class cache l2Cache;

void extractAddressParams(unsigned int addressInInt, class cache* l1Cache, unsigned int* indexLocation, unsigned int* tagAddress);
int readFromAddress(class cache* cache_ds, unsigned int addressInInt);

void LRUForHit(class cache* l1Cache, unsigned int indexLocation, unsigned int tagLocation);
void LRUForMiss(class cache* l1Cache, unsigned int indexLocation, unsigned int* tagLocation);


int powerOfTwo(int num)
{
	if(num!=1)
	{
		while( (num%2 == 0) && num>1)
			num = num/2;

		if( num == 1)
			return(1);
	}
	
	return(0);
}



void extractAddressParams(unsigned int addressInInt, class cache* cache_ds, unsigned int* indexLocation, unsigned int* tagAddress)
{
	int noOfBlockBits = 0, noOfIndexBits = 0, tempIndexNo = 0, i=0;
	
	noOfBlockBits = log2(cache_ds->c_blocksize);
	noOfIndexBits = log2(cache_ds->c_numOfSets);

	*indexLocation = addressInInt>>noOfBlockBits;

	for( i=0; i<noOfIndexBits; i++)
	{
		tempIndexNo = ( 1 | tempIndexNo<<1 );
	}

	*indexLocation = ( *indexLocation & tempIndexNo );
	*tagAddress = addressInInt>>(noOfBlockBits + noOfIndexBits);
}


//Recursive solution
int readFromAddress(class cache* cache_ds, unsigned int addressInInt)
{
	int i=0, foundInvalidEntry = 0, noOfBlockBits = 0, noOfIndexBits = 0, getVal = 0;
	unsigned int tagLocation = 0, indexLocation = 0, tagAddress = 0, temptagLocation = 0;

	noOfBlockBits = log2(cache_ds->c_blocksize);
	noOfIndexBits = log2(cache_ds->c_numOfSets);


	cache_ds->readCounter +=1;
	extractAddressParams(addressInInt, cache_ds, &indexLocation, &tagAddress);


	for( i=0; i< (int)cache_ds->c_assoc; i++)
	{
		if( cache_ds->c_tagArray[indexLocation + (i*cache_ds->c_numOfSets)] == tagAddress )	//Checking Tag Entries
		{
		
			if( cache_ds->valid_in_bit[indexLocation + (i*cache_ds->c_numOfSets)] != 0 )
			{
				LRUForHit(cache_ds, indexLocation, ( indexLocation + (i*cache_ds->c_numOfSets) ) );
				if( cache_ds->level == 1)
					return(0);
				else
					return(10);
			}
		}
	}


	for( i=0; i< (int)cache_ds->c_assoc; i++)
	{
		if( cache_ds->valid_in_bit[indexLocation + (i*cache_ds->c_numOfSets)] == 0 )
		{
			cache_ds->valid_in_bit[indexLocation + (i*cache_ds->c_numOfSets)] = 1;
			tagLocation =  indexLocation + (i*cache_ds->c_numOfSets);
			foundInvalidEntry = 1;
			break;
		}
	}

	if( foundInvalidEntry == 1 )
	{
		cache_ds->readMissCounter += 1;

		if( cache_ds->nextLevel != NULL)
		{
			getVal = readFromAddress(cache_ds->nextLevel, addressInInt);
		}
		
		cache_ds->c_tagArray[tagLocation] = tagAddress;
		
		LRUForMiss(cache_ds, indexLocation, &temptagLocation);
		cache_ds->LRUCounter[tagLocation] = 0;
		
		
		if( cache_ds->nextLevel != NULL)
		{
			//l1 cache miss, l2 hit
			if( getVal == 10)
			{
				return(1);
			}
			else if( getVal == 20)		//l1 cache miss, l2 miss
			{
				return(2);
			}
		}
		
		
		//L1 cache only, no L2 cache then return 20 cycle latency
		if( cache_ds->level == 1)
			return(2);
		else
			return(20);	//L2 cache returning with miss, return 20 cycles
	}


	cache_ds->readMissCounter += 1;

	
	LRUForMiss(cache_ds, indexLocation, &tagLocation);
	cache_ds->LRUCounter[tagLocation] = 0;
	

	if( cache_ds->nextLevel != NULL)
	{
		getVal = readFromAddress(cache_ds->nextLevel, addressInInt);
	}

	cache_ds->c_tagArray[tagLocation] = tagAddress;
	
	
	if( cache_ds->nextLevel != NULL)
	{
		//l1 cache miss, l2 hit
		if( getVal == 10)
		{
			return(1);
		}
		else if( getVal == 20)		//l1 cache miss, l2 miss
		{
			return(2);
		}
	}
		
		
	//L1 cache only, no L2 cache then return 20 cycle latency
	if( cache_ds->level == 1)
		return(2);
	else
		return(20);	//L2 cache returning with miss, return 20 cycles
			
			

	return(0);
}





void LRUForHit(class cache* l1Cache, unsigned int indexLocation, unsigned int tagLocation)
{
	int i = 0;

	for( i=0; i< (int)l1Cache->c_assoc; i++)
	{
		if( (int)l1Cache->LRUCounter[indexLocation + (i*l1Cache->c_numOfSets)] < (int)l1Cache->LRUCounter[tagLocation] )	
		{
			l1Cache->LRUCounter[indexLocation + (i*l1Cache->c_numOfSets)] = ((int)l1Cache->LRUCounter[indexLocation + (i*l1Cache->c_numOfSets)]) + 1;
		}
	}
	
	l1Cache->LRUCounter[tagLocation] = 0;
}



void LRUForMiss(class cache* l1Cache, unsigned int indexLocation, unsigned int* tagLocation)
{
	unsigned int i = 0;
	int max = -1;
	*tagLocation = 0;
	for( i=0; i<l1Cache->c_assoc; i++)
	{
		if( (int)l1Cache->LRUCounter[indexLocation + (i*l1Cache->c_numOfSets)] > (int)max )
		{
			max = l1Cache->LRUCounter[indexLocation + (i*l1Cache->c_numOfSets)];
			*tagLocation = ( indexLocation + (i*l1Cache->c_numOfSets) );
		}
	}


	for( i=0; i<l1Cache->c_assoc; i++)
	{
		l1Cache->LRUCounter[indexLocation + (i*l1Cache->c_numOfSets)] = ((int)l1Cache->LRUCounter[indexLocation + (i*l1Cache->c_numOfSets)]) + 1;
	}
}
