# CS311 Computer Organization midterm prep

* What is ISA and microarchitecture
  * ISA is a layer between user program and kernel
  * ISA is implemented with microarchitecture on a specific processor

* Computer architecture = ISA + microarchitecutre

* 이 수업이 왜 중요한가...
  * moore's law (used to) show the number of transistors double every 2 years, meaning that therate of improvement and the impact it has is significant in modern day computing
  * 컴퓨터를 빨리, 그리고 유저에게 더 알맞게 돌아가게하자.

## What did I learn so far 
* basic logic
* performance
* ISA (MIPS)
* Data and Control for single cycle and pipelined design

### Basic Logic
* logic gates 
* MUX
  * mux 정말 중요함. 나중에 control path할때도 많이 쓰임.
  * mux selects particular input, control signal does this

* combinational logic vs sequential logic
  * combinational : the output depends on current input only
  * sequential : output depends on the previous input as well. 
    * EX) using data flip flops for Registers and memory 

* latches vs flip-flops

* CMOS
  * type of transistor
    * transistor는 스위치 같은거고 high voltage랑 ground사이의 값으로 존재한다.
  * NMOS + PMOS
    * NMOS는 high voltage 일때 conducting. PMOS는 그반대.
  * implementing an inverter with CMOS is a series connection of PMOS at the top and NMOS below.
  * "NOT"을 CMOS로 만들기 쉽기때문에 NAND랑 NOR를 구현하는게 더쉽다

* transistor값들은 ideal하게 0과 1이 아니라 그 사이값들이다.
  * 실제로 amplifier같은걸 씀 (SRAM에서 bit를 읽을때라던지)

* what are clocked elements?
  * latches
  * flip flops
  * edge_triggered : clock의 rising edge 혹은 falling edge일때만 값을 바꾼다.

* what is cycle time 
  * function of a critical path 
    * critical path : longest path from one storage to the next
  * 그럼 이 cycle time을 줄여보려고 해야겠지
    * level of logic gate를 줄여서 path의 길이를 줄인다
    * 큰 path를 나누자 sub path로

* Memory
  * 메모리는 store data, read data, write data  할 수 있어야한다
  * memory는 clock dependent할꺼같아
  * SRAM, DRAM, disk
  * SRAM : fast because it has more transistors (6) but expensive
  * DRAM : slower since it needs refreshing
    * better density since only 1 transistor and 1 capacitor. 그래서 대용량으로 저렴하게 만들수있지.
    * capacitor는 leaking하니까 refresh 해줘야함

### Performance 
* CPU execution time = cycle time * instruction count * CPI

* instruction count depends on
  * algorithm, programming language, compiler, ISA
* CPI depends on 
  * algorithm(possibly), programming language, compiler, ISA
* cycle time depends on
  * ISA, technology

* Performance 왜 중요한가
  * performance는 항상 improve하는게 목적이다
  * 굳이 performance가 time related 일 필요없다. enery efficiency도 있음.

* Amdahl's law
  * 어떤 프로그램을 돌리느냐에 따라 improvement가 다르다
  * matrix multiplication을 겁나 많이 하는 프로그램에서는 matrix multiplication을 빨리 할수있는 instruction을 만들었는데 이게 다른 instruction들을 총체적으로 다 느리게 한다고 해도 이득일 수도 있지만 그렇지 않다면 오히려 손해겠지.
  * equation ? 
    * execution time after improvement = (time affected by improvement)/improvment + time unaffected
* what are the fallacies?



### ISA
* why MIPS is good?
* RISC vs CISC
* non-leaf vs leaf functions
* 모든 assembly function 다 보고 가자...
* what are the key design principles that we follow?


### Data path and control
* Implementation of the processor (microarchitecture) determines cycle time and CPI!!

* Why does MIPS compute PC+4?
  * to support pipeline. pipeline에서 instruction fetch를 하는데 매 cycle마다 instruction을 가져와야하기때문에 pc를 바로 increase한다. (simulator구현할때 미리해야겠구나 하하)


### Pipeline
* pipeline registers
* what are some things we have to watch out when designing pipelines?
  * data hazard
  * control hazard
  * structural hazard


#### Single cycle vs Pipeline
* single cycle의 CPI는 1
* pipeline의 CPI도 ideally 1
  * 하지만 ideal하지 않을 경우가 많다. 왜?


* data flipflop , latch 한번 더 공부
* 숙제복습



