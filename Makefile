CXX ?= g++
CXXFLAGS ?= -Wall -g
MKDIR ?= mkdir -p
DEPDIR ?= .deps
OBJECTS := macexe.o macresfork.o code.o code0.o jumptable.o idc.o staticdata.o util.o main.o
BIN := macloader

$(BIN): $(OBJECTS)
	$(CXX) $+ -o $@

-include $(wildcard $(addsuffix /*.d,$(DEPDIR)))

%.o: %.cpp
	$(MKDIR) $(DEPDIR)
	$(CXX) -MMD -MF "$(DEPDIR)/$(*F).d" -MQ "$@" -MP $(CXXFLAGS) -c $(<) -o $*.o

clean:
	rm -f $(BIN)
	rm -f $(OBJECTS)

all: $(BIN)
