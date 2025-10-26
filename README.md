# Computer Architecture

## Performance-Oriented Rendering Project

### Overview

This project was developed as part of the **Computer Architecture course** at **Universidad Carlos III de Madrid (UC3M)**.  
Its objective is to explore **performance optimization in sequential C++ programs**, focusing on data layout efficiency and software design clarity.

The assignment consisted of implementing a **3D image synthesis (ray tracing)** application capable of generating **PPM images** from scene and configuration files.  
The project emphasizes **performance, correctness**, and **software engineering practices** in **modern C++23**.

### Objectives

- Implement a sequential ray tracer (no parallelism or multithreading)  
- Compare two data layout strategies:  
  - SOA (Structure of Arrays)  
  - AOS (Array of Structures)  
- Develop both implementations:  
  - `render-soa`  
  - `render-aos`  
- Ensure high code quality and test coverage  
- Evaluate performance and energy efficiency  

### Technical Details

- **Language:** C++23  
- **Build system:** CMake + Ninja  
- **Compiler:** GCC 14  
- **External libraries:**  
  - GSL (Guideline Support Library)  
  - GoogleTest  
- **Output format:** PPM (P3, plain text)  
- **Scene elements:** spheres and cylinders  
- **Material types:** matte, metal, refractive  

The renderer parses text-based scene and configuration files to produce deterministic RGB images.  
All components were implemented from scratch, following strict code-quality and style requirements.

### Repository Structure

common/ # Shared logic and utilities
soa/ # SOA implementation (Structure of Arrays)
aos/ # AOS implementation (Array of Structures)
utcommon/ # Unit tests for common components
utsoa/ # Unit tests for SOA
utaos/ # Unit tests for AOS
cmake/ # Build utilities
.devcontainer/ # Development environment setup

### Evaluation

The project was assessed on:
- Performance (runtime and energy efficiency)
- Code quality and documentation
- Design clarity
- Unit and functional testing
- Individual contributions within the development team

### Academic Context

This repository represents academic work completed for the **Computer Architecture** course at **Universidad Carlos III de Madrid**.  
It demonstrates applied knowledge in **software optimization**, **C++23 programming**, and **performance-aware system design**.




