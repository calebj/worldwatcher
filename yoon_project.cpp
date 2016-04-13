#include "std_lib_facilities_4.h"
#include "gamewindow.cpp"
#include "Graph.h"
#include "Window.h"
#include "GUI.h"
using namespace Graph_lib;

int main(){
	game_window win1(Point(0,0), 850, 675, "Game");
	return gui_main();
}