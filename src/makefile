SRCS = $(wildcard *.cpp)

TARGET = cobra-movegen
CXX = clang++

FLAGS = -Wall -Wextra -Wshadow -Wmissing-declarations -Wno-missing-braces -Wconversion -fno-exceptions -std=c++20

debug = no
optimise = yes

ifneq ($(debug),yes)
	FLAGS += -DNDEBUG
# else
# 	FLAGS += -g
endif

ifneq ($(optimise),no)
	FLAGS += -O3 -funroll-loops -march=native -mtune=native
	ifneq ($(debug),yes)
		FLAGS += -flto
	endif
endif

help:
	@echo ""
	@echo "To compile Cobra Movegen, type:"
	@echo "make -j [target] [config]"
	@echo ""
	@echo "Supported targets:"
	@echo "help                   shows this message"
	@echo "build                  build binary"
	@echo ""
	@echo "Supported configs for build ({} represents default):"
	@echo "debug    =  yes  / {no}"
	@echo "optimise = {yes} /  no "
	@echo ""

build:
	$(CXX) -o $(TARGET) $(FLAGS) $(SRCS)