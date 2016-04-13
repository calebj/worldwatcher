CC     = g++     # the c compiler to use
DFLAGS = -std=c++14        # common defines
FLTK   = -lfltk -lfltk_images	# fltk defines

all: main

clean:
	rm -f *.o

# ==== FLTK LIBRARY ====
Graph.o: Graph.cpp
	$(CC) $(DFLAGS) -c -o $@ $<
	
GUI.o: GUI.cpp
	$(CC) $(DFLAGS) -c -o $@ $<
	
Simple_window.o: Simple_window.cpp
	$(CC) $(DFLAGS) -c -o $@ $<
	
Window.o: Window.cpp
	$(CC) $(DFLAGS) -c -o $@ $<

# ==== MAIN FILE ====
main: Graph.o GUI.o Simple_window.o Window.o
	$(CC) $(DFLAGS) $(FLTK) yoon_project.cpp Graph.o GUI.o Simple_window.o Window.o

