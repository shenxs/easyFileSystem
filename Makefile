compile:*.cpp
	g++ -g main.cpp -std=c++11
debug:./a.out
	gdb -tui ./a.out
redisk:
	rm virtualdisk ;touch virtualdisk
clean:
	rm *.out
