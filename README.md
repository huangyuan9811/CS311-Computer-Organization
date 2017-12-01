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

* Order of stages modified to IF - WB - ID - EX - MEM to prevent data hazard 
* To handle pipeline register update conflict, I created 2 copies of pipeline registers 
```
struct IF_ID_PIPELINE {
  instruction * IF_ID_INST;
  uint32_t IF_ID_NPC;
}

typedef struct CPU_STATE_Struct { 

  ...

  uint32_t IF_ID_PIPELINE past_IF_ID_pipeline;
  uint32_t IF_ID_PIPELINE new_IF_ID_pipeline;

  ...

} CPU_State;
```

* Along with pipeline registers, I added control signals 
```
struct WB_control_signals {
  bool REG_WRITE;
  bool MEM_TO_REG;
}

struct MEM_WB_PIPELINE {
  
  ...

  struct WB_control_signals WB_CONTROL;
}
```
* Pipeline registers and control signals are updated after each cycle

### Project 4 Cache Design 
* Adding data cache to the MIPS simulator
* Built on top of project 3
* Data cache spec
  * Cache size : 64 bytes
  * Block size : 8 bytes
  * 4-way set associative
* 32 bit address divides into
  * 28-bit tag bit
  * 1-bit index bit : used for indexing into the cache (only 2 sets in cache)
  * 3-bit offset bit : MSB is the word bit (2 words within 1 cache block)
* Added cache helper table (similar to supplemental page table in pintos...)
  * Same shape as the `uint32_t ***Cache` but half the size
  * keeps track of valid bit, dirty bit and tag bit to check if the data in the cache is the correct data
* Queues for LRU eviction algorithm 

