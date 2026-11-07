generator_release :
	cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

generator_debug :
	cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

build :
	cmake --build build -j$$(nproc)

tests :
	ctest --test-dir build --output-on-failure

benchmarks :
	./build/run_benchmarks

main :
	./build/tip

.PHONY : generator build tests benchmarks main
