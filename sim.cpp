#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<limits.h>


enum instrn_state_t {IF, ID, IS, EX, WB};
//enum instrn_state_t {IF=0, ID=1, IS=2, EX=3, WB=4};
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


struct BTB
{
	unsigned int size;
	unsigned int assoc;
	unsigned int blocksize;
	unsigned int numOfSets;

	unsigned int* tagArray;
	int* LRUCounter;

	unsigned int branchCounter;
	unsigned int BTBMissButBranchTaken;
	unsigned int noOfPredictionsFromBTB;
};





unsigned int global_Seq_num 	= 0;

unsigned int dispatch_Q_Size 	= 0;
unsigned int peak_N_size 		= 0;

unsigned int max_S_value		= 0;
unsigned int sched_Q_size 		= 0;

unsigned int issue_List_Size	= 0;
unsigned int IS_List_Size_OP_0	= 0;
unsigned int IS_List_Size_OP_1	= 0;
unsigned int IS_List_Size_OP_2	= 0;

unsigned int minTagValue 		= 0;
unsigned int minTagISVal		= 0;

unsigned int traceDepleted		= 0;


unsigned int min_Seq_num		= 0;


void init_Register_table();
int return_Fake_ROB_pos();
void fetch(FILE *traceFile, int *dispatch_Q);
void dispatch(FILE *traceFile, int *dispatch_Q, int *sched_Q);
void issue(FILE *traceFile, int*, int*, int*, int*);
void execute(FILE *traceFile, int*, int*, int*, int*);
void fake_retire();
int advance_cycle();
int return_Queue_position(int *queue, int maxSize);
void insertionSortList(int *array, int sizeOfArray);








int count_Cycle = 0;


int main(int agrc, char* argv[])
{

	int 			i = 0;//, count = 0;
	unsigned int 	blockSize = 0, cache_L1_size = 0, cache_L2_size = 0;
	unsigned int 	cache_L1_assoc = 0, cache_L2_assoc = 0, size_N = 0, sched_size = 0;
	char 			*traceFileName;
	FILE 			*traceFile;
	//Fake_ROB_Class	fake_ROB[1024];

	sched_size 		= 	atoi(argv[1]);
	size_N			= 	atoi(argv[2]);
	blockSize 		= 	atoi(argv[3]);
	cache_L1_size 	= 	atoi(argv[4]);
	cache_L1_assoc	=	atoi(argv[5]);
	cache_L2_size	=	atoi(argv[6]);
	cache_L2_assoc 	=	atoi(argv[7]);
	traceFileName 	= 	argv[8];


	peak_N_size = size_N;
	max_S_value = sched_size;

	int 	dispatch_Q[2*peak_N_size];
	int 	sched_Q[max_S_value];
	int 	execute_Q[peak_N_size];
	int		exec_Q_OP_0[(peak_N_size*5)];
	int 	exec_Q_OP_1[peak_N_size];
	int 	exec_Q_OP_2[peak_N_size];


	for( i = 0; i<(2*peak_N_size); i++ )
	{
		dispatch_Q[i] = -1;
	}

	for( i = 0; i<max_S_value; i++ )
	{
		sched_Q[i] = -1;
	}

	for( i = 0; i<(5*peak_N_size); i++ )
	{
		//execute_Q[i] = -1;
		exec_Q_OP_0[i] = -1;
		//exec_Q_OP_1[i] = -1;
		//exec_Q_OP_2[i] = -1;
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

	//printf("\nAfter initializing regiser tables");
	//printf("\nAfter initializing regiser tables");
	do
	{
		
		printf("\n\n\n************************  NEW_CYCLE() %d **************************\n", (count_Cycle));
		printf("\n---------------------------------------------------------------------------FAKE-ROB----");
		fake_retire();
		printf("\nFAKE-ROB Done!");
		
		printf("\n-------------------------------------------------------------EXECUTE()-----------------");
		execute(traceFile, sched_Q, exec_Q_OP_0, exec_Q_OP_1, exec_Q_OP_2);
		printf("\nEXECUTE() Done!, sched-Q size: %d, dispatch_Q_Size: %d, exec_Q size: %d", sched_Q_size, dispatch_Q_Size, issue_List_Size);
		
		printf("\n------------------------------------------------ISSUE()--------------------------------");
		issue(traceFile, sched_Q, exec_Q_OP_0, exec_Q_OP_1, exec_Q_OP_2);
		printf("\nISSUE() Done!, sched-Q size: %d, dispatch_Q_Size: %d", sched_Q_size, dispatch_Q_Size);
		
		printf("\n---------------------------DISPATCH()-------------------------------------------------");
		dispatch(traceFile, dispatch_Q, sched_Q);
		printf("\nDISPATCH() Done!, sched-Q size: %d, dispatch_Q_Size: %d", sched_Q_size, dispatch_Q_Size);
		
		printf("\n-------------FETCH()------------------------------------------------------------------");
		fetch(traceFile, dispatch_Q);
		printf("\nFETCH() Done!, sched-Q size: %d, dispatch_Q_Size: %d", sched_Q_size, dispatch_Q_Size);
		count_Cycle++;
		//if(count_Cycle == 200)
			//break;
	}while(advance_cycle() == 1);

	printf("\n");
	printf("\n\n------------------  FINISH()  -------------\n\n");
	count_Cycle--;
	printf("\nFinal cycle count: %d", count_Cycle);
	
	
	printf("\nCONFIGURATION");
	printf("\n superscalar bandwidth (N) = %d", peak_N_size);
	printf("\n dispatch queue size (2*N) = %d", (2*peak_N_size));
	printf("\n schedule queue size (S)   = %d", max_S_value);
	printf("\nRESULTS");
	printf("\n number of instructions = %d", global_Seq_num);
	printf("\n number of cycles       = %d", count_Cycle);
	printf("\n IPC                    = %.2f", ((double)global_Seq_num/count_Cycle) );
	
	printf("\n");


	return 0;
	
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
}


void fetch(FILE *traceFile, int* dispatch_Q)
{
	char *readBuf, *tempString, *temp;
	int instructionsToRead = 0, tempInt = 0, i = 0, position = 0, qPos = 0;
	
	
	if( peak_N_size > ((peak_N_size*2) - dispatch_Q_Size) )
	{
		instructionsToRead = ((peak_N_size*2) - dispatch_Q_Size);
	}
	else
	{
		instructionsToRead = peak_N_size;
	}
	
	
	 
	printf("\nDispatch_Q_Size: %d, peak_N*2: %d, intrn to read: %d", dispatch_Q_Size, (2*peak_N_size), instructionsToRead);

	readBuf = (char*)malloc(sizeof(char)*100);
	
	while( (i<instructionsToRead) && ( (temp = fgets(readBuf, 100, traceFile))!=NULL ) )
	{
		//printf("\nFetching: i = %d, instrn to read: %d, peak_N_size: %d", i, instructionsToRead, peak_N_size);
		if( temp == (char*)NULL)
		{
			traceDepleted = 1;
			break;
		}

		position = return_Fake_ROB_pos();
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
		
		printf("\nFETCH(): Dispatch_Q[%d]: %d  <--Instrn", qPos, fake_ROB[position].seq_Number);
		
		dispatch_Q_Size++;
		global_Seq_num = (global_Seq_num + 1);
		i++;
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
	int i = 0, j = 0, temp = 0, key = 0, count = 0;
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
	int i = 0, j =0;
	int minTag = 0, instrn_In_SchedQ = 0, tempPos = 0;
	reg_State_t src1Ready, src2Ready;
	src1Ready = NOTREADY;
	src2Ready = NOTREADY;

	int tempListInstrnID[dispatch_Q_Size];

	
	insertionSortList(dispatch_Q, (2*peak_N_size) );
	
	{
		for( i =0; i<(2*peak_N_size); i++)
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
							printf("\nDipatching Instr: %d into sched-Q[%d]", fake_ROB[dispatch_Q[i]].seq_Number, tempPos );
							sched_Q[tempPos] = dispatch_Q[i];
							sched_Q_size++;
							dispatch_Q_Size--;

							//minTagValue += 1;
							
							fake_ROB[dispatch_Q[i]].intr_state = IS;

							src1Ready = NOTREADY;
							src2Ready = NOTREADY;

							
							printf("\nInstr: %d  has SRC1: %d,  SRC2: %d", fake_ROB[dispatch_Q[i]].seq_Number, fake_ROB[dispatch_Q[i]].src_Reg_1, fake_ROB[dispatch_Q[i]].src_Reg_2);
							if(fake_ROB[dispatch_Q[i]].src_Reg_1 != -1)
							{
								if( fake_ROB[dispatch_Q[i]].src_Reg_1_state == READY)
								{
									src1Ready = READY;
									printf("\nSRC_REG_STATE_1 Ready");
								}
								else if( register_File_Table[fake_ROB[dispatch_Q[i]].src_Reg_1].ready == READY)
								{
									src1Ready = READY;
									fake_ROB[dispatch_Q[i]].src_Reg_1_state = READY;
									printf("\nREG_TABLE[src_reg_1] Ready");
								}
								else
								{
									src1Ready = NOTREADY;
									fake_ROB[dispatch_Q[i]].src_Reg_1 = register_File_Table[fake_ROB[dispatch_Q[i]].src_Reg_1].tag; 
									printf("\nREG_TABLE[src_reg_1] NOT READY");
								}
							}
							else
							{
								src1Ready = READY;
								fake_ROB[dispatch_Q[i]].src_Reg_1_state = READY;
								printf("\nSRC_REG_1 =-1 => Ready");
							}
							
							
							if(fake_ROB[dispatch_Q[i]].src_Reg_2 != -1)
							{
								if( fake_ROB[dispatch_Q[i]].src_Reg_2_state == READY)
								{
									src2Ready = READY;
									printf("\nSRC_REG_STATE_2 Ready");
								}
								else if( register_File_Table[fake_ROB[dispatch_Q[i]].src_Reg_2].ready == READY)
								{
									src2Ready = READY;
									fake_ROB[dispatch_Q[i]].src_Reg_2_state = READY;
									printf("\nREG_TABLE[src_reg_2] Ready");
								}
								else
								{
									src2Ready = NOTREADY;
									fake_ROB[dispatch_Q[i]].src_Reg_2 = register_File_Table[fake_ROB[dispatch_Q[i]].src_Reg_2].tag;
									printf("\nREG_TABLE[src_reg_2] NOT READY");
								}
									
							}
							else
							{
								src2Ready = READY;
								fake_ROB[dispatch_Q[i]].src_Reg_2_state = READY;
								printf("\nSRC_REG_2 =-1 => Ready");
							}


							if( (src1Ready == READY) && (src2Ready == READY) )
							{
								printf("\nDISPATCH(): Instr :%d is ready", fake_ROB[dispatch_Q[i]].seq_Number);
								fake_ROB[dispatch_Q[i]].readyState = READY;
							}
							else
							{
								printf("\nDISPATCH(): Instr :%d is NOT READY", fake_ROB[dispatch_Q[i]].seq_Number);
								fake_ROB[dispatch_Q[i]].readyState = NOTREADY;
							}


							if(fake_ROB[dispatch_Q[i]].destn_Reg != -1)
							{
								register_File_Table[fake_ROB[dispatch_Q[i]].destn_Reg].ready = NOTREADY;
								//check here 
								register_File_Table[fake_ROB[dispatch_Q[i]].destn_Reg].tag = dispatch_Q[i];
								printf("\nDestn Reg: %d Is NOT Ready", fake_ROB[dispatch_Q[i]].destn_Reg);
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
	}



	for( i =0; i<(2*peak_N_size); i++)
	{
		
		if( dispatch_Q[i] != -1 )
		{
			
			if( fake_ROB[dispatch_Q[i]].intr_state == IF )
			{
				printf("\nIF -> ID, Decoding Instrn: %d", fake_ROB[dispatch_Q[i]].seq_Number);
				fake_ROB[dispatch_Q[i]].intr_state = ID;
				fake_ROB[dispatch_Q[i]].decode_Begin_Cycle = count_Cycle;
			}
		}
	}
}





void issue(FILE *traceFile, int *sched_Q, int *exec_Q_OP_0, int *exec_Q_OP_1, int *exec_Q_OP_2)
{
	int i = 0, j = 0, tempPos = 0;
	int minTag = 0, instrn_In_SchedQ = 0;
	reg_State_t src1Ready, src2Ready;
	src1Ready = NOTREADY;
	src2Ready = NOTREADY;


	insertionSortList(sched_Q, (max_S_value) );
	

	i = 0;
	j = 0;
	//For operation type 2
	for( i = 0; i<max_S_value; i++) //while( j < peak_N_size)
	{
		if( j < peak_N_size)	//for( i = 0; i<max_S_value; i++)
		{
		
			//printf("\nISSUE(): Sched_q[%d]: %d,  j: %d", i, sched_Q[i], j);
		
			if( (sched_Q[i] != -1) && (fake_ROB[sched_Q[i]].intr_state == IS))
			{
				if( (fake_ROB[sched_Q[i]].readyState == READY) )//&& (fake_ROB[sched_Q[i]].operation_t == 2) ) //fake_ROB[sched_Q[i]].seq_Number == minTagISVal)
				{
					if( (tempPos = return_Queue_position(exec_Q_OP_0, (5*peak_N_size))) != -1)
					{
						
						printf("\nISSUE(): OP-2  .... Instruction  %d is ready", fake_ROB[sched_Q[i]].seq_Number);
						exec_Q_OP_0[tempPos] = sched_Q[i];
						printf("			ISSUE(): exec_Q[%d]: %d", tempPos, fake_ROB[exec_Q_OP_0[tempPos]].seq_Number);
						IS_List_Size_OP_0++;
						//issue_List_Size++;
						sched_Q_size--;

						//minTagISVal += 1;

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



void execute(FILE *traceFile, int *sched_Q, int *exec_Q_OP_0, int *exec_Q_OP_1, int *exec_Q_OP_2)
{
	int i = 0, j = 0;

	
	
	
	for( i = 0; i< (5*peak_N_size); i++)
	{
		
		
		
		//For operation type 0
		if(exec_Q_OP_0[i] != -1)
		{
			
			if( fake_ROB[exec_Q_OP_0[i]].latency >0 )
			{
				fake_ROB[exec_Q_OP_0[i]].latency = (fake_ROB[exec_Q_OP_0[i]].latency - 1);
			}
			
			
			printf("\nEXECUTE(): OP-0   Instr: exec_Q[%d]: %d   has latency: %d", i, fake_ROB[exec_Q_OP_0[i]].seq_Number, fake_ROB[exec_Q_OP_0[i]].latency);
			if( (fake_ROB[exec_Q_OP_0[i]].latency == 0) && (fake_ROB[exec_Q_OP_0[i]].intr_state == EX))
			{
				printf("\nEXECUTE(): Executing Instr..............: %d", fake_ROB[exec_Q_OP_0[i]].seq_Number);
				
				fake_ROB[exec_Q_OP_0[i]].intr_state = WB;
				IS_List_Size_OP_0--;
				
				
				printf("\nMaking Destn reg: %d   ready if reg[tag]%d matches this instruction %d    ----------====", fake_ROB[exec_Q_OP_0[i]].destn_Reg, register_File_Table[fake_ROB[exec_Q_OP_0[i]].destn_Reg].tag, exec_Q_OP_0[i]);
				if(fake_ROB[exec_Q_OP_0[i]].destn_Reg != -1)
				{
					if( register_File_Table[fake_ROB[exec_Q_OP_0[i]].destn_Reg].tag == exec_Q_OP_0[i] )
					{
						register_File_Table[fake_ROB[exec_Q_OP_0[i]].destn_Reg].ready = READY;
						printf("\nTag and instruction mathces, Making destn reg READY    ----------****");
					}
					
				}
				
				
				fake_ROB[exec_Q_OP_0[i]].wb_Begin_Cycle = count_Cycle;
				

				
				for( j = 0; j<max_S_value; j++)
				{
					if( (sched_Q[j]!=-1) && (fake_ROB[sched_Q[j]].readyState == NOTREADY) )
					{
						//printf("\nEXECUTE(): Sched_Q[j-->%d] instr not READY: %d", j, sched_Q[j]);
						
						if(fake_ROB[sched_Q[j]].intr_state == IS)
						{
							//printf("\nEXECUTE(): Instr: %d, is in IS state", sched_Q[j]);
							
							if( fake_ROB[sched_Q[j]].src_Reg_1 == exec_Q_OP_0[i])
							{
								printf("\nEXECUTE(): Instr src_reg_1: %d ...has dependency on instr: %d", fake_ROB[sched_Q[j]].seq_Number, fake_ROB[exec_Q_OP_0[i]].seq_Number);
								
								fake_ROB[sched_Q[j]].src_Reg_1_state = READY;
							}
							if( fake_ROB[sched_Q[j]].src_Reg_2 == exec_Q_OP_0[i])
							{
								printf("\nEXECUTE(): Instr src_reg_2: %d ...has dependency on instr: %d", fake_ROB[sched_Q[j]].seq_Number, fake_ROB[exec_Q_OP_0[i]].seq_Number);
								
								fake_ROB[sched_Q[j]].src_Reg_2_state = READY;
							}
							
							
							if( (fake_ROB[sched_Q[j]].src_Reg_1_state == READY) && (fake_ROB[sched_Q[j]].src_Reg_2_state == READY) )
							{
								printf("\nEXECUTE(): Instr: %d ...is now FULLY READY-----", fake_ROB[sched_Q[j]].seq_Number);
								fake_ROB[sched_Q[j]].readyState = READY;
							}
							
							
						}
					}
				}

				exec_Q_OP_0[i] = -1;
			}

		}
			
	}
}





void fake_retire()
{
	int  i = 0, position = 0, j = 0;
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
					printf("\n%d ", fake_ROB[i].seq_Number);
					printf("fu{%d} ", fake_ROB[i].operation_t);
					printf("src{%d,%d} ", fake_ROB[i].orig_Src_Reg_1, fake_ROB[i].orig_Src_Reg_2);
					printf("dst{%d} ", fake_ROB[i].destn_Reg);
					printf("IF{%d,%d} ", fake_ROB[i].fetch_Begin_Cycle, (fake_ROB[i].decode_Begin_Cycle - fake_ROB[i].fetch_Begin_Cycle));
					printf("ID{%d,%d} ", fake_ROB[i].decode_Begin_Cycle, (fake_ROB[i].issue_Begin_Cycle - fake_ROB[i].decode_Begin_Cycle));
					printf("IS{%d,%d} ", fake_ROB[i].issue_Begin_Cycle, (fake_ROB[i].execute_Begin_Cycle - fake_ROB[i].issue_Begin_Cycle));
					printf("EX{%d,%d} ", fake_ROB[i].execute_Begin_Cycle, (fake_ROB[i].wb_Begin_Cycle - fake_ROB[i].execute_Begin_Cycle));
					printf("WB{%d,%d} ", fake_ROB[i].wb_Begin_Cycle, (fake_ROB[i].wb_End_Cycle -fake_ROB[i].wb_Begin_Cycle ));
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
