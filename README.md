# CS311
Computer Organization Fall2017

### Project 1 MIPS Assembler
* Converting the MIPS ISA to binary instruction

### Project 2 MIPS Simulator
* Single cycle simulator 
* Loads the binary and executes the instructions.
* Updates registers and memory.

### Project 3 MIPS pipeline simulator
* 5 stage pipeline simulator 
* IF - ID - EX - MEM - WB
* Hazard handlings are implemented
  * Data forwarding supported for MEM/WB-to-EX, EX/MEM-to-EX
    * Data hazard still occurs for instructions following lw
  * Static branch prediction (always predicts branch not taken)
    * Branch misprediction caused 3 stalls and flushing
* Jumps cause 1 cycle stall

### Project 4 Cache Design 
* Adding data cache to the MIPS simulator
* Built on top of project 3
