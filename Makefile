RELEASE_FLAGS=-O3
DEBUG_FLAGS=-ggdb
SECURITY_FLAGS=-fstack-protector -D_FORTIFY_SOURCE=2 -fsanitize=address,leak,undefined
WARNING_FLAGS=-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Wuninitialized -Wfloat-equal -Wformat=2 -Wundef -Werror -Wunused-parameter -Wunused-variable -Wunused-function -Wmissing-declarations -Wreorder -Wswitch-enum -Wdeprecated-declarations -Wvla -Wlogical-op
LD_FLAGS=-L./deps -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl
INCLUDE_FLAGS=-Iinclude

# We wanna change this if we compile in debug or release mode.
CURRENT_BUILD_FLAGS=$(INCLUDE_FLAGS) $(DEBUG_FLAGS) $(WARNING_FLAGS) $(SECURITY_FLAGS)
DEPS_BUILD_FLAGS=$(INCLUDE_FLAGS) $(DEBUG_FLAGS)


all: output tags deps
	g++ $(DEPS_BUILD_FLAGS) -c src/glad.c -o output/glad.o
	g++ $(CURRENT_BUILD_FLAGS) -c src/main.cpp -o output/main.o $(LD_FLAGS)
	g++ $(CURRENT_BUILD_FLAGS) output/main.o output/glad.o -o output/dumb_program $(LD_FLAGS)

output: clean
	mkdir -p output

tags:
	find . -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' -type f | xargs etags

clean:
	rm -rf output/

run:
	./output/dumb_program
