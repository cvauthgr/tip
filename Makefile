generator :
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

build :
	cmake --build build -j$$(nproc)

tests :
	ctest --test-dir build --output-on-failure -j$$(nproc) --repeat until-fail:100

benchmarks :
	./build/run_benchmarks

main :
	./build/main

.PHONY : generator build tests benchmarks main
