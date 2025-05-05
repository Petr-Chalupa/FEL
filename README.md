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
- [AIAA - Python](#aiaa---python)

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
- _**(TO BE IMPLEMENTED)**_

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
- Haskell Assignments:
  - λ-Calculus Evaluator (Hw03)
    - Lambda calculus interpreter with normal order reduction
    - Alpha conversion, beta reduction and capture-avoiding substitution
    - Leftmost redex finding for proper evaluation order
    - Expression representation for variables, applications, and abstractions
  - Parser of λ-programs (Hw04)
    - _**(TO BE IMPLEMENTED)**_

### PSIA - C
- Network Programming ~ UDP File Transfer:
    - CRC32 packet verification
    - MD5 file integrity check
    - Selective repeat and stop-and-wait for flow control
    - Automatic retry on packet loss
    - Configurable window size and packet size
  - Client-Server Architecture
    - Support for multiple clients
    - Configurable ports and IP addresses
  - File Operations
    - Binary file transfer support
    - Integrity verification
  - _Note: [md5](https://github.com/Zunawe/md5-c) is a publicly available library_

### AIAA - Python
- _**(TO BE IMPLEMENTED)**_
- _...This is from a different course outisde of my university..._

