generator_release :
	cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

generator_debug :
	cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

build :
	cmake --build build -j$$(nproc)

tests :
	ctest --test-dir build --output-on-failure -j$$(nproc) --repeat until-fail:10000

benchmarks :
	taskset -c 2 ./build/run_benchmarks

main :
	./build/tip

clean :
	rm -r build

debug_tests :
	rm -r build
	make generator_debug
	make build
	~/gf/gf2 ./build/run_tests

.PHONY : generator build tests benchmarks main clean
