CXX=g++
RELEASE_FLAGS := -O3
DEBUG_FLAGS := -ggdb -O2
SECURITY_FLAGS := -fstack-protector -D_FORTIFY_SOURCE=2  -fsanitize=address,leak,undefined
WARNING_FLAGS := -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wsign-conversion -Wuninitialized -Wfloat-equal -Wformat=2 -Wundef -Wunused-parameter -Wunused-variable -Wunused-function -Wmissing-declarations -Wreorder -Wswitch-enum -Wdeprecated-declarations -Wvla -Wlogical-op
LD_FLAGS := -L./deps -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl
INCLUDE_FLAGS := -Iinclude

OUTPUT_DIR := output

# Pre-compiled header flag
PCH_HEADER := include/pch.hpp
PCH_OUTPUT := $(OUTPUT_DIR)/glm_pch.hpp.gch

# change this line to switch between release/debug.
CURRENT_BUILD_FLAGS := $(INCLUDE_FLAGS) $(DEBUG_FLAGS) $(WARNING_FLAGS) # Not using security for now because it takes too much time! $(SECURITY_FLAGS)

# no need to change it
DEPS_BUILD_FLAGS := $(INCLUDE_FLAGS) $(DEBUG_FLAGS)

all: output tags deps precompile_headers
	$(CXX) $(DEPS_BUILD_FLAGS) -c src/glad.c -o $(OUTPUT_DIR)/glad.o
	$(CXX) $(CURRENT_BUILD_FLAGS) -include $(PCH_HEADER) -c src/main.cpp -o $(OUTPUT_DIR)/main.o $(LD_FLAGS)
	$(CXX) $(CURRENT_BUILD_FLAGS) -include $(PCH_HEADER) -c src/Shader.cpp -o $(OUTPUT_DIR)/Shader.o $(LD_FLAGS)
	$(CXX) $(CURRENT_BUILD_FLAGS) -include $(PCH_HEADER) $(OUTPUT_DIR)/Shader.o $(OUTPUT_DIR)/main.o $(OUTPUT_DIR)/glad.o -o $(OUTPUT_DIR)/dumb_program $(LD_FLAGS)

precompile_headers:
	$(CXX) $(DEPS_BUILD_FLAGS) -x c++-header $(PCH_HEADER) -o $(PCH_OUTPUT)

output:
	mkdir -p $(OUTPUT_DIR)

tags:
	find . -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' -type f | xargs etags

clean:
	rm -rf output/

run:
	./output/dumb_program

debug:
	gdb $(OUTPUT_DIR)/dumb_program
