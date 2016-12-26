all: parser

OBJS = $(GM)/parser.o  \
       $(GM)/lexer.o   \
       main.o          \
       $(CR)/codegen.o \
       $(CR)/corefn.o  \
       $(CR)/slot.o    \
       native.o        \

LLVMCONFIG = llvm-config
CPPFLAGS = `$(LLVMCONFIG) --cppflags` -std=c++11 -fblocks
LDFLAGS = `$(LLVMCONFIG) --ldflags` -lpthread -ldl -lz -lncurses -rdynamic -lBlocksRuntime
LIBS = `$(LLVMCONFIG) --libs`
GM = grammar
CR = core

clean:
	$(RM) -rf $(GM)/parser.cpp $(GM)/parser.hpp $(GM)/lexer.cpp $(GM)/parser.output ../compiler $(OBJS)

$(GM)/parser.cpp: $(GM)/parser.y
	bison -d -o $@ $^ -v

$(GM)/parser.hpp: $(GM)/parser.cpp

$(GM)/lexer.cpp: $(GM)/lexer.l $(GM)/parser.hpp
	flex -o $@ $^

%.o: %.cpp
	clang++ -c $(CPPFLAGS) -o $@ $<

parser: $(OBJS)
	clang++ -o ../compiler $(OBJS) $(LIBS) $(LDFLAGS)
