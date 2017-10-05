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
  struct trace_item LW_SW[4];
 
  struct btb_entry btb_table[64];
  memset(btb_table, 0, (sizeof(struct btb_entry) * 64)); 
  
  size_t size;
  char *trace_file_name;
  
  int trace_view_on = 0, added = 0, no_print = 0;
  int stop = -1,  end = 0, taken = 0, branch_cycle = 0; 
  unsigned int squashed[3];
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
  
  	// Check for lw hazard DEBUG: should these be detected in the ID_EX buffer?
  	/*if( (EX_MEM.type == 3) && ((EX_MEM.dReg == IF_ID.sReg_a) || (EX_MEM.dReg == IF_ID.sReg_b)))
  	{
  		*fetch_entry = IF_ID;
  		
  		IF_ID = ID_EX;
  		
  		ID_EX.type = 0;		
   	}
   	else if( branch_predictor == 0 )
   	{ 
   		if( (EX_MEM.type == 5)) //branch
		{
			//Check PC of Branch with instruction in ID buffer
			unsigned int b_pc, id_pc;
			b_pc = EX_MEM.PC;
			id_pc = ID_EX.PC;
			
			//printf("Check Branch Condition\n");
			
			if((id_pc - b_pc) == 4)
			{
				//Branch not taken
			}
			else
			{
				//branch taken, squash first two buffers
				for(int i = 0 ; i < 3 ; i++)
				{
					if(squashed[i] == 0)
					{
						squashed[i] = EX_MEM.PC;
						break;
					}
				}
			}
		}
		
		size = trace_get_item(&fetch_entry);
   	}
   	else if((branch_predictor == 1))
   	{
   		//Branch Detected, consult BTB
   		if(IF_ID.type == 5)
   		{
   			unsigned int b_pc;
			
			//Bitmask PC with 1111110000
			int index = (IF_ID.Addr & 1008) >> 4;
			//printf("Addr: %x\n", IF_ID.Addr);
			//printf("index: %d\n", index);
			//table entry matches
			if(btb_table[index].entry_Addr == IF_ID.Addr)
			{
				//printf("PC value found for %d\n", IF_ID.Addr);
				//printf("btb prediction: %d\n", btb_table[index].btb_taken);
				taken = btb_table[index].btb_taken;
			}
			else
			{
				//overwrite or set BTB entry
				//printf("Overwrite/Set\n");
				btb_table[index].entry_Addr = IF_ID.Addr;
				btb_table[index].btb_taken = 0; //predict not taken by default
			}
   		}
   	
   		if(EX_MEM.type == 5) //branch
		{
			//printf("BRANCH RESOLUTION:\n");
			//Check PC of Branch with instruction in ID buffer
			unsigned int b_pc, id_pc;
			b_pc = EX_MEM.PC;
			id_pc = ID_EX.PC;
			//printf("Addr: %x\n", EX_MEM.Addr);
			//printf("index: %d\n", (EX_MEM.Addr & 1008) >> 4);
			//Branch not taken
			if((id_pc - b_pc) == 4)
			{
				//prediction wrong, correct table
				if(taken == 1)
				{
					int fix_index = (EX_MEM.Addr & 1008) >> 4;
					btb_table[fix_index].btb_taken = 0;
					btb_table[fix_index].entry_Addr = EX_MEM.Addr; //DEBUG if needed or not
					//branch was taken, squash two loaded instructions

				}
			}
			else //branch taken
			{
				//Prediction wrong
				if(taken == 0)
				{
					int fix_index = (EX_MEM.Addr & 1008) >> 4;
					btb_table[fix_index].btb_taken = 1;
					btb_table[fix_index].entry_Addr = EX_MEM.Addr; //DEBUG if needed or not
					//NOTE: no need to squash because trace is Dynamic
				}
			}
		}
		
		size = trace_get_item(&fetch_entry);
   	}
   	else
   	{
    	size = trace_get_item(&fetch_entry);
   	}*/
   	
   	//DEBUG wrap in else
   	size = trace_get_item(&fetch_1);
   	size = trace_get_item(&fetch_2);   	
   
    if (cycle_number == stop) {       /* no more instructions (trace_items) to simulate */
        printf("+ Simulation terminates at cycle : %u\n", cycle_number);
      	break;
    }
    else{           
		//Scheduler
		*fetch_alu = *fetch_1;
		*fetch_lw_sw = *fetch_2;
		
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
		
		cycle_number++;	
    }  

    if (trace_view_on && !no_print) {/* print the executed instruction if trace_view_on=1 and don't print the first cycle's initial value*/
      switch(alu_entry->type) {
        case ti_NOP:
          printf("ALU: [cycle %d] NOP:\n",cycle_number) ;
          break;
        case ti_RTYPE:
          printf("ALU: [cycle %d] RTYPE:",cycle_number) ;
          printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(dReg: %d) \n", alu_entry->PC, alu_entry->sReg_a, alu_entry->sReg_b, alu_entry->dReg);
          break;
        case ti_ITYPE:
          printf("ALU: [cycle %d] ITYPE:",cycle_number) ;
          printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", alu_entry->PC, alu_entry->sReg_a, alu_entry->dReg, alu_entry->Addr);
          break;
        case ti_LOAD:
          printf("ALU: [cycle %d] LOAD:",cycle_number) ;      
          printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", alu_entry->PC, alu_entry->sReg_a, alu_entry->dReg, alu_entry->Addr);
          break;
        case ti_STORE:
          printf("ALU: [cycle %d] STORE:",cycle_number) ;      
          printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", alu_entry->PC, alu_entry->sReg_a, alu_entry->sReg_b, alu_entry->Addr);
          break;
        case ti_BRANCH:
          printf("ALU: [cycle %d] BRANCH:",cycle_number) ;
          printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", alu_entry->PC, alu_entry->sReg_a, alu_entry->sReg_b, alu_entry->Addr);
          break;
        case ti_JTYPE:
          printf("ALU: [cycle %d] JTYPE:",cycle_number) ;
          printf(" (PC: %x)(addr: %x)\n", alu_entry->PC,alu_entry->Addr);
          break;
        case ti_SPECIAL:
          printf("ALU: [cycle %d] SPECIAL:",cycle_number) ;      	
          break;
        case ti_JRTYPE:
          printf("ALU: [cycle %d] JRTYPE:",cycle_number) ;
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

    }
  }

  trace_uninit();

  exit(0);
}
