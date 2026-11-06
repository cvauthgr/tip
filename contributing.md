# Contributing to tip

`tip` is a C++23 policy-based bump allocator built for high-performance and low-latency execution environments. The primary design constraints of this project are **instruction cache locality, branchless hot paths, and cycle-accurate memory access.**

`tip` is not completed yet . And as it my first big scale project some decisions that I made could be wrong or 
different implementations might not be correct . Please respect that!

I welcome contributions, but because of the strict performance constraints, all submissions are evaluated on empirical metrics .

## 1. The Core Rule: No AI on the Hot Path
Current LLMs and AI coding assistants rely on statistical averages and standard library abstractions. They do not understand L1/L2 cache-line splitting, AVX-512 SIMD alignment requirements, or branch prediction penalties.

For this reason, **AI-generated code is strictly prohibited for the core allocation logic.** All pointer arithmetic, alignment math, and fallback policies must be handwritten and manually verified at the assembly level.

## 2. Modifying the Allocation Hot Path
If you are submitting a Pull Request that modifies `tip`'s core allocation logic, alignment calculations, or fallback policies, your PR **must** include the following in the description:

*   **x86_64 Assembly Output:** Provide a Godbolt/Compiler Explorer link comparing the before-and-after assembly of the hot path. Changes that introduce unnecessary stack frames, virtual dispatch, or branching (`cmp`/`jmp`) on the primary allocation path will be **rejected**.
*   **Micro-Benchmarks:** Provide the terminal output of the Google Benchmark suite. Your implementation must demonstrably match or beat the baseline throughput of `std::pmr::monotonic_buffer_resource`.

**Important** : As of now `tip` lacks a comprehensive benchmarking and testing suite . Everyone willing is more that 
welcome to start working on it!

## 3. Memory Safety & CI/CD
Do not request a review if the CI pipeline is red. Your code __must be__ subjected to:
*   AddressSanitizer (ASAN)
*   MemorySanitizer (MSAN)
*   UndefinedBehaviorSanitizer (UBSAN)
*   Valgrind Memcheck

If a PR introduces a memory leak, segfault, or undefined behavior, it will be closed until the contributor resolves the issue locally.

## 4. High-Value Contribution Areas
If you want to contribute to the repository without navigating the assembly constraints of the hot path, we highly encourage PRs in the following infrastructure areas:

*   **Testing Infrastructure:** Expanding the GTest coverage for edge-case memory exhaustion scenarios and LIFO stack checkpoints.
*   **Fuzzing:** Writing fuzzers to test bizarre or massive alignment requests to ensure the bitwise masking holds up.
*   **Mutation Testing:** Integrating and expanding Mull to guarantee our test suite actually catches logic regressions.
*   **Build Systems:** Enhancing the CMake and Ninja build configurations, or improving `compile_commands.json` generation.
*   **Documentation:** Expanding Doxygen coverage or writing markdown tutorials on how the bitwise alignment math functions under the hood.

## 5. Submitting a PR
1. Fork the repository and create your feature branch.
2. Ensure you are building with C++23 standards enabled (`-std=c++23`).
3. Run the full sanitizer suite and Google Benchmarks locally.
4. Open a PR and fill out the PR with your metrics.

**Important** : Metrics are not required as of now (subject to change) .
