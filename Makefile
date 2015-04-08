CC = g++
SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:%.cpp=%.o)
AR = ar
BUILDEXE = glf.exe
BUILDAR = glf.a



all : $(OBJS)
	$(AR) cr $(BUILDAR) $^ 
	ranlib $(BUILDAR)
	$(CC) -g -o $(BUILDEXE) $^ -lpthread
	rm -f *.o
	rm -f *.d

sinclude $(SRCS:%.cpp=%.d)

%.o: %.cpp
	$(CC) -g -c $< -o $@

%.d : %.cpp
	@set -e; \
	$(CC) -M $< \
	| sed 's/$(*F)\.o[ :]*/$(subst /,\/,$(basename $@)).o $(subst /,\/,$@) : /g' > $@; \
	#[ -s $@ ] || rm -f $@



.PHONY : clean
clean :
	rm -f $(BUILDEXE)
	rm -f $(BUILDAR)
	rm -f *.o 
	rm -f *.d


