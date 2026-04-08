<!-- omit from toc -->
# FEL
This repository contains selected coursework completed during my studies in
OI at CTU FEL. \
_If you found this repo. because you need help with some homework, do not copy, just get an inspiration :)_

<!-- omit from toc -->
## Theoretical Foundations
- LAG - Linear Algebra
- DMA - Discrete Mathematics
- LGR - Logic and Graph Theory
- MA1, MA2 - Mathematical Analysis I & II
- PST - Probability and Statistics
- NUM - Numerical Methods
- OPT - Optimization
- PKS - Computer and Communication Networks

<!-- omit from toc -->
## Programming & Engineering Courses
- [PRP - C](#prp---c)
- [RPH - Python](#rph---python)
- [PJV - Java](#pjv---java)
- [APO - Assembly \& C](#apo---assembly--c)
- [FUP - Racket \& Haskell](#fup---racket--haskell)
- [PSIA - C](#psia---c)
- [ALG - C](#alg---c)
- [OSY - Shell \& C](#osy---shell--c)
- [KEO - Hardware](#keo---hardware)
- [PDV - C++ \& Java](#pdv---c--java)
- [ZUI - Python](#zui---python)
- [DBS - Java \& SQL](#dbs---java--sql)

### PRP - C
- Command Line Tools ~ utilities demonstrating C programming concepts:
  - Array and string manipulation (hw01, hw02)
  - File operations and parsing (hw03)
  - Data structures implementation (hw04, hw05)
  - Memory management (hw06)
  - Custom grep implementation (hw07)
- Graph Algorithms ~ implementations of fundamental graph operations and algorithms:
  - Queue implemetation (hw08)
  - File I/O for both text and binary formats (hw09)
  - Memory management with dynamic allocation (hw09)
  - Implementations of various graph algorithms (hw10)
  - _Note: hw10 framework was provided, my contribution was integration of specific functions (pq_update, dijkstra_set_graph, dijkstra_get_solution, graph.c)_

### RPH - Python
- Spam Filter ~ a modular email classification system with various spam detection methods:
  - Multiple filter types (Naive, Paranoid, Random and advanced heuristic Filter)
  - Analyzes email headers, domains, subject keywords, text patterns and links
  - Uses confusion matrix for quality assessment
  - Training and testing on email corpus data
- Reversi ~ a player implementation for the Reversi board game:
  - Heuristic-based move selection
  - Position rating matrix
  - Valid move detection with weighted decision-making
  - Complete game rules implementation

### PJV - Java
- Core Java Concepts - Standalone assignments demonstrating Java fundamentals:
  - Calculator (console-based arithmetic operations)
  - CircularArrayQueue (queue implementation with fixed capacity)
  - SequenceStats (statistical calculations on number sequences)
  - BruteForceAttacker (recursive password cracking with a character set)
  - Tree & Node (binary tree with recursive traversal and string representation)
- [2D Game Engine](https://github.com/Petr-Chalupa/JTileEngine) - Semester project implementing a custom 2D game engine in JavaFX

### APO - Assembly & C
- Assembly Assignments (RISC-V):
  - Array manipulation and sorting algorithms in assembly (hw02.S, lab01.S, lab02.S)
  - Serial port I/O echo implementation (lab03.S)
  - Fibonacci sequence generation on a pipelined CPU (hw03.S)
  - 32-bit word to hexadecimal digits conversion and serial port output (hw04.S)
  - _Note: All assembly code is tested in [QtRVSim](https://github.com/cvut/qtrvsim) - a RISC-V simulator_
- C Assignments:
  - Image convolution using a mask (hw01.c)
  - UART calculator for adding unsigned numbers, built without static libraries (hw05.c)
    - _Note: hw05_crt0.S was provided as a minimal replacement for C runtime_
  - Simple LCD program (lab04.c)
- Pong Game (C):
  - Implementation of the classic Pong game for the MZ-APO educational board
  - Features:
    - Real-time graphics rendering on the LCD
    - Paddle and ball movement with collision detection
    - Score tracking and high scores
    - Modular code structure for hardware abstraction
  - _Note: Only the following files are my original work:_
    - _`pong.c`, `utils.c`, `IO_utils.c`, `logo.c` and their respective header files in `include/`_
    - _Other files (e.g., hardware abstraction, font data) were provided as part of the assignment framework._

### FUP - Racket & Haskell
- Racket Assignments:
  - Image Processing ~ ASCII art generation from images (hw01):
    - Image to grayscale matrix conversion
    - Matrix scaling and transformation
    - ASCII character mapping based on pixel intensity
    - Support for custom character sets and block sizes
  - SVG Generator ~ declarative SVG generation (hw02):
    - Dynamic SVG element creation
    - Support for basic shapes (circle, rect, line)
    - Expression evaluation with environments
    - Conditional rendering capabilities
  - Exam assigment - Manhattan distance (exam01.rkt):
    - Assigns symbols to grid cells based on proximity to given points
    - Handles ties and exact matches with special symbols
- Haskell Assignments:
  - λ-Calculus Evaluator (Hw03)
    - Lambda calculus interpreter with normal order reduction
    - Alpha conversion, beta reduction and capture-avoiding substitution
    - Leftmost redex finding for proper evaluation order
    - Expression representation for variables, applications, and abstractions
  - Parser of λ-programs (Hw04)
    - Parser implementation for λ-calculus expressions
    - Support for variable definitions and substitutions
    - _Note: Parser monad implementation was provided_
  - Exam assigment - Minimum spanning tree (Exam02.hs):
    - Minimum spanning tree computation using Jarník's algorithm
    - Custom graph and edge data types
    - Handles undirected graphs by edge reversal

### PSIA - C
- Network Programming ~ UDP File Transfer:
    - CRC32 packet verification
    - MD5 file integrity check
    - Selective repeat and stop-and-wait for flow control
    - Automatic retry on packet loss
    - Configurable window size and packet size
  - Client-Server Architecture
    - Configurable ports and IP addresses
  - File Operations
    - Binary file transfer support
    - Integrity verification
  - _Note: [md5](https://github.com/Zunawe/md5-c) is a publicly available library_

### ALG - C
- Duplicate Number Finder (hw00.c):
    - Training exercise
- Park Designer (hw01.c):
    - Optimizes park layout with specific constraints:
      - Central area must contain minimum number of rocks
      - Maximizes number of forest tiles in total area
    - Uses prefix sum arrays for efficient area calculations
- Agent Placement (hw02.c):
    - Optimizes agent placement in a graph:
      - Type A scores based on occupied neighbors
      - Type B scores based on unoccupied neighbors
    - Uses backtracking with pruning optimizations
- Tree Path Finder (hw03.c):
    - Finds longest valid path in a tri-colored tree:
      - Path cannot contain 3 consecutive same-colored nodes
    - Uses recursive DFS with path validation
    - Queue-based tree construction from level-order input
- Mouse vs Cat Path (hw04.c):
    - Finds shortest "safe" path for a mouse while avoiding the cat:
      - Graph nodes can be "noisy"; visiting noisy nodes alerts the cat
      - Mouse must avoid positions reachable by the cat within constrained jumps
    - Uses multi-state per node storage and BFS with pruning to return (distance, noisy_used) for optimal path
- Binary search tree with lazy deletion (hw05.c):
    - Implements a BST that supports `ins K` and `del K` commands.
    - Uses lazy deletion and periodically compacts the tree when the sum of deleted-node heights exceeds the sum of active-node heights.
    - Compaction preserves pre-order of active nodes
- Turing machine simulator (hw06.c):
  - Decodes a compact machine encoding into transitions and simulates the machine
  - Checks whether the machine is deterministic and explores branching configurations using BFS
- Irrigatio (hw07.c):
  - Selects high-value horizontal/vertical segments using prefix-sums
  - Sorts segments by weight and combines them via DP for optimal total value
- Totem Buyer (hw08.c):
  - Maximizes profit from buying totems between villages and selling them
  - Villages merge when their separating totem is purchased
  - Purchase cost depends on warrior count difference between villages
  - Similar structure to matrix chain multiplication problem
- Triples (exam.c):
  - Finds non-overlapping 3-cell segments (horizontal and vertical) in a colored matrix
  - Scores each segment: 3 points for all same color, 1 point for 2 same colors, 0 points otherwise
  - Uses backtracking to maximize total score by selecting optimal segment combinations
  - Includes pruning optimization with greedy upper bound calculation for performance

### OSY - Shell & C
- Shell Scripting:
  - File System Path Processor (hw01.sh):
    - Processes paths from stdin (prefixed with "PATH ")
    - Optional .tgz archive creation (-z flag)
  - Text Processing Utilities (hw02.sh):
    - PDF file listing with case-insensitive sorting
    - Number-prefixed line filtering
    - Sentence extraction and formatting
    - C #include directive prefix manipulation
- Unix Programming:
  - Process forking and piping (hw03):
    - Parent-child process communication via pipes
    - Random number generator with signal handling
    - Integration with provided GCD calculator
    - _Note: `nsd_src` (GCD calculator) was provided as part of the assignment_
  - Producer-Consumer Threading (hw04):
    - Multithreaded word processing with shared list
    - Mutex-protected stdout and list operations
    - Conditional variables for synchronization
  - Factory Simulation (hw05):
    - Command-driven factory simulator reading commands from stdin
    - Worker threads process product steps at named places
    - Synchronization with mutexes and condition variables
    - Dynamic worker management and resource accounting
  - Hexadecimal Converter (hw06):
    - Reads unsigned integers from stdin and prints them as hexadecimal
    - Implementation using direct syscalls
    - Built with no libc
- Nova Microkernel Assignments:
  - nbrk system call (ec_syscall.cc):
    - Handles dynamic memory with page table operations and allocation/deallocation
  - User-space memory allocator (mem_alloc.c):
    - Custom heap management for user programs
    - Integrates with kernel page management
  - Thread management system calls (ec_syscall.cc):
    - thread_create: Creates new threads with user-provided stack
    - thread_yield: Switches execution context to the next thread in buffer
    - Circular queue scheduler
  - _Note: Nova kernel framework, build system, and infrastructure were provided; my contributions are `ec_syscall.cc` and `mem_alloc.c`._
    - _The kernel framework provides: CPU/MMU abstraction (`cpu.h`, `ptab.h`), physical memory manager (`kalloc.h`), execution context infrastructure (`ec.h`)..._

### KEO - Hardware
- Complete PCB project design and realization
- Documentation: [`Prezentace.pdf`](KEO/Prezentace.pdf), [`Protokol.pdf`](KEO/Protokol.pdf), [`Schema.pdf`](KEO/Schema.pdf), [`DPS.pdf`](KEO/DPS.pdf)
- Design files: [`Zdroj.kicad_sch`](KEO/Zdroj/Zdroj.kicad_sch), [`Zdroj.kicad_pcb`](KEO/Zdroj/Zdroj.kicad_pcb), [`Zdroj.kicad_pro`](KEO/Zdroj/Zdroj.kicad_pro)

### PDV - C++ & Java
- Parallel (C++):
  - Producer-Consumer Threading (hw01):
    - Generic thread pool template with parametrized job and worker types
    - Producer-consumer coordination without busy-waiting
  - Vector Sum (hw02):
    - Parallel array summation using various OpenMP scheduling strategies
    - Comparison of `static` vs. `dynamic` scheduling performance for uneven workloads
    - Analysis of load balancing trade-offs
  - Lock-Free BST (hw03):
    - Thread-safe binary search tree using atomic operations and lock-free synchronization
    - Lock-free insertion and deletion with compare-and-swap operations
  - Parallel Database Query (hw04):
    - Evaluation of universal and existential predicates
    - Early termination using #pragma omp cancel for
  - Parallel Radix Sort (hw05):
    - Recursive MSD radix sort implementation for strings
    - Parallelized using recursive sub-sorting of buckets
- Distributed (Java):

### ZUI - Python
- Blocks World Solver (hw01):
  - A* search algorithm with custom heuristic for solving the blocks world puzzle
  - Heuristic based on correct prefix matching of goal stacks
  - Finds optimal sequence of block moves to reach goal configuration
  - _Note: blockworld.py environment was provided as part of the assignment_

### DBS - Java & SQL


