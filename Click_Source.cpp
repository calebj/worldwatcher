#include "std_lib_facilities_4.h"
#include "Simple_window.h"
#include "Graph.h"
//#include <ctime>
//#include <cstdlib>

Button *s1_N, *s1_S, *s1_E, *s1_W;

void cb_NSWE(Address addr, Address pw)
{

}

void cb_sattle(Address addr, Address pw)
{
    int px=reference_to<Fl_Widget>(addr).x();
    int py=reference_to<Fl_Widget>(addr).y();

    //Generate Buttons---------------------------------------------------------------
    s1_N = new Button(Point(px,py-40),20,20,"N",cb_NSWE);
    s1_S = new Button(Point(px,py+40),20,20,"S",cb_NSWE);
    s1_W = new Button(Point(px-40,py),20,20,"W",cb_NSWE);
    s1_E = new Button(Point(px+40,py),20,20,"E",cb_NSWE);
    
    reference_to<Simple_window>(pw).attach(*s1_N);
    reference_to<Simple_window>(pw).attach(*s1_S);
    reference_to<Simple_window>(pw).attach(*s1_E);
    reference_to<Simple_window>(pw).attach(*s1_W);
    reference_to<Simple_window>(pw).redraw();
}

int main()
try {
  if(H112 != 201401L)
	  error("Error: incorrect std_lib_facilities_4.h version ", H112);
  
	
	using namespace Graph_lib;
	
    //Simple window------------------------------------------------------------------
	Point tl(100,100);
	
	Simple_window win(tl,600,400,"Click");
	//-------------------------------------------------------------------------------
    
    //Generate two random circle-----------------------------------------------------
    int x, y, w = 20, h=20;
  
    srand(time(0));
    
    x=rand()%(600-w) + (w/2);
    y=rand()%(400-h) + (h/2);
    Button s1(Point(x,y),w,h,"S1",cb_sattle);
    cout << x << "\t" << y << endl;
    
    x=rand()%600;
    y=rand()%400;
    Button s2(Point(x,y),w,h,"S2",cb_sattle);
	//--------------------------------------------------------------------------------
    
    
    
    
    
    
    
    
    
    win.attach(s1);
    win.attach(s2);
	win.wait_for_button();
  
  
  return 0;
}
catch(exception& e) {
	cerr << "exception: " << e.what() << '\n';
	return 1;
}
catch (...) {
	cerr << "Some exception\n";
	return 2;
}
