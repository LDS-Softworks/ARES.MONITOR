.PHONY: all clean build nossl

# --- Tools and Flags ---
## For now we just support:
## OpenOrbis SDK (c++ Compiler toolchain)

## Specific OpenSSL Compiled against OpenOrbis' SDK, you NEED to cross-compile this yourself.
# You may find OpenSSL's source code @ https://github.com/openssl/openssl/
OPENSSL_LIB_PATH = ./Modules/openssl/build

## Legacy code stuff that i really don't need to change... yet.
CXX         = clang++
INCLUDES 	=  -I./src/include -I./src/include/* -I./src/include/modules/transcript/
CFLAGS      = -std=c++17
SOURCES     := $(wildcard src/*.cpp)
# --- Output Paths for binaries, only for Linux and MacOS x86_64 ---
## even though it can indeed emmit arm64 binaries using clang++ on macOS, if ran on an Apple Silicon Apple Computer.
COUT 		= ./bin/ARES_RBSD.elf

# --- Build Targets ---
all: build

# Target to build all modules into ARES.
# If you defined any other modules, add them here so they are compiled into the binary. if not, these could be skipped.
# Please wrap your modules into a #ifdef __MODULE_%MODNAME%. to make this process easier.
ALLWMOD:
	@echo All modules will be built...
	@make CFLAGS+=-D__MODULE_AEX CFLAGS+=-D__MODULE_ASC

build:
	if [ -d "bin" ]; then \
		echo "./bin exists, continuing..."; \
	else \
		mkdir -p bin; \
	fi
	@$(MAKE) ALLWMOD

$(COUT):build
## Missing SSL and Crypto libraries because... idk, idk where macOS has the compatLinux SSL/Crypto/OpenSSL Libraries at. or if i can even use those.
nossl: $(SOURCES)
	@echo "Cross-compiling for Linux via Zig..."
	@$(ZIGCXX) $(ZIGCFLAGS_LINUX) $(INCLUDES) $^ -o $(LINUXCOUT)

clean:
	@bash ./Helpers/clean.sh