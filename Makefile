.PHONY: format
format:
	./scripts/format.sh

CMAKE_USER_OPTIONS=""
.PHONY: cmake-build
cmake-build:
	cmake -S . -B build $(CMAKE_USER_OPTIONS)

.PHONY: build
build: cmake-build
	cmake --build build --verbose

.PHONY: test
test: build
	ctest --test-dir build --output-on-failure
