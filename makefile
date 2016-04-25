CXX     = g++     # the c compiler to use
CXXFLAGS = -std=c++14 -g       # common defines
FLTK   = -lfltk -lfltk_images	# fltk libs
FLTK_LOCATION = ./fltk/

FLTK_OBJS = Graph.o GUI.o Simple_window.o Window.o

BINS = worldwatcher

all: $(BINS)

clean:
	rm $(FLTK_OBJS) $(BINS)

# ==== FLTK LIBRARY ====
Graph.o: $(FLTK_LOCATION)/Graph.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	
GUI.o: $(FLTK_LOCATION)/GUI.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	
Simple_window.o: $(FLTK_LOCATION)/Simple_window.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	
Window.o: $(FLTK_LOCATION)/Window.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

worldwatcher: worldwatcher.cpp $(FLTK_OBJS)
	$(CXX) $(CXXFLAGS) $(FLTK) -o $@ $(FLTK_OBJS) worldwatcher.cpp
