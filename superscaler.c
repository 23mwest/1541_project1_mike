/**************************************************************/
/* CS/COE 1541				 			
   just compile with gcc -o pipeline pipeline.c			
   and execute using							
   ./pipeline  /afs/cs.pitt.edu/courses/1541/short_traces/sample.tr	0  
***************************************************************/

#include <stdio.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <string.h> //added
#include "CPU.h" 

struct btb_entry {
	unsigned int entry_Addr;
	unsigned int btb_taken;
};

int noop(struct trace_item **inst)
{
	(*inst)->type = 0;
	(*inst)->sReg_a = 0;
	(*inst)->sReg_b = 0;
	(*inst)->dReg = 0;
	(*inst)->PC = 0;
	(*inst)->Addr = 1;
}

int main(int argc, char **argv)
{
  struct trace_item *fetch_alu=malloc(sizeof(struct trace_item)),
   *fetch_lw_sw=malloc(sizeof(struct trace_item)),
   *fetch_1=malloc(sizeof(struct trace_item)),
   *fetch_2=malloc(sizeof(struct trace_item)),   
   *alu_entry=malloc(sizeof(struct trace_item)), 
   *lw_sw_entry=malloc(sizeof(struct trace_item));

  //Buffer Declarations
  struct trace_item ALU[4];
  memset(ALU, 0, (sizeof(struct trace_item) * 4)); 
  struct trace_item LW_SW[4];
  memset(LW_SW, 0, (sizeof(struct trace_item) * 4)); 
  struct trace_item squashed[2];
  
  
  struct btb_entry btb_table[64];
  memset(btb_table, 0, (sizeof(struct btb_entry) * 64)); 
  
  size_t size;
  char *trace_file_name;
  
  int trace_view_on = 0, added = 0, no_print = 0;
  int stop = -1,  end = 0, taken = 0, branch_cycle = 0, wait = 0; 
  int hold[2] = {0};
  int branch_predictor = 0;

  unsigned int cycle_number = 0;

  if (argc == 2) {
    //Assume branch prediction field = 0 and trace = 0
  }
  else if (argc == 4) {
  	branch_predictor = atoi(argv[2]);
  	trace_view_on = atoi(argv[3]);
  }
  else {
 	fprintf(stdout, "\nUSAGE: tv <trace_file> <branch prediction (0/1)> <switch - (0/1)>\n");
    fprintf(stdout, "\n(branch) 0- predict not taken 1- use 1-bit branch detector.\n");
    fprintf(stdout, "\n(switch) to turn on or off individual item view.\n\n");
    exit(0);
  }
    
  trace_file_name = argv[1];

  fprintf(stdout, "\n ** opening file %s\n", trace_file_name);

  trace_fd = fopen(trace_file_name, "rb");

  if (!trace_fd) {
    fprintf(stdout, "\ntrace file %s not opened.\n\n", trace_file_name);
    exit(0);
  }

  trace_init();

//Begin pipeline computation
  while(1) {
   	if(hold[0] != 1) size = trace_get_item(&fetch_1);
	if(hold[1] != 1) size = trace_get_item(&fetch_2);
	
   	if(wait == 0) hold[0] = 0;
   	if(wait == 0) hold[1] = 0;
   	
   	//printf("fetch_1 type : %d\n", fetch_1->type);
   	//printf("fetch_2 type : %d\n", fetch_2->type);  	  	
   
    if (cycle_number == stop) {       /* no more instructions (trace_items) to simulate */
        printf("+ Simulation terminates at cycle : %u\n", cycle_number);
      	break;
    }
    else
    {           
	//Scheduler ----------------------------------------------------------------------
	if(fetch_1->type == 5 || fetch_2->type == 5)
	{
		//Branch detected
		if(fetch_1->type == fetch_2->type)
		{
			//both are branches
			//check for data dependency on ops in the Reg
			if(LW_SW[0].dReg == fetch_1->sReg_a || LW_SW[0].dReg == fetch_1->sReg_b)
			{
				//Data dependance with fetch_1, check fetch 2
				if(LW_SW[0].dReg == fetch_2->sReg_a || LW_SW[0].dReg == fetch_2->sReg_b)
				{
					//data dependance with fetch 2, insert 2 noops
					noop(&fetch_alu);
					noop(&fetch_lw_sw);
					hold[0] = 0;
					hold[1] = 0;
				}
				else
				{
					//data dependance on fetch_1 but not fetch_2
					fetch_alu = fetch_2;
					
					//insert noop into sw/lw pipeline
					noop(&fetch_lw_sw);
					
					hold[0] = 1;
					hold[1] = 0;
				}		
			}
			else{
				//No data dependance on fetch 1
				//schedule fetch 1 and hold fetch 2
				fetch_alu = fetch_1;
				
				//insert noop into sw/lw pipeline
				noop(&fetch_lw_sw);
				//push branch to first position in instruction buffer
				fetch_1 = fetch_2;
				
				hold[0] = 1;
				hold[1] = 0;
			}			

		}
		else if(fetch_1->type == 5)
		{
			//fetch_1 is a branch
			//We don't care about fetch_2, a branch is always executed with a no-op (unless there are data dependancies)
			
			//check for data dependancy on ops in the Reg
			if(LW_SW[0].dReg == fetch_1->sReg_a || LW_SW[0].dReg == fetch_1->sReg_b)
			{
				//data dependancy on fetch_1, two noops //DEBUG technically could check fetch_2 and issue that
					noop(&fetch_alu);
					noop(&fetch_lw_sw);
					//make sure we fetch new instructions
					hold[0] = 0;
					hold[1] = 0;
			}
			else
			{
				//first op is branch
				fetch_alu = fetch_1;
				
				//issue branch with no-op
				noop(&fetch_lw_sw);
				
				//move fetch_2 to position 1
				fetch_1 = fetch_2;
				
				//don't fetch a new fetch_1, fetch new fetch_2
				hold[0] = 1;
				hold[1] = 0;
			}
		}
		else if (fetch_2->type ==5 && (fetch_1->type == 3 || fetch_1->type ==4))
		{
			//first op is lw/sw, second is branch
			
			//check for data dependency of lw/sw on ops in the Reg
			if(LW_SW[0].dReg == fetch_1->sReg_a || LW_SW[0].dReg == fetch_1->sReg_b)
			{
				//data dependance on fetch_1, insert two no-ops
				//fetch_lw_sw = fetch_1;
				
				//no-op alu pipeline
				noop(&fetch_alu);
				noop(&fetch_lw_sw);
				
				//fetch fetch_1 and don't fetch fetch_2
				hold[0] = 0;
				hold[1] = 0;
			}
			else
			{
				//issue 1st instruction and move branch up
				fetch_alu = fetch_1;
				fetch_1 = fetch_2;
					
				//insert noop into sw/lw pipeline
				noop(&fetch_lw_sw);
					
				hold[0] = 1;
				hold[1] = 0;
			}
		}
		else 
		{
			//2nd is branch, first is some other ALU Op
			//check for data dependency between instructions
			if(fetch_1->dReg == fetch_2->sReg_a || fetch_1->dReg == fetch_2->sReg_b)
			{
				//data dependance between operations, only issue fetch_1
				//first, check for load hazard on fetch_1
				if(LW_SW[0].dReg == fetch_1->sReg_a || LW_SW[0].dReg == fetch_1->sReg_b)
				{
					//load hazard, insert 2 noops
					noop(&fetch_alu);
					noop(&fetch_lw_sw);
				
					//fetch fetch_1 and don't fetch fetch_2
					hold[0] = 0;
					hold[1] = 0;
				}
				else
				{
					//issue fetch 1 and move up fetch_2 branch
						fetch_alu = fetch_1;
						fetch_1 = fetch_2;
					
						//insert noop into sw/lw pipeline
						noop(&fetch_lw_sw);
					
						hold[0] = 1;
						hold[1] = 0;
				}			
			}
			else
			{
				//issue both the branch and alu operation
				fetch_alu = fetch_2;
				fetch_lw_sw = fetch_1;
				
				hold[0] = 0;
				hold[1] = 0;				
			}

		}
		
	}	
	else if((fetch_1->type == 3 || fetch_1->type == 4) && (fetch_2->type == 3 || fetch_2->type == 4))
	{
		//Both instructions are either lw/sw
		if(LW_SW[0].dReg == fetch_1->sReg_a || LW_SW[0].dReg == fetch_1->sReg_b)
		{
				//Data dependance with fetch_1, check fetch 2
			if(LW_SW[0].dReg == fetch_2->sReg_a || LW_SW[0].dReg == fetch_2->sReg_b)
			{
				//data dependance with fetch 2, insert 2 noops
				noop(&fetch_alu);
				noop(&fetch_lw_sw);
				hold[0] = 0;
				hold[1] = 0;
			}
			else
			{
				//data dependance on fetch_1, but not fetch_2
				
				//issue fetch_2 if fetch_1 isn't dependant on it
				if(fetch_1->dReg == fetch_2->dReg || fetch_1->sReg_a == fetch_2->dReg || fetch_1->sReg_b == fetch_2->dReg)
				{
					//fetch_1 depends on fetch_2, insert 2 noops
					noop(&fetch_alu);
					noop(&fetch_lw_sw);
				
					//fetch fetch_1 and don't fetch fetch_2
					hold[0] = 0;
					hold[1] = 0;
				}
				else
				{
					//issue fetch 2
					fetch_lw_sw = fetch_2;
					
					//insert noop into sw/lw pipeline
					noop(&fetch_alu);
					
					hold[0] = 1;
					hold[1] = 0;
				}	
			}
		}
		else
		{
			//No data dependance, issue fetch 1 and hold fetch 2
			fetch_lw_sw = fetch_1;
			noop(&fetch_alu);
			
			fetch_1 = fetch_2;
			hold[0] = 1;
			hold[1] = 0;
		}
		//Possible error, wont this if statement only allow entry if they're both sw/lw?	
	
	}
	else if(((fetch_1->type == 3 || fetch_1->type == 4) && (fetch_2->type != 3 && fetch_2->type != 4)) || ((fetch_2->type == 3 || fetch_2->type == 4) && (fetch_1->type != 3 && fetch_1->type != 4)))
	{
		//Both instructions are different but neither is a branch
		//i.e. one will go to ALU and one will go to LW_SW provided no data dependances
		if(fetch_1->type != 3 || fetch_1->type != 4)
		{
			//fetch_1 is the alu op
			fetch_alu = fetch_1;
			fetch_lw_sw = fetch_2;
		}
		else
		{	
			//fetch_2 is the alu op
			fetch_alu = fetch_2;
			fetch_lw_sw = fetch_1;
			//check dependance between fetches
			//if fetch_2 depends on fetch_1, insert no-op into alu and move it to position 1
			if(fetch_2->dReg == fetch_1->dReg || fetch_2->sReg_a == fetch_1->dReg || fetch_2->sReg_b == fetch_1->dReg)
			{
				//fetch_1 is dependent on the lw_sw instruction
				fetch_lw_sw = fetch_1;
				noop(&fetch_alu);
				
				fetch_1 = fetch_2;
				hold[0] = 1;
				hold[1] = 0;
			}
		}
	
	}
	else //---------------------------
	{
		//Both Instructions require the ALU but neither are branches
		
		//issue fetch_1 and move up fetch_2
		fetch_alu = fetch_1;
		noop(&fetch_lw_sw);
		
		fetch_1 = fetch_2;
		hold[0] = 1;
		hold[1] = 0;
	}
	
	//Branch Prediction Cases
	if( branch_predictor == 0 )
   	{ 
   		if( (ALU[1].type == 5)) //branch
		{
			//Check PC of Branch with instruction in ID buffer
			unsigned int b_pc, id_pc;
			b_pc = ALU[1].PC;
			id_pc = ALU[0].PC;
			
			//printf("Check Branch Condition\n");
			
			if((id_pc - b_pc) == 4)
			{
				//Branch not taken
			}
			else
			{
				//branch taken, squash instructions in REG stages
				squashed[0] = *fetch_alu;
				squashed[1] = *fetch_lw_sw;
				
				*fetch_alu = ALU[0];
				*fetch_lw_sw = LW_SW[0];
				 
				ALU[0].type = 0;
				ALU[0].sReg_a = 0;
				ALU[0].sReg_b = 0;
				ALU[0].dReg = 0;
				ALU[0].PC = 0;
				ALU[0].Addr = 1;
				LW_SW[0].type = 0;
				LW_SW[0].sReg_a = 0;
				LW_SW[0].sReg_b = 0;
				LW_SW[0].dReg = 0;
				LW_SW[0].PC = 0;
				LW_SW[0].Addr = 1;				
				
				hold[0] = 1;
				hold[1] = 1;
				wait = 1;
			}
			if(wait == 1) wait--;
			
		}
		
   	}
   	if((branch_predictor == 1))
   	{
   		//Branch Detected, consult BTB
   		if(fetch_alu->type == 5)
   		{
   			//printf("BRANCH DETECTED\n");
   			unsigned int b_pc;
			
			//Bitmask PC with 1111110000
			int index = (fetch_alu->Addr & 1008) >> 4;
			
			//table entry matches
			if(btb_table[index].entry_Addr == ALU[0].Addr)
			{
				taken = btb_table[index].btb_taken;
			}
			else
			{
				//overwrite or set BTB entry
				btb_table[index].entry_Addr = fetch_alu->Addr;
				btb_table[index].btb_taken = 0; //predict not taken by default
			}
   		}
   		
   		if(ALU[1].type == 5) //branch
		{
			//printf("BRANCH RESOLUTION:\n");
			//Check PC of Branch with instruction in ID buffer
			unsigned int b_pc, id_pc;
			b_pc = ALU[1].PC;
			id_pc = ALU[0].PC;
			//Branch not taken
			if((id_pc - b_pc) == 4)
			{
				//prediction wrong, correct table
				if(taken == 1)
				{
					int fix_index = (ALU[1].Addr & 1008) >> 4;
					btb_table[fix_index].btb_taken = 0;
					btb_table[fix_index].entry_Addr = ALU[1].Addr;
					//branch was taken, squash two loaded instructions
					squashed[0] = *fetch_alu;
					squashed[1] = *fetch_lw_sw;
				
					*fetch_alu = ALU[0];
					*fetch_lw_sw = LW_SW[0];
					 
					ALU[0].type = 0;
					ALU[0].sReg_a = 0;
					ALU[0].sReg_b = 0;
					ALU[0].dReg = 0;
					ALU[0].PC = 0;
					ALU[0].Addr = 1;
					LW_SW[0].type = 0;
					LW_SW[0].sReg_a = 0;
					LW_SW[0].sReg_b = 0;
					LW_SW[0].dReg = 0;
					LW_SW[0].PC = 0;
					LW_SW[0].Addr = 1;				
				
					hold[0] = 1;
					hold[1] = 1;
					wait = 1;
				}
			}
			else //branch taken
			{
				//Prediction wrong
				if(taken == 0)
				{
					int fix_index = (ALU[1].Addr & 1008) >> 4;
					btb_table[fix_index].btb_taken = 1;
					btb_table[fix_index].entry_Addr = ALU[1].Addr; //DEBUG if needed or not
					//NOTE: no need to squash because trace is Dynamic
				}
			}
			if(wait == 1) wait--;
			
		}
   	}
		
		//Superscaler Pipelines REG[0] => EX[1] => MEM[2] => WB[3]
		*alu_entry = ALU[3];
		*lw_sw_entry = LW_SW[3];
		
		ALU[3] = ALU[2];
		LW_SW[3] = LW_SW[2];		
		ALU[2] = ALU[1];
		LW_SW[2] = LW_SW[1];
		ALU[1] = ALU[0];
		LW_SW[1] = LW_SW[0];
		
		//Issue instructions
		ALU[0] = *fetch_alu;
		LW_SW[0] = *fetch_lw_sw;
		
		if(!size && end == 0)
		{
			end = 1;
			stop = cycle_number + 4;
		}
		
		//Print Current Cycle instructions
		/*printf("---------Cycle:  %d---------\n", cycle_number);
		printf("ALU REG|| type: %d\n", ALU[0].type);
		printf("ALU_EX|| type: %d\n", ALU[1].type);
		printf("ALU_MEM|| type: %d\n", ALU[2].type);
		printf("ALU_WB|| type: %d\n", ALU[3].type);
		printf("---------------------------\n\n");
		
		printf("---------Cycle:  %d---------\n", cycle_number);
		printf("LWSW REG|| type: %d\n", LW_SW[0].type);
		printf("LWSW_EX|| type: %d\n", LW_SW[1].type);
		printf("LWSW_MEM|| type: %d\n", LW_SW[2].type);
		printf("LWSW_WB|| type: %d\n", LW_SW[3].type);
		printf("---------------------------\n\n");*/
		
		cycle_number++;	
    }  

    if (trace_view_on && !no_print) {/* print the executed instruction if trace_view_on=1 and don't print the first cycle's initial value*/
    //printf("------------------------------------------\n");
      switch(alu_entry->type) {
        case ti_NOP:
          printf("ALU:   [cycle %d] NOP:\n",cycle_number) ;
          break;
        case ti_RTYPE:
          printf("ALU:   [cycle %d] RTYPE:",cycle_number) ;
          printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(dReg: %d) \n", alu_entry->PC, alu_entry->sReg_a, alu_entry->sReg_b, alu_entry->dReg);
          break;
        case ti_ITYPE:
          printf("ALU:   [cycle %d] ITYPE:",cycle_number) ;
          printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", alu_entry->PC, alu_entry->sReg_a, alu_entry->dReg, alu_entry->Addr);
          break;
        case ti_LOAD:
          printf("ALU:   [cycle %d] LOAD:",cycle_number) ;      
          printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", alu_entry->PC, alu_entry->sReg_a, alu_entry->dReg, alu_entry->Addr);
          break;
        case ti_STORE:
          printf("ALU:   [cycle %d] STORE:",cycle_number) ;      
          printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", alu_entry->PC, alu_entry->sReg_a, alu_entry->sReg_b, alu_entry->Addr);
          break;
        case ti_BRANCH:
          printf("ALU:   [cycle %d] BRANCH:",cycle_number) ;
          printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", alu_entry->PC, alu_entry->sReg_a, alu_entry->sReg_b, alu_entry->Addr);
          break;
        case ti_JTYPE:
          printf("ALU:   [cycle %d] JTYPE:",cycle_number) ;
          printf(" (PC: %x)(addr: %x)\n", alu_entry->PC,alu_entry->Addr);
          break;
        case ti_SPECIAL:
          printf("ALU:   [cycle %d] SPECIAL:",cycle_number) ;      	
          break;
        case ti_JRTYPE:
          printf("ALU:   [cycle %d] JRTYPE:",cycle_number) ;
          printf(" (PC: %x) (sReg_a: %d)(addr: %x)\n", alu_entry->PC, alu_entry->dReg, alu_entry->Addr);
          break;
      }
      switch(lw_sw_entry->type) {
        case ti_NOP:
          printf("LW/SW: [cycle %d] NOP:\n",cycle_number) ;
          break;
        case ti_RTYPE:
          printf("LW/SW: [cycle %d] RTYPE:",cycle_number) ;
          printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(dReg: %d) \n", lw_sw_entry->PC, lw_sw_entry->sReg_a, lw_sw_entry->sReg_b, lw_sw_entry->dReg);
          break;
        case ti_ITYPE:
          printf("LW/SW: [cycle %d] ITYPE:",cycle_number) ;
          printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", lw_sw_entry->PC, lw_sw_entry->sReg_a, lw_sw_entry->dReg, lw_sw_entry->Addr);
          break;
        case ti_LOAD:
          printf("LW/SW: [cycle %d] LOAD:",cycle_number) ;      
          printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", lw_sw_entry->PC, lw_sw_entry->sReg_a, lw_sw_entry->dReg, lw_sw_entry->Addr);
          break;
        case ti_STORE:
          printf("LW/SW: [cycle %d] STORE:",cycle_number) ;      
          printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", lw_sw_entry->PC, lw_sw_entry->sReg_a, lw_sw_entry->sReg_b, lw_sw_entry->Addr);
          break;
        case ti_BRANCH:
          printf("LW/SW: [cycle %d] BRANCH:",cycle_number) ;
          printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", lw_sw_entry->PC, lw_sw_entry->sReg_a, lw_sw_entry->sReg_b, lw_sw_entry->Addr);
          break;
        case ti_JTYPE:
          printf("LW/SW: [cycle %d] JTYPE:",cycle_number) ;
          printf(" (PC: %x)(addr: %x)\n", lw_sw_entry->PC,lw_sw_entry->Addr);
          break;
        case ti_SPECIAL:
          printf("LW/SW: [cycle %d] SPECIAL:",cycle_number) ;      	
          break;
        case ti_JRTYPE:
          printf("LW/SW: [cycle %d] JRTYPE:",cycle_number) ;
          printf(" (PC: %x) (sReg_a: %d)(addr: %x)\n", lw_sw_entry->PC, lw_sw_entry->dReg, lw_sw_entry->Addr);
          break;
      }
    printf("------------------------------------------\n");
    }
  }

  trace_uninit();

  exit(0);
}
