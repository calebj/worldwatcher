#include "std_lib_facilities_4.h"
#include "Graph.h"
#include "Window.h"
#include "GUI.h"
#include "playwindow.cpp"

using namespace Graph_lib;


struct game_window : Graph_lib::Window 
{
    game_window(Point xy, int w, int h, const string& title);
	Button start_button;
	Button explain_button;
	Button menu_button;
	Button next_button;
	Image logo;
	Image rule1;
	Image rule2;


	play_window win2;

	int explain_screen = 0;

private:
	bool wait_for_button();
    bool button_pushed;
	Button quit_button;
	Button back_button;

	static void cb_quit(Address, Address window); 
	void quit();
	static void cb_start(Address, Address window);
	void start();
	static void cb_explain(Address, Address window);
	void explain();
	static void cb_back(Address, Address window);
	void back();
	static void cb_menu(Address, Address window);
	void menu();
	static void cb_next(Address, Address window);
	void next();

};

bool game_window::wait_for_button(){
	show();
    button_pushed = false;
    while (!button_pushed) Fl::wait();
    Fl::redraw();
    return button_pushed;
}

void game_window::cb_quit(Address, Address pw)
	{
		reference_to<game_window>(pw).quit();
	} 

void game_window::quit(){Fl_Window::hide();}

void game_window::cb_explain(Address, Address pw){
	reference_to<game_window>(pw).explain();
}

void game_window::explain(){
		detach(logo);
		attach(back_button);
		attach(next_button);
		attach(menu_button);
		Fl_Window::redraw();
		start_button.hide();
		explain_button.hide();
		attach(rule1);
		explain_screen++;
		Fl_Window::redraw();
}
void game_window::cb_start(Address, Address pw)
	{
		reference_to<game_window>(pw).start();
	}
void game_window::start(){
	Fl_Window::hide();
	win2.show();
}

void game_window::cb_back(Address, Address pw)
{
 	reference_to<game_window>(pw).back();
} 

void game_window::back()
{
	if(explain_screen == 1){
		back_button.hide();
		next_button.hide();
		menu_button.hide();
		explain_button.show();
		start_button.show();
		attach(logo);
		detach(rule1);
		explain_screen -= 1;
		Fl_Window::redraw();
	}
	else if(explain_screen == 2){
		detach(rule2);
		attach(rule1);
		explain_screen -=1;
		Fl_Window::redraw();
	}

}

void game_window::cb_menu(Address, Address pw)
	{
		reference_to<game_window>(pw).menu();
	} 

void game_window::menu(){
	if(explain_screen == 1){
		back_button.hide();
		next_button.hide();
		menu_button.hide();
		explain_button.show();
		start_button.show();
		attach(logo);
		detach(rule1);
		explain_screen = 0;
		Fl_Window::redraw();
	}
	else if(explain_screen == 2){
		back_button.hide();
		next_button.hide();
		menu_button.hide();
		explain_button.show();
		start_button.show();
		attach(logo);
		detach(rule2);
		explain_screen = 0;
		Fl_Window::redraw();
	}
	
}

void game_window::cb_next(Address, Address pw)
	{
		reference_to<game_window>(pw).next();
	} 

void game_window::next(){
	if(explain_screen == 1){
		detach(rule1);
		attach(rule2);
		explain_screen++;
		Fl_Window::redraw();
	}
	
	
}


game_window::game_window(Point xy, int w, int h, const string& title) 
	:Window(xy,w,h,title),
	win2(Point(0,0),900,700,"play"),
	quit_button(Point(0,0), 200, 80, "Quit", cb_quit),
	start_button(Point((x_max()/2)-250,480),200,80,"Start", cb_start),
	explain_button(Point(x_max()/2+50,480),200,80, "How to play", cb_explain),
	menu_button(Point(x_max()-300,0), 150, 50, "menu", cb_menu),
	back_button(Point(x_max()-150,0),150,50, "back", cb_back),
	next_button(Point(x_max()-450,0),150,50, "next", cb_next),
	logo(Point(0,0), "logo.jpg"),
	rule1(Point(0,0), "rule1.jpg"),
	rule2(Point(0,0), "rule2.jpg")
	

{
	attach(logo);
	attach(quit_button);
	attach(start_button);
	attach(explain_button);
	win2.hide();
}