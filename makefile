CXX     = g++
# common defines
FLTK_DIST = ./fltk-1.3.3/
# -static-libgcc -Wl,-static
CXXFLAGS = -std=c++14 -Wall
#-I$(FLTK_DIST)

# Dynamically link libraries./fltk-1.3.3//lib/fltk_images.a
FLTK_LIBS = fltk fltk_images
FLTK   = $(addprefix -l,$(FLTK_LIBS))

# Static link libraries
#FLTK_LIBS = $(addsuffix .a,fltk fltk_images)
#FLTK   = $(addprefix $(FLTK_DIST)/lib/lib,$(FLTK_LIBS))

FLTK_LOCATION = ./fltk/
FLTK_OBJS = Graph.o GUI.o Window.o
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
