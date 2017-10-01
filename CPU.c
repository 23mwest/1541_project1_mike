/**************************************************************/
/* CS/COE 1541				 			
   just compile with gcc -o pipeline pipeline.c			
   and execute using							
   ./pipeline  /afs/cs.pitt.edu/courses/1541/short_traces/sample.tr	0  
***************************************************************/

#include <stdio.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include "CPU.h" 

int main(int argc, char **argv)
{
  struct trace_item *fetch_entry, *tr_entry;
  size_t size;
  char *trace_file_name;
  
  //Buffer Declarations
  struct trace_item IF_ID;
  struct trace_item ID_EX;
  struct trace_item EX_MEM;
  struct trace_item MEM_WB;
  
  int trace_view_on = 0;
  int branch_predictor = 0;
  
  unsigned char t_type = 0;
  unsigned char t_sReg_a= 0;
  unsigned char t_sReg_b= 0;
  unsigned char t_dReg= 0;
  unsigned int t_PC = 0;
  unsigned int t_Addr = 0;

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

  while(1) {
  
  	// Check for lw hazard
  	if( (ID_EX.type == 3) && ((ID_EX.dReg == IF_ID.sReg_a) || (ID_EX.dReg == IF_ID.sReg_b)))
  	{
  		*fetch_entry = IF_ID;
  		//IF_ID.type = 0;
  		
  		//*fetch_entry = IF_ID; //WTF
  		
  		IF_ID.type = 0;
   	}
   	/*else if( (EX_MEM.type == 5) || (EX_MEM.type == 6) || (EX_MEM.type == 8)) //branch/jump/jr
   	{ 
		//Check PC of Branch/Jump/jr
		unsigned int b_pc, 
		= EX_MEM.PC;
		
		
   	}*/
   	else
   	{
   		 size = trace_get_item(&fetch_entry);
   	}
   
    if (!size) {       /* no more instructions (trace_items) to simulate */
      printf("+ Simulation terminates at cycle : %u\n", cycle_number);
      break;
    }
    else{           
    
		struct trace_item temp1, temp2;
		//Copy first two buffers into temps
		temp1 = IF_ID;
		temp2 = ID_EX;
		//Bring new instruction into IF_ID buffer
		IF_ID = *fetch_entry;
		
		//Propagate the old instructions to the next stage
		ID_EX = temp1;
		temp1 = EX_MEM;

		EX_MEM = temp2;
		
    	//*tr_entry = MEM_WB; //WTF
    	temp2 = MEM_WB;
    	
		MEM_WB = temp1;
		
		*tr_entry = temp2;
		
		//Print Current Cycle instructions
		//printf("IF_ID|| type: %d\n", IF_ID.type);
		//printf("ID_EX|| type: %d\n", ID_EX.type);
		//printf("EX_MEM|| type: %d\n", EX_MEM.type);
		//printf("MEM_WB|| type: %d\n", MEM_WB.type);
		
		cycle_number++;
    }  

    if (trace_view_on && (cycle_number != 1)) {/* print the executed instruction if trace_view_on=1 */
      switch(tr_entry->type) {
        case ti_NOP:
          printf("[cycle %d] NOP:",cycle_number) ;
          break;
        case ti_RTYPE:
          printf("[cycle %d] RTYPE:",cycle_number) ;
          printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(dReg: %d) \n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->dReg);
          break;
        case ti_ITYPE:
          printf("[cycle %d] ITYPE:",cycle_number) ;
          printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->dReg, tr_entry->Addr);
          break;
        case ti_LOAD:
          printf("[cycle %d] LOAD:",cycle_number) ;      
          printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->dReg, tr_entry->Addr);
          break;
        case ti_STORE:
          printf("[cycle %d] STORE:",cycle_number) ;      
          printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->Addr);
          break;
        case ti_BRANCH:
          printf("[cycle %d] BRANCH:",cycle_number) ;
          printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->Addr);
          break;
        case ti_JTYPE:
          printf("[cycle %d] JTYPE:",cycle_number) ;
          printf(" (PC: %x)(addr: %x)\n", tr_entry->PC,tr_entry->Addr);
          break;
        case ti_SPECIAL:
          printf("[cycle %d] SPECIAL:",cycle_number) ;      	
          break;
        case ti_JRTYPE:
          printf("[cycle %d] JRTYPE:",cycle_number) ;
          printf(" (PC: %x) (sReg_a: %d)(addr: %x)\n", tr_entry->PC, tr_entry->dReg, tr_entry->Addr);
          break;
      }
    }
  }

  trace_uninit();

  exit(0);
}
