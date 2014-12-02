#include "sim.h"


enum instrn_state_t {IF, ID, IS, EX, WB};
enum reg_State_t {READY, NOTREADY};

class Fake_ROB_Class
{
	public:
		instrn_state_t 	intr_state;
		unsigned int 	prog_Counter;
		int 			operation_t;
		int 			destn_Reg;
		int 			src_Reg_1;
		int 			src_Reg_2;
		int				orig_Src_Reg_1;
		int				orig_Src_Reg_2;
		unsigned int 	mem_Address;
		int 			latency;
		unsigned int 	seq_Number;
		reg_State_t 	src_Reg_1_state;
		reg_State_t 	src_Reg_2_state;
		reg_State_t		readyState;

		int 			valid_Entry;
		int				fetch_Begin_Cycle;
		int				decode_Begin_Cycle;
		int				issue_Begin_Cycle;
		int				execute_Begin_Cycle;
		int				wb_Begin_Cycle;
		int				wb_End_Cycle;


}fake_ROB[1024];


class Register_File_Class
{
	public:
		reg_State_t ready;
		int tag;

}register_File_Table[128];




unsigned int global_Seq_num 	= 0;

unsigned int dispatch_Q_Size 	= 0;
unsigned int peak_N_size 		= 0;

unsigned int max_S_value		= 0;
unsigned int sched_Q_size 		= 0;

unsigned int issue_List_Size	= 0;

unsigned int minTagValue 		= 0;
unsigned int minTagISVal		= 0;

unsigned int traceDepleted		= 0;


unsigned int min_Seq_num		= 0;


void init_Register_table();
int return_Fake_ROB_pos();
void fetch(FILE *traceFile, int *dispatch_Q);
void dispatch(FILE *traceFile, int *dispatch_Q, int *sched_Q);
void issue(FILE *traceFile, int*, int*);
void execute(FILE *traceFile, int*, int*);
void fake_retire();
int advance_cycle();
int return_Queue_position(int *queue, int maxSize);
void insertionSortList(int *array, int sizeOfArray);



int count_Cycle = 0, max_Latency = 0;


int main(int agrc, char* argv[])
{

	int 			i = 0, noOfTagEntries = 0, j =0;
	unsigned int 	blockSize = 0, cache_L1_size = 0, cache_L2_size = 0;
	unsigned int 	cache_L1_assoc = 0, cache_L2_assoc = 0, size_N = 0, sched_size = 0;
	char 			*traceFileName;
	FILE 			*traceFile;

	sched_size 		= 	atoi(argv[1]);
	size_N			= 	atoi(argv[2]);
	blockSize 		= 	atoi(argv[3]);
	cache_L1_size 	= 	atoi(argv[4]);
	cache_L1_assoc	=	atoi(argv[5]);
	cache_L2_size	=	atoi(argv[6]);
	cache_L2_assoc 	=	atoi(argv[7]);
	traceFileName 	= 	argv[8];



	if( cache_L1_size == 0 )
	{
		max_Latency = 5;
		l1Cache.c_size = cache_L1_size;
	}
	else
	{
		max_Latency = 20;
		l1Cache.c_blocksize = blockSize;
		l1Cache.c_size = cache_L1_size;
		l1Cache.c_assoc = cache_L1_assoc;
		
		l1Cache.c_numOfSets = (l1Cache.c_size/(l1Cache.c_blocksize*l1Cache.c_assoc));
	
		//Initialize L1 Cache
		noOfTagEntries = (l1Cache.c_numOfSets*l1Cache.c_assoc);
		l1Cache.c_tagArray = (unsigned int*)malloc( (noOfTagEntries*sizeof(unsigned int)) );
		l1Cache.valid_in_bit = (unsigned int*)malloc( (noOfTagEntries*sizeof(unsigned int)) );
		l1Cache.LRUCounter = (int*)malloc( (noOfTagEntries*sizeof(int)) );

		memset( l1Cache.c_tagArray, 0, (sizeof(l1Cache.c_tagArray[0])*noOfTagEntries) );
		memset( l1Cache.valid_in_bit, 0, (sizeof(l1Cache.valid_in_bit[0])*noOfTagEntries) );
		memset( l1Cache.LRUCounter, 0, (sizeof(l1Cache.LRUCounter[0])*noOfTagEntries) );


		l1Cache.level = 1;
		l1Cache.readCounter = 0;
		l1Cache.readMissCounter = 0;
		l1Cache.nextLevel = NULL;
		
	}
	
	if( cache_L2_size != 0 )
	{
		l2Cache.c_blocksize = blockSize;
		l2Cache.c_size = cache_L2_size;
		l2Cache.c_assoc = cache_L2_assoc;
		l1Cache.nextLevel = &l2Cache;
		
		
		
		l2Cache.c_numOfSets = (l2Cache.c_size/(l2Cache.c_blocksize*l2Cache.c_assoc));
		
		noOfTagEntries = l2Cache.c_numOfSets*l2Cache.c_assoc;
		l2Cache.c_tagArray = (unsigned int*)malloc( (noOfTagEntries*sizeof(unsigned int)) );
		l2Cache.valid_in_bit = (unsigned int*)malloc( (noOfTagEntries*sizeof(unsigned int)) );
		l2Cache.LRUCounter = (int*)malloc( (noOfTagEntries*sizeof(int)) );


		memset( l2Cache.c_tagArray, 0, (sizeof(l2Cache.c_tagArray[0])*noOfTagEntries) );
		memset( l2Cache.valid_in_bit, 0, (sizeof(l2Cache.valid_in_bit[0])*noOfTagEntries) );
		memset( l2Cache.LRUCounter, 0, (sizeof(l2Cache.LRUCounter[0])*noOfTagEntries) );


		l2Cache.level = 2;
		l2Cache.readCounter = 0;
		l2Cache.readMissCounter = 0;
	}
	else
	{
		l2Cache.c_size = cache_L2_size;
	}
	
	
	peak_N_size = size_N;
	max_S_value = sched_size;

	int 	dispatch_Q[2*peak_N_size];
	int 	sched_Q[max_S_value];
	int 	execute_Q[(peak_N_size*max_Latency)];

	for( i = 0; i<((int)(2*peak_N_size)); i++ )
	{
		dispatch_Q[i] = -1;
	}

	for( i = 0; i<(int)max_S_value; i++ )
	{
		sched_Q[i] = -1;
	}

	for( i = 0; i<((int)(max_Latency*peak_N_size)); i++ )
	{
		execute_Q[i] = -1;
	}


	for( i =0; i<1024; i++)
	{
		fake_ROB[i].valid_Entry = -1;
		fake_ROB[i].seq_Number = -1;
	}

	if( (traceFile = fopen(traceFileName, "r")) == NULL )
	{
		printf("\nUnable to Open Trace File");
	}

	init_Register_table();

	
	do
	{
		
		fake_retire();
		execute(traceFile, sched_Q, execute_Q);
		issue(traceFile, sched_Q, execute_Q);
		dispatch(traceFile, dispatch_Q, sched_Q);
		fetch(traceFile, dispatch_Q);
		
		count_Cycle++;
		
	}while(advance_cycle() == 1);


	//Final Values after all cycles are completed
	count_Cycle--;
	
	if( l1Cache.c_size!= 0)
	{
		printf("L1 CACHE CONTENTS\n");
		printf("a. number of accesses :%d\n", l1Cache.readCounter);
		printf("b. number of misses :%d\n", l1Cache.readMissCounter);

		for( i=0; i<(int)l1Cache.c_numOfSets; i++)
		{		
			printf("set %d	:", i);
			for( j=0; j<(int)l1Cache.c_assoc; j++)
			{
				printf("%-10x",l1Cache.c_tagArray[i + (j*l1Cache.c_numOfSets)]);
			}
			printf("\n");
		}
		printf("\n");
	}
	
	
	if( l2Cache.c_size!= 0)
	{
		printf("L2 CACHE CONTENTS\n");
		printf("a. number of accesses :%d\n", l2Cache.readCounter);
		printf("b. number of misses :%d\n", l2Cache.readMissCounter);
		for( i=0; i<(int)l2Cache.c_numOfSets; i++)
		{
			printf("set %d: ", i);
			for( j=0; j<(int)l2Cache.c_assoc; j++)
			{
				printf("%-10x",l2Cache.c_tagArray[i + (j*l2Cache.c_numOfSets)]);
			}
			printf("\n");
		}
		printf("\n");
	}
	
	
	printf("CONFIGURATION\n");
	printf(" superscalar bandwidth (N) = %d\n", peak_N_size);
	printf(" dispatch queue size (2*N) = %d\n", (2*peak_N_size));
	printf(" schedule queue size (S)   = %d\n", max_S_value);
	printf("RESULTS\n");
	printf(" number of instructions = %d\n", global_Seq_num);
	printf(" number of cycles       = %d\n", count_Cycle);
	printf(" IPC                    = %.2f\n", ((double)global_Seq_num/count_Cycle) );
	
	return(0);
	
}




void init_Register_table()
{
	int i =0;
	for(i=0; i< 128; i++)
	{
		register_File_Table[i].ready = READY;
		register_File_Table[i].tag = 0;
	}
}


int return_Fake_ROB_pos()
{
	int i = 0;
	for( i = 0; i< 1024; i++)
	{
		if(fake_ROB[i].valid_Entry == -1)
		{
			return(i);
		}
	}
	return(-1);
}


void fetch(FILE *traceFile, int* dispatch_Q)
{
	char *readBuf, *tempString, *temp;
	int instructionsToRead = 0, i = 0, position = 0, qPos = 0;
	
	
	if( peak_N_size > ((peak_N_size*2) - dispatch_Q_Size) )
	{
		instructionsToRead = ((peak_N_size*2) - dispatch_Q_Size);
	}
	else
	{
		instructionsToRead = peak_N_size;
	}
	
	
	//printf("\nDispatch_Q_Size: %d, peak_N*2: %d, intrn to read: %d", dispatch_Q_Size, (2*peak_N_size), instructionsToRead);

	readBuf = (char*)malloc(sizeof(char)*100);
	
	while( (i<instructionsToRead) && ( (temp = fgets(readBuf, 100, traceFile))!=NULL ) )
	{
		//printf("\nFetching: i = %d, instrn to read: %d, peak_N_size: %d", i, instructionsToRead, peak_N_size);
		if( temp == (char*)NULL)
		{
			traceDepleted = 1;
			break;
		}

		if( (position = return_Fake_ROB_pos()) != -1 )
		{
			//printf("\nPosition returned: %d", position);
			tempString = strtok(readBuf, " ");
			fake_ROB[position].intr_state = IF;
			fake_ROB[position].prog_Counter = strtoll(tempString, NULL, 16);
			
			tempString = strtok(NULL, " ");
			fake_ROB[position].operation_t = atoi(tempString);
			
			tempString = strtok(NULL, " ");
			fake_ROB[position].destn_Reg = atoi(tempString);
			
			tempString = strtok(NULL, " ");
			fake_ROB[position].src_Reg_1 = atoi(tempString);
			fake_ROB[position].orig_Src_Reg_1 = atoi(tempString);
			
			tempString = strtok(NULL, " ");
			fake_ROB[position].src_Reg_2 = atoi(tempString);
			fake_ROB[position].orig_Src_Reg_2 = atoi(tempString);
			
			tempString = strtok(NULL, " \n");
			fake_ROB[position].mem_Address = strtoll(tempString, NULL, 16);
			
			fake_ROB[position].seq_Number = global_Seq_num;


			fake_ROB[position].src_Reg_1_state = NOTREADY;
			fake_ROB[position].src_Reg_2_state = NOTREADY;
			fake_ROB[position].readyState = NOTREADY;
			fake_ROB[position].valid_Entry = 2;
			fake_ROB[position].fetch_Begin_Cycle = count_Cycle;

			if( fake_ROB[position].operation_t == 0)
			{
				fake_ROB[position].latency = 1;
			}
			else if( fake_ROB[position].operation_t == 1)
			{
				fake_ROB[position].latency = 2;
			}
			else if( fake_ROB[position].operation_t == 2)
			{
				fake_ROB[position].latency = 5;
			}

			
			qPos = return_Queue_position(dispatch_Q, (2*peak_N_size));
			dispatch_Q[qPos] = position;
			
			//printf("\nFETCH(): Dispatch_Q[%d]: %d  <--Instrn", qPos, fake_ROB[position].seq_Number);
			
			dispatch_Q_Size++;
			global_Seq_num = (global_Seq_num + 1);
			i++;
		}
	}

}



int return_Queue_position(int *queue, int maxSize)
{
	int i =0;
	
	for( i = 0;i<maxSize; i++)
	{
		if(queue[i] ==-1)
		{
			return(i);
		}
	}
	
	return(-1);
	
}


void insertionSortList(int *array, int sizeOfArray)
{
	int i = 0, j = 0, temp = 0, count = 0;
	int tempArray[sizeOfArray];
	
	
	for( i = 0; i< sizeOfArray; i++)
	{
		if(array[i]!=-1)
		{
			tempArray[count++] = array[i];
		}
	}
	
	
	for( i = 1; i<count; i++)
	{
		temp = tempArray[i];
		j = i-1;
		
		while( (j >= 0) && (fake_ROB[temp].seq_Number < fake_ROB[tempArray[j]].seq_Number) )
		{
			tempArray[j+1] = tempArray[j];
			j--;
		}
			
		tempArray[j+1] = temp;
	}
	
	for( i = 0; i<count; i++ )
	{
		array[i] = tempArray[i];
	}
	
	for( i = count; i<sizeOfArray; i++ )
	{
		array[i] = -1;
	}
}



void dispatch(FILE *traceFile, int *dispatch_Q, int *sched_Q)
{
	int i = 0, tempPos = 0;
	reg_State_t src1Ready, src2Ready;

	src1Ready = NOTREADY;
	src2Ready = NOTREADY;

	
	insertionSortList(dispatch_Q, (2*peak_N_size) );
	
	
	for( i =0; i<((int)(2*peak_N_size)); i++)
	{
		if(sched_Q_size < max_S_value)
		{
			//printf("\nDISPatcH(): sched_Q %d < max_S_value %d", sched_Q_size, max_S_value);
			if( (dispatch_Q[i] != -1) && (fake_ROB[dispatch_Q[i]].intr_state == ID) )
			{
				//printf("\nDISPatcH(): sched_Q size=%d < max_S_value=%d,    and dispatch_Q[%d] = %d", sched_Q_size, max_S_value, i, dispatch_Q[i]);
				//if( fake_ROB[dispatch_Q[i]].seq_Number == minTagValue)
				{
					if( (tempPos = return_Queue_position(sched_Q, max_S_value)) != -1)
					{
						//printf("\nDipatching Instr: %d into sched-Q[%d]", fake_ROB[dispatch_Q[i]].seq_Number, tempPos );
						sched_Q[tempPos] = dispatch_Q[i];
						sched_Q_size++;
						dispatch_Q_Size--;

						//minTagValue += 1;
						
						fake_ROB[dispatch_Q[i]].intr_state = IS;

						src1Ready = NOTREADY;
						src2Ready = NOTREADY;

						
						//printf("\nInstr: %d  has SRC1: %d,  SRC2: %d", fake_ROB[dispatch_Q[i]].seq_Number, fake_ROB[dispatch_Q[i]].src_Reg_1, fake_ROB[dispatch_Q[i]].src_Reg_2);
						if(fake_ROB[dispatch_Q[i]].src_Reg_1 != -1)
						{
							if( fake_ROB[dispatch_Q[i]].src_Reg_1_state == READY)
							{
								src1Ready = READY;
								//printf("\nSRC_REG_STATE_1 Ready");
							}
							else if( register_File_Table[fake_ROB[dispatch_Q[i]].src_Reg_1].ready == READY)
							{
								src1Ready = READY;
								fake_ROB[dispatch_Q[i]].src_Reg_1_state = READY;
								//printf("\nREG_TABLE[src_reg_1] Ready");
							}
							else
							{
								src1Ready = NOTREADY;
								fake_ROB[dispatch_Q[i]].src_Reg_1 = register_File_Table[fake_ROB[dispatch_Q[i]].src_Reg_1].tag; 
								//printf("\nREG_TABLE[src_reg_1] NOT READY");
							}
						}
						else
						{
							src1Ready = READY;
							fake_ROB[dispatch_Q[i]].src_Reg_1_state = READY;
							//printf("\nSRC_REG_1 =-1 => Ready");
						}
						
						
						if(fake_ROB[dispatch_Q[i]].src_Reg_2 != -1)
						{
							if( fake_ROB[dispatch_Q[i]].src_Reg_2_state == READY)
							{
								src2Ready = READY;
								//printf("\nSRC_REG_STATE_2 Ready");
							}
							else if( register_File_Table[fake_ROB[dispatch_Q[i]].src_Reg_2].ready == READY)
							{
								src2Ready = READY;
								fake_ROB[dispatch_Q[i]].src_Reg_2_state = READY;
								//printf("\nREG_TABLE[src_reg_2] Ready");
							}
							else
							{
								src2Ready = NOTREADY;
								fake_ROB[dispatch_Q[i]].src_Reg_2 = register_File_Table[fake_ROB[dispatch_Q[i]].src_Reg_2].tag;
								//printf("\nREG_TABLE[src_reg_2] NOT READY");
							}
								
						}
						else
						{
							src2Ready = READY;
							fake_ROB[dispatch_Q[i]].src_Reg_2_state = READY;
							//printf("\nSRC_REG_2 =-1 => Ready");
						}


						if( (src1Ready == READY) && (src2Ready == READY) )
						{
							//printf("\nDISPATCH(): Instr :%d is ready", fake_ROB[dispatch_Q[i]].seq_Number);
							fake_ROB[dispatch_Q[i]].readyState = READY;
						}
						else
						{
							//printf("\nDISPATCH(): Instr :%d is NOT READY", fake_ROB[dispatch_Q[i]].seq_Number);
							fake_ROB[dispatch_Q[i]].readyState = NOTREADY;
						}


						if(fake_ROB[dispatch_Q[i]].destn_Reg != -1)
						{
							register_File_Table[fake_ROB[dispatch_Q[i]].destn_Reg].ready = NOTREADY;
							//check here 
							register_File_Table[fake_ROB[dispatch_Q[i]].destn_Reg].tag = dispatch_Q[i];
							//printf("\nDestn Reg: %d Is NOT Ready", fake_ROB[dispatch_Q[i]].destn_Reg);
						}
						
						fake_ROB[dispatch_Q[i]].issue_Begin_Cycle = count_Cycle;

						dispatch_Q[i] = -1;

					}
					
				}
			}
		}
		else
		{
			break;
		}
	}



	for( i =0; i<((int)(2*peak_N_size)); i++)
	{
		
		if( dispatch_Q[i] != -1 )
		{
			
			if( fake_ROB[dispatch_Q[i]].intr_state == IF )
			{
				//printf("\nIF -> ID, Decoding Instrn: %d", fake_ROB[dispatch_Q[i]].seq_Number);
				fake_ROB[dispatch_Q[i]].intr_state = ID;
				fake_ROB[dispatch_Q[i]].decode_Begin_Cycle = count_Cycle;
			}
		}
	}
}





void issue(FILE *traceFile, int *sched_Q, int *execute_Q)
{
	int i = 0, j = 0, tempPos = 0, getVal = 0;
	reg_State_t src1Ready, src2Ready;
	
	src1Ready = NOTREADY;
	src2Ready = NOTREADY;


	insertionSortList(sched_Q, (max_S_value) );
	

	i = 0;
	j = 0;
	//For operation type 2
	for( i = 0; i< (int)max_S_value; i++) //while( j < peak_N_size)
	{
		if( j < (int)peak_N_size)	//for( i = 0; i<max_S_value; i++)
		{
		
			//printf("\nISSUE(): Sched_q[%d]: %d,  j: %d", i, sched_Q[i], j);
		
			if( (sched_Q[i] != -1) && (fake_ROB[sched_Q[i]].intr_state == IS))
			{
				if( (fake_ROB[sched_Q[i]].readyState == READY) )//&& (fake_ROB[sched_Q[i]].operation_t == 2) ) //fake_ROB[sched_Q[i]].seq_Number == minTagISVal)
				{
					if( (tempPos = return_Queue_position(execute_Q, (5*peak_N_size))) != -1)
					{
						
						//printf("\nISSUE(): OP-2  .... Instruction  %d is ready", fake_ROB[sched_Q[i]].seq_Number);
						execute_Q[tempPos] = sched_Q[i];
						//printf("			ISSUE(): exec_Q[%d]: %d", tempPos, fake_ROB[execute_Q[tempPos]].seq_Number);
						
						issue_List_Size++;
						sched_Q_size--;

						//minTagISVal += 1;
						
						
						
						if(fake_ROB[sched_Q[i]].operation_t == 2)
						{
							
							if( l1Cache.c_size != 0)
							{
								
								getVal = readFromAddress(&l1Cache, fake_ROB[sched_Q[i]].mem_Address);
								
								//A return value of 0 indicates that the address was hit in L1 Cache
								if( getVal == 0 )
								{
									fake_ROB[sched_Q[i]].latency = 5;
								}
								
								//A return value of 1 indicates that the address was miss in L1 Cache but hit in L2 cache
								if( getVal == 1 )
								{
									fake_ROB[sched_Q[i]].latency = 10;
								}
								
								//A return value of 2 indicates that the address was miss both in L1 & L2 Cache
								if( getVal == 2 )
								{
									fake_ROB[sched_Q[i]].latency = 20;
								}
							}
						}
						
						
						
						
						
						
						
						
						

						fake_ROB[sched_Q[i]].intr_state = EX;
						fake_ROB[sched_Q[i]].execute_Begin_Cycle = count_Cycle;


						sched_Q[i] = -1;
						j++;
					}
				}
			
			}
			
			
		}
		
	
	}
	
}



void execute(FILE *traceFile, int *sched_Q, int *execute_Q)
{
	int i = 0, j = 0;

	
	
	
	for( i = 0; i< ((int)(max_Latency*peak_N_size)); i++)
	{
		
		
		
		//For operation type 0
		if(execute_Q[i] != -1)
		{
			
			if( fake_ROB[execute_Q[i]].latency >0 )
			{
				fake_ROB[execute_Q[i]].latency = (fake_ROB[execute_Q[i]].latency - 1);
			}
			
			
			//printf("\nEXECUTE(): OP-0   Instr: exec_Q[%d]: %d   has latency: %d", i, fake_ROB[execute_Q[i]].seq_Number, fake_ROB[execute_Q[i]].latency);
			if( (fake_ROB[execute_Q[i]].latency == 0) && (fake_ROB[execute_Q[i]].intr_state == EX))
			{
				//printf("\nEXECUTE(): Executing Instr..............: %d", fake_ROB[execute_Q[i]].seq_Number);
				
				fake_ROB[execute_Q[i]].intr_state = WB;
				issue_List_Size--;
				
				
				//printf("\nMaking Destn reg: %d   ready if reg[tag]%d matches this instruction %d    ----------====", fake_ROB[execute_Q[i]].destn_Reg, register_File_Table[fake_ROB[execute_Q[i]].destn_Reg].tag, execute_Q[i]);
				if(fake_ROB[execute_Q[i]].destn_Reg != -1)
				{
					if( register_File_Table[fake_ROB[execute_Q[i]].destn_Reg].tag == execute_Q[i] )
					{
						register_File_Table[fake_ROB[execute_Q[i]].destn_Reg].ready = READY;
						//printf("\nTag and instruction mathces, Making destn reg READY    ----------****");
					}
					
				}
				
				
				fake_ROB[execute_Q[i]].wb_Begin_Cycle = count_Cycle;
				

				
				for( j = 0; j< (int)max_S_value; j++)
				{
					if( (sched_Q[j]!=-1) && (fake_ROB[sched_Q[j]].readyState == NOTREADY) )
					{
						//printf("\nEXECUTE(): Sched_Q[j-->%d] instr not READY: %d", j, sched_Q[j]);
						
						if(fake_ROB[sched_Q[j]].intr_state == IS)
						{
							//printf("\nEXECUTE(): Instr: %d, is in IS state", sched_Q[j]);
							
							if( fake_ROB[sched_Q[j]].src_Reg_1 == execute_Q[i])
							{
								//printf("\nEXECUTE(): Instr src_reg_1: %d ...has dependency on instr: %d", fake_ROB[sched_Q[j]].seq_Number, fake_ROB[execute_Q[i]].seq_Number);
								
								fake_ROB[sched_Q[j]].src_Reg_1_state = READY;
							}
							if( fake_ROB[sched_Q[j]].src_Reg_2 == execute_Q[i])
							{
								//printf("\nEXECUTE(): Instr src_reg_2: %d ...has dependency on instr: %d", fake_ROB[sched_Q[j]].seq_Number, fake_ROB[execute_Q[i]].seq_Number);
								
								fake_ROB[sched_Q[j]].src_Reg_2_state = READY;
							}
							
							
							if( (fake_ROB[sched_Q[j]].src_Reg_1_state == READY) && (fake_ROB[sched_Q[j]].src_Reg_2_state == READY) )
							{
								//printf("\nEXECUTE(): Instr: %d ...is now FULLY READY-----", fake_ROB[sched_Q[j]].seq_Number);
								fake_ROB[sched_Q[j]].readyState = READY;
							}
							
							
						}
					}
				}

				execute_Q[i] = -1;
			}

		}
			
	}
}





void fake_retire()
{
	int  i = 0, j = 0;
	int tempList[1024];

	
	for( i = 0; i< 1024; i++)
	{
		if( (fake_ROB[i].intr_state == WB) && (fake_ROB[i].valid_Entry == 2) )
		{
			fake_ROB[i].wb_End_Cycle = count_Cycle;
			fake_ROB[i].valid_Entry = 3;
		}
	}

	
	for( i = 0; i< 1024; i++)
	{
		if( (fake_ROB[i].intr_state == WB) && (fake_ROB[i].valid_Entry == 3) )
		{
			tempList[i] = i;
		}
		else
		{
			tempList[i] = -1;
		}

	}
	

	insertionSortList(tempList, 1024);
	

	for( j = 0; j< 1024; j++)
	{

		if(tempList[j] != -1)
		{
			i = tempList[j];
			//printf("\n Pos to 1024:::: fake_ROB[%d].seq_number: %d", i , fake_ROB[i].seq_Number);
			if( (fake_ROB[i].intr_state == WB) && (fake_ROB[i].valid_Entry == 3) )
			{
				if( min_Seq_num ==  fake_ROB[i].seq_Number)
				{
					printf("%d ", fake_ROB[i].seq_Number);
					printf("fu{%d} ", fake_ROB[i].operation_t);
					printf("src{%d,%d} ", fake_ROB[i].orig_Src_Reg_1, fake_ROB[i].orig_Src_Reg_2);
					printf("dst{%d} ", fake_ROB[i].destn_Reg);
					printf("IF{%d,%d} ", fake_ROB[i].fetch_Begin_Cycle, (fake_ROB[i].decode_Begin_Cycle - fake_ROB[i].fetch_Begin_Cycle));
					printf("ID{%d,%d} ", fake_ROB[i].decode_Begin_Cycle, (fake_ROB[i].issue_Begin_Cycle - fake_ROB[i].decode_Begin_Cycle));
					printf("IS{%d,%d} ", fake_ROB[i].issue_Begin_Cycle, (fake_ROB[i].execute_Begin_Cycle - fake_ROB[i].issue_Begin_Cycle));
					printf("EX{%d,%d} ", fake_ROB[i].execute_Begin_Cycle, (fake_ROB[i].wb_Begin_Cycle - fake_ROB[i].execute_Begin_Cycle));
					printf("WB{%d,%d}\n", fake_ROB[i].wb_Begin_Cycle, (fake_ROB[i].wb_End_Cycle -fake_ROB[i].wb_Begin_Cycle ));
					fake_ROB[i].valid_Entry = -1;
					
					min_Seq_num = (fake_ROB[i].seq_Number + 1);
				}
			}

			if( (fake_ROB[i].valid_Entry != -1) && (fake_ROB[i].intr_state != WB) )
			{
				
				//printf("\nBreaking at pos : %d, min seq no: %d", i, min_Seq_num);
				return;
			}
		}
		else
		{
			//printf("\nBreaking at TEMP POS : %d, min seq no: %d", i, min_Seq_num);
			return;
		}

	}
}




int advance_cycle()
{
	int i = 0, presentROB = 0;
	for( i = 0; i< 1024; i++)
	{
		if(fake_ROB[i].valid_Entry != -1 )
		{
			presentROB = 1;
			break;
		}
	}


	if( (presentROB == 0) || (traceDepleted == 1) )
	{
		return(0);
	}
	else
	{
		return(1);
	}
}
