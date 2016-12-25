all: parser

OBJS = parser.o  \
       codegen.o \
       main.o    \
       tokens.o  \
       corefn.o  \
       native.o  \
       slot.o    \

LLVMCONFIG = llvm-config
CPPFLAGS = `$(LLVMCONFIG) --cppflags` -std=c++11 -fblocks
LDFLAGS = `$(LLVMCONFIG) --ldflags` -lpthread -ldl -lz -lncurses -rdynamic -lBlocksRuntime
LIBS = `$(LLVMCONFIG) --libs`

clean:
	$(RM) -rf parser.cpp parser.hpp tokens.cpp ../compiler parser.output $(OBJS)

parser.cpp: parser.y
	bison -d -o $@ $^ -v

parser.hpp: parser.cpp

tokens.cpp: tokens.l parser.hpp
	flex -o $@ $^

%.o: %.cpp
	clang++ -c $(CPPFLAGS) -o $@ $<

parser: $(OBJS)
	clang++ -o ../compiler $(OBJS) $(LIBS) $(LDFLAGS)
