# CS311 Final Exam prep

## What did I learn 
* Pipeline hazards
* Cache (related to pipeline..)
* Virtual memory 
* Instruction level parallelism
* Shared memory and multi-processing
* SIMD and multi-threading

## Pipeline hazards

## Cache

## VM

## ILP
Why is instruction level parallelism used?
* increase the number of instructions that can be executed at a given time
* 그냥 뭐든지 더 빨리 더 많이 하면 좋으니까...

### Some ways to achieve ILP
* Super pipelining 
Increase the number of pipeline stages -> reduce the clock cycle time
* Multiple issue 
Fetch more than 1 instruction at a given cycle. 
Need to duplicate hardware

### Multiple Issue
* Static multiple issue == VLIW
* Dynamic multiple issue == Super scalar

### Things to watch out and consider for multiple issue datapath

### VLIW
* pros
* cons
    
### Super scalar processor
* 3 different units 
    * Fetch unit
    * Execution unit
    * Commit unit
* pros
* cons

## Parallel processors
이제까지는 single core이었지만 이제는 멀티코어로 넘어왔다.

### SIMD vs MIMD vs SISD
* SIMD
single instruction multiple data - 즉 같은 instruction 하나를 여러개의 데이터에 동시에 수행하는 것이다. 
예를 들면 4 바이트짜리 워드 4개에 각각 8을 더하겠다!라는 걸 수행할 떄 쓰임.
thus perfect for vector operations! and therefore deep learning of course
* MIMD
multiple instruction multiple data - SIMD보다 조금 더 복잡하다고 생각할 수 있는데 여러개의 다른 instruction들을 다른 데이터에 동시에 수행한다.
그렇지만 같은 목적을 가지고 있다.
예를 들어 4 바이트짜리 워드 4개가 있는데 하나에는 더하기, 두번째꺼에는 곱하기 이런식으로 complex equation을 solving할때 쓰일 수 있음.

### Vector vs scalar

### Hardware multithreading 
Related to MIMD???
하나의 processor에 multiple thread를 수행하는 것인데...time sharing 같은 느낌
하나의 thread가 stall되는 경우가 발생할때 다른 thread를 수행시켜주는 식으로 작동한다면 processor가 idle하지 않고 항상 바쁘게 있을 수 있다.
PC register랑 data register를 duplicate 시켜서 context switching 비슷한 거 할때 overhead를 줄일 수 있다.
* Fine-gained multithreading 
* Coarse-grained
* Simultaneous

### Shared memory multiprocessors
Shared memory means all the cores are sharing memory and I/O through some communication network.

### SMP
Multiple cores on a chip but they share memory. 
Although each processor has its own virtual address space, they have the same physical adddress space.
They can access the same memory location through load and store instructions.
* uniform memory access (UMA)
* nonuniform memory access (NUMA)

Must consider synchronization problems.
Lock 같은걸 사용해서 동시에 두개의 프로세서가 같은 데이터를 write 하고 read하지 못하게 해야한다

### Message passing multiprocessors