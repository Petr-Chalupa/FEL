<!-- omit from toc -->
# FEL

This repository serves as a part of my portfolio. \
Link to my university: [fel.cvut.cz](https://fel.cvut.cz)

_If you found this repo. because you need help with some homework, do not copy, just get an inspiration :) \
[LICENSE](LICENSE.md)_

<!-- omit from toc -->
## Portfolio
- [PRP - C](#prp---c)
- [RPH - Python](#rph---python)
- [PJV - Java](#pjv---java)
- [APO - Assembly \& C](#apo---assembly--c)
- [FUP - Racket \& Haskell](#fup---racket--haskell)
- [PSIA - C](#psia---c)
- [ALG - C](#alg---c)
- [OSY - C](#osy---c)
- [KEO](#keo)

### PRP - C
- Command Line Tools ~ utilities demonstrating C programming concepts:
  - Array and string manipulation (hw01, hw02)
  - File operations and parsing (hw03)
  - Data structures implementation (hw04, hw05)
  - Memory management (hw06)
- Graph Algorithms ~ implementations of fundamental graph operations and algorithms:
  - Graph representation and manipulation (hw07, hw08)
  - File I/O for both text and binary formats (hw09)
  - Memory management with dynamic allocation (hw09)
  - Implementations of various graph algorithms (hw10)
  - _Note: hw10 framework was provided, my contribution was integration of specific functions (pq_update, dijkstra.c, dijkstra_set_graph & dijkstra_get_solution, graph.c)_

### RPH - Python
- Spam Filter ~ a modular email classification system with various spam detection methods:
  - Multiple filter types (Naive, Paranoid, Random and advanced heuristic Filter)
  - Analyzes email headers, domains, subject keywords, text patterns and links
  - Uses confusion matrix for quality assessment
  - Training and testing on email corpus data
- Reversi ~ a player implementation for the Reversi board game:
  - Heuristic-based move selection
  - Position rating matrix
  - Valid move detection with weighted decision making
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
    - `pong.c`, `utils.c`, `IO_utils.c`, `logo.c` and their respective header files in `include/`
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

### OSY - C
- Shell Scripting:
  - File System Path Processor (hw01.sh):
    - Processes paths from stdin (prefixed with "PATH ")
    - Optional .tgz archive creation (-z flag)
  - Text Processing Utilities (hw02.sh):
    - PDF file listing with case-insensitive sorting
    - Number-prefixed line filtering
    - Sentence extraction and formatting
    - C #include directive prefix manipulation
- C programming:
  - Process forking and piping (hw03):
    - Parent-child process communication via pipes
    - Random number generator with signal handling
    - Integration with provided GCD calculator
    - _Note: `nsd_src` (GCD calculator) was provided as part of the assignment_

### KEO
- PCB Project:
  - Board design using KiCad
  - Realization
  - Documentation