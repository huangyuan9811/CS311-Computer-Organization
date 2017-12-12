# CS311 Final Exam prep

## What did I learn 
* Pipeline hazards
* Cache (related to pipeline..)
* Virtual memory 
* Instruction level parallelism
* Shared memory and multi-processing
* SIMD and multi-threading

## Pipeline hazards
* Structural hazard 
    * In our simple case, it is solved by having separate instruction and data cache
* Data hazard
데이터가 아직 준비가 안되어있는데 다음 instruction에서 register를 일찍 읽어버리면 잘못된 값을 읽으니까 hazard!
    * Read before write hazard
```angular2html
add $3, $2, $1
sub $4, $3, $5
```
   * Load-use hazard
```angular2html
lw  $1, 0($2)
add $3, $1, $4
```
   * Mem-to-mem hazard
```angular2html
lw $1, 0($2)
sw $1, 4($2)
```
* Control hazard
    * Branch prediction
        * Static branch prediction vs dynamic branch prediction
        * Dynamic branch prediction requires things like branch history table 
            * Can use 1 bit or 2-bit predictor (2-bit은 틀려도 한번 봐준다는 개념)
        * Also using branch delay slot helps
    * Jumps
        * Jump requires 1 stall always
    * Exceptions
        * Need to re-execute the instruction that caused this exception, so we need to save this problematic PC

* In essence, all these hazards can be solved through stalling...But there must be a better way of course.

### Data Forwarding
이미 계산은 다 되어있고 준비는 되었는데 WB stage에 아직 도달안해서 register에 적히지 않은 값들을 필요로 하는 instruction들에게 미리 보내주는 것. 아이 똑똒해
* 특히나 ALU의 source register들의 input값들이 주로 data hazard가 많이 걸린다. 그래서 여기에 MUX를 추가해서 어떤 값을 쓸지 정하면 된다. Forwarded data vs from some registers
* Check if the forwarding criterion is met in the EX stage 
* Without data forwarding, 2 stalls are needed in between 2 consecutive and dependent instructions
* 1 stall is still needed for load-use hazard despite the data forwarding

### Branch prediction
* Static branch prediction
    * Branch taken
        * good if the branch checking is at the end of the loop
    * Branch not taken (PR3 구현)
        * at the beginning of the loop
* Dynamic branch prediction
    * keep track of the history and predict the future
    * 1 bit predictor is ok but probably not that useful when we have lots of resources on our hands to implement more sophisticated predictor
    * 2 bit predictor is like a state machine
* Placing the branch decision unit in the ID stage will reduce the number of stall cycles
    * 굳이 MEM stage에서 하지말고 일찍 해버리자
    * ID stage에서 EX에서 ALU를 가져와버려서 branch target address를 계산해버리고
    * 그럼 branch 해야하는지 안해야하는지는? 이거는 register 2개를 읽는거라 data hazard가 있을 수 있어서 additional stalling이 필요할 수 도 있다.
* Delayed branch 
    * Implemented in MIPS architecture
    * 위에서 ID stage에서 branch decision을 해결한다고 해도 1 stall이 필요한데 그때 낭비하지말고 branch와 관련없는 instruction을 가져와서 그냥 실행시켜버리는 건 어떤가 == delay slot
    * 컴파일러가 현명하게 알맞는 무관한 instruction을 가져다와서 해주는 듯
    * not programmers job in MIPS at least

## Cache
Why do we need cache? To reduce the memory access.
* Cache write
    * Write hit 
        * Write back vs write through 
            * Whenever cache write happens, do we update the memory or not?
            * Write back is normally better, and it makes a difference when it is a cache hit 
    * Write miss
        * Stalling the pipeline (PR4에서 구현)
            * Find a free space in the cache (if none, evict a block), bring in the data, update the data and resume pipeline
        * Not stalling 
            * Write allocate vs no write allocate
                * 그냥 바로 캐시에 데이터 적어버린다 (자리없음 eviction하라는건가) vs 캐시는 패스하고 메모리에 업데이트해준다.
* Cache read
    * Read hit
    * Read miss 
        * Stall the pipeline (PR4에서 구현)
* Calculating offset bits, index bits and tag bits
    * Offset bit == 2 bits : 1 word는 4 byte인 경우 기본적으로 2 bit는 offset이다
    * Index bit (= number of cache lines)
        * direct mapped일 경우 : cache_size / cache_block_size
        * n-way associative : cache_size / (cache_block_size * n)
        * fully associative : 0 bit
    * Tag bit : 32 bit - Index bit - Offset bit
    * Valid bit과 dirty bit이 필요할 경우가 많기 때문에 주로 (2 + (32-index-offset))bits가 tag+alpha bit로 cache block하나당 필요하다.
        

### How to reduce miss rate
1. Increase associativity
    * Direct mapped vs set-associative vs fully associative
        * Direct mapped인 경우에는 cache line 하나에 cache block하나만 들어갈 수 있어서 만약 index가 같은 여러개의 address의 데이터 접근이 많다면 eviction이 매우 자주 일어난다
        * Set-associative는 more than 1 cache block can be placed in one cache line (ex. 4-way면 up to 4 cache blocks with the same index bit can be placed in the cache at the same time)
        * Fully associative는 index bit없이 모든 cache block들이 한 줄에 다 들어간다
     
2. Multi-level cache
    * L1 and L2 cache
    * L1 cache focuses on reducing the hit time and L2 cache focuses on reducing the miss rate 
    * So, L1 cache tends to be small while L2 cache tend to have high associativity
    * Helps to decrease the AMAT
    
### How to improve cache performance
* Cache performance measured by AMAT
    * AMAT = Hit time + miss rate * miss penalty
* And also counting the CPI_stall 
    * CPI_stall = CPI_ideal + miss rate * miss penalty
1. Reduce the miss rate
    * Larger cache
    * Increase associativity
2. Reduce hit time
    * Smaller cache
    * Direct mapped
    * Smaller blocks
3. Reduce miss penalty
    * Smaller blocks
    * Use multiple level cache
    * Write buffer - don't have to wait for write to finish before reading

## Virtual Memory
* Virtual memory는 각 프로세스가 디스크 크기만큼의 메모리를 가지고 있는 것처럼 보여준다. 
* Main memory를 캐시처럼 사용 ->실제로 메모리를 접근할때에는 page address translation이 필요히다. (virtual to physical)

If we look at the pipeline, we added cache for reducing the number of access to main memory.

But if we want to access main memory, we need physical address, which means we need to perform address translation.

However, in order to do address translation, we need to access the page table, which is in the main memory...

This means that every time we try to access memory, we have to at least access main memory once for address translation..너무 노답이잖아.
So we add TLB!

### Translation Lookaside Buffer
* A cache for page table.
* Mostly very small(16~512 entries), since the hit latency is important. (작아야 access time이 줄어든다)
* TLB misses do not mean page fault. It could be a TLB miss, but the page may still be in the main memory.
    * 예를 들어 좀 오랫동안 access되지 않았던 페이지라면 아직 메인 메모리에서는 evict되지 않고 있지만 TLB에 translation 엔트리가 없을 수도 있다. 
* Ideal case : TLB hit (no need to access the page table) && Page in main memory (no page fault, no need to access the disk) && Data in cache (no need to access the main memory)
    * Ideal case에서는 매인 메모리조차 접근안하고 해결됌!

#### Then, can we improve the cache access time?
* 애초에 cache를 physical address로 접근하지 않고 virtual address space를 사용해서 만들 수 있지 않을까? (to reduce the address translation time)
    * coherency problem occur
        * cache can be shared by multiple processes, so it is not possible to distinguish from which process the cached data is for (may need to put pid tags)
* Make the TLB access and cache access parallel (VA->PA address translation을 끝내기 전에 cache access를 시작한다)
    * How is this possible when cache is accessed with physical address?
    * Note that between physical and virtual address, lower 12 bits (page offset if page is 4KB) is the same. 
    * Luckily, index bit for cache is the lower bits of the physical address.
    * Since lowest 2 bits are word offset, we can use up to 10 bits (12 - 2) for the index bit 

### Pros and cons of increasing page size (ex. 4KB to 64KB)
* Pros
    * Page table size will decrease (1 entry used to cover 4KB but now will cover 64KB)
    * can exploit spatial locality (more data brought at once)
* Cons
    * Higher miss penalty (more data has to be transferred)
    * Higher internal fragmentation (many data may not be used)
    * Higher competition for cache space (less number of pages will be in the cache)
* 결론 무식하게 페이지 사이즈 막 늘리고 그러지말자

## Instruction Level Parallelism
Why is ILP used?
* increase the number of instructions that can be executed at a given time
* 그냥 뭐든지 더 빨리 더 많이 하면 좋으니까...

### Some ways to achieve ILP
* Super pipelining 
    * Increase the number of pipeline stages -> reduce the clock cycle time
* Multiple issue 
    * Fetch more than 1 instruction at the same time.
    * Need to duplicate hardware. (ex. multiple read/write ports)

### 2 types of Multiple Issue processor
* Static multiple issue == VLIW
    * Let the compiler do the job
* Dynamic multiple issue == Super scalar
    * Let CPU do the job

### Things to watch out and consider for multiple issue datapath
* How many instruction should we fetch at the same time?
* Which instructions should we fetch together?
* Need to think about dependency and hazards carefully

### VLIW (Very Long Instruction Word, literally)
진짜로 1 instruction을 가져오는게 아니라 여러개를 붙여서 가져온다. (Normally an ALU operating instruction and Mem access instruction)
* Compiler at compile time will look at the dependencies of the instructions and create an issue packet
    * Make sure we don't have 2 mem access instructions at the same time to avoid structural hazard
* Need to duplicate hardware 
* If number of independent instructions is too small, then there is very little performance improvement 
* To solve above problem, loop unrolling is commonly used

* pros
    * Hardware can be kept simple (less power consumption)
    * scalable
* cons
    * Compile time is long and compiling and complex
    * Object code is incompatible 
    * Code bloat (loop unrolling will be very bloated)
    * Large memory bandwidth
    * When hazard occurs, we need to stall all future instructions

그래서 결론은 다이나믹으로...

### Super scalar processor
* Dynamically at run time, decide how many and which instructions to execute at the same time
* 3 different units 
    * Instruction fetch unit
    * Execution unit
    * Commit unit
* Instruction fetching and committing MUST be done in-order
    * In the commit unit, there is a queue where instructions that are ready to be committed are in line waiting for the confirmation
    * Speculative instructions will not be committed until it is confirmed that the speculation is correct
* Execution can be done out-of-order and this will increase IPC
    * When performing out-of-order execution, dependencies have to be checked
    * Read after write : true dependency 우리가 항상 자주 마주치던...
    * Write after write : ouput dependency
   
    이건 같은 레지스터를 변경하는건데 이때 순서가 바뀌면 나중에 $1을 쓰는 애들이 이상한 값을 받으니까 그러면 안된다
```angular2html
lw  $1, 0($2)
add $1, $3, $4
...
sub $5, $1, $3
```
   * Write after read : anti-dependency
   
   먼저 $2의 값을 읽는 instruction이 있는데 그 후에 $2의 값을 변경하는 instruction이 있다. 여기서 순서가 바뀌어버리면 add할때 $2의 값이 다른 값이 되어버리니까 이것도 안된다.
```angular2html
add $3, $2, $1
sub $2, $4, $5
```
   * WAW랑 WAR은 storage conflict and can be solved with register renaming
   
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


### SMP (Shared memory multi-processor)
Multiple cores on a chip but they share memory. 
Although each processor has its own virtual address space, they have the same physical adddress space.
They can access the same memory location through load and store instructions.
* uniform memory access (UMA)
* nonuniform memory access (NUMA)

Must consider cache coherence and synchronization problems.

#### Cache coherence
Coherent하다는건 어떤 프로세서가 data write을 진행하더라도 이 address의 데이터를 원하는 다른 모든 프로세서는 앞으로 업데이트된 데이터를 받을 수 있어야 한다.
Serial하게 1차원적으로 read and write sequence가 만들어져야 한다고 생각하면 될듯.
그럼 write한 것을 다른 캐시 플러스 메인 메모리에게 propagate해야겠지
Must perform write propagation and serialization

* write back vs write through
write through는 주로 inefficient하다. Write할때마다 데이터가 전달되어야해서 high bandwidth를 요구하기 때문. 하지만 생각하기에도 구현하기에도 쉽지..
write back은 cache hit일때는 cache access만으로 끝나기 때문에 efficient한 반면 구현하려면 조금 더 복잡하다.
write through -> update based protocol
write back -> invalidation based protocol

##### Invalidation based protocol (write-back)
프로세서 하나가 write을 하게 되면 이 프로세서의 캐시에 들어있는 데이터는 exclusive state고 이 같은 address의 데이터를 가지고 있는 다른 모든 캐시들에게 메세지를 보내서
그 데이터들은 invalidation state가 된다. 
이제 cache controller는 프로세서 + snooping signal을 양방향에서 받으면서 캐시를 관리해야한다.
* MSI protocol (Modified, shared, invalidated)
Depending on the processor-initiated and bus-initiated transactions, the states of caches change between 3 stages.
그림 그려보면 이해됌ㅋㅋ
* MESI protocol (메시!)
Added exclusive state to MSI -> valid && clean
BusRd를 통해서 snooping을 하면 다른 캐시들이 copy를 가지고 있는지 아니면 안가지고 있는지 확인하고 아무도 안가지고 있으면 exclusive state로 가면 된다
Exclusive에서 PRWr로 인해 modified로 변환할때는 bus transaction이 필요없다. 혼자만 가지고 있으니까!

#### Locks
Lock 같은걸 사용해서 동시에 두개의 프로세서가 같은 데이터를 write 하고 read하지 못하게 해야한다
* spin locks

### Message passing multiprocessors


## Concepts
* Multi-core/Multi-processor
* Multi-threading
* SIMD/MIMD/SISD
* VLIW
* Vector
* Superscalar
* Shared memory
* Instruction level parallelism
* Job level parallelism
* Multiple issue

볼때마다 새롭구나..