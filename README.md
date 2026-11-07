# Tip 

A high performance linear memory allocator written in C++23 

## Why use it ?

As a linear allocator tip does minimal work under the hood to allocate and construct an object

The steps are quite literally :

- Align a pointer 
- Decrease the size of the object from the capacity
- Construct the object

This way we achieve things like (tip Vs ptmalloc (single object allocate and construct)) :

![](media/image.png)

A 23x decrease in time is something substancial if your project needs it 

**Note** : Benchmarks for most APIs are still being written

## Requirements 

- CMake version **3.22 and above**
- Ninja **1.13 and above**
- Unix Makefiles **4.0 and above**
- g++ (or any C++ compiler) supporting **C++23**

## Build 

The projects contains a Makefile as well as the CMakeLists.txt file 

If you want to use make : 

```bash
# Generates the build scripts for Ninja
make generator 
# Build the project 
make build 
# Execute the tests
make tests
# Execute the benchmarks
make benchmarks
```

If you **don't** want to use Unix Makefiles :

```bash
# Generate the build scripts for Ninja
cmake -S . -B build -G Ninja -j$(nproc) -DCMAKE_BUILD_TYPE=Release
# Build the project
cmake --build build -j$(nproc)
# Run the tests (100 times each)
ctest --test-dir build --output-on-failure -j$(nproc) --repeat until-fail:100
# Run the tests once per test
ctest --test-dir build --output-on-failure -j$(nproc) 
# Run the benchmarks
./build/run_benchmarks
```

# Contributing 

See **contributing.md**

# Goals 

A simple to use interface with a highly optimised hot path , allowing for rapid reuse 
of allocated space for any demanding workflow that can benefit from using such allocator

## Acknowledgement

As my first "big" project it is obvious that not every decision or implementation is correct or the "best" . If you see something wrong or inefficient you are more than welcome to submit a pull request . Every bit of help is taken into consideration!
