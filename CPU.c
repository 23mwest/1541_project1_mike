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
  struct trace_item *tr_entry;
  size_t size;
  char *trace_file_name;
  
  //Buffer Declarations
  struct trace_item IF_ID;
  struct trace_item ID_EX;
  struct trace_item EX_MEM;
  struct trace_item MEM_WB;
  
  int trace_view_on = 0;
  
  unsigned char t_type = 0;
  unsigned char t_sReg_a= 0;
  unsigned char t_sReg_b= 0;
  unsigned char t_dReg= 0;
  unsigned int t_PC = 0;
  unsigned int t_Addr = 0;

  unsigned int cycle_number = 0;

  if (argc == 1) {
    fprintf(stdout, "\nUSAGE: tv <trace_file> <switch - any character>\n");
    fprintf(stdout, "\n(switch) to turn on or off individual item view.\n\n");
    exit(0);
  }
    
  trace_file_name = argv[1];
  if (argc == 3) trace_view_on = atoi(argv[2]) ; //doesn't check for number of arg
  

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
  		tr_entry = &IF_ID;
  		IF_ID.type = 0;
  		
  		/*tr_entry->type = IF_ID.type;
  		tr_entry->sReg_a = IF_ID.sReg_a;
  		tr_entry->sReg_b = IF_ID.sReg_b;
  		tr_entry->dReg = IF_ID.dReg;
  		tr_entry->PC = IF_ID.PC;
  		tr_entry->Addr = IF_ID.Addr;
  		
  		IF_ID.type = 0;*/
   	}
   	else
   	{
   		 size = trace_get_item(&tr_entry);
   	}
   
    if (!size) {       /* no more instructions (trace_items) to simulate */
      printf("+ Simulation terminates at cycle : %u\n", cycle_number);
      break;
    }
    else{           
      	cycle_number++;
    
		struct trace_item temp1, temp2;
		//Copy first two buffers into temps
		temp1 = IF_ID;
		temp2 = ID_EX;
		//Bring new instruction into IF_ID buffer
		IF_ID = *tr_entry;
		//Propagate the old instructions to the next stage
		ID_EX = temp1;
		temp1 = EX_MEM;	
		EX_MEM = temp2;
		//Output the completed instruction
		t_type = MEM_WB.type;
    	t_sReg_a = MEM_WB.sReg_a;
    	t_sReg_b = MEM_WB.sReg_b;
    	t_dReg = MEM_WB.dReg;
    	t_PC = MEM_WB.PC;
    	t_Addr = MEM_WB.Addr;
    
		MEM_WB = temp1;
		
		
    }  
	

// SIMULATION OF A SINGLE CYCLE cpu IS TRIVIAL - EACH INSTRUCTION IS EXECUTED
// IN ONE CYCLE

    if (trace_view_on) {/* print the executed instruction if trace_view_on=1 */
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


