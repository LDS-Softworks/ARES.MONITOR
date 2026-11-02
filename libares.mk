## Make File for the shared library libares, for Embedded targets.
OUT			="bin/lib/libares.so"
CXX			= clang++
INCLUDES 	=  -I./src/include -I./src/include/* -I./src/include/modules/transcript/
CFLAGS      = -std=c++17 -D__SELF_LIBARES -D__MODULE_AEX -D__MODULE_ASC -Wall -Wextra -Wpedantic -shared -fPIC
SOURCES     := $(wildcard src/*.cpp)

.PHONY: all clean

all:
	if [ -d "bin/lib" ]; then \
		echo "bin/lib exists, continuing..."; \
	else \
		mkdir -p bin/lib; \
	fi
	$(CXX) $(CFLAGS) $(INCLUDES) -o $(OUT) $(SOURCES)
