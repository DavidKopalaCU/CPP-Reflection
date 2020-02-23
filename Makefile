CXX = g++
CXXFLAGS = -g -rdynamic

main: main.o Reflection.o ElfReader.o ReflectiveClass.o
	$(CXX) $(CXXFLAGS) -o main main.o Reflection.o ElfReader.o ReflectiveClass.o

clean:
	rm -r *.o