
//
// This is a GUI support code to the chapters 12-16 of the book
// "Programming -- Principles and Practice Using C++" by Bjarne Stroustrup
//

#include "Window.h"
#include "Graph.h"
#include "GUI.h"

//------------------------------------------------------------------------------

namespace Graph_lib {

Window::Window(int ww, int hh, const string& title, const bool& resizable)
    :Fl_Window(ww,hh,title.c_str()),w(ww),h(hh)
{
    init(resizable);
}

//------------------------------------------------------------------------------

Window::Window(Point xy, int ww, int hh, const string& title,
        const bool& resizable)
    :Fl_Window(xy.x,xy.y,ww,hh,title.c_str()),w(ww),h(hh)
{ 
    init(resizable);
}

//------------------------------------------------------------------------------

void Window::init(const bool& is_resizable)
{
    if (is_resizable)
        resizable(this);
    show();
}

//------------------------------------------------------------------------------

void Window::draw()
{
    Fl_Window::draw();
    for (unsigned int i=0; i<shapes.size(); ++i) shapes[i]->draw();
    for (unsigned int i=0; i<widgets.size(); ++i) widgets[i]->draw();
}

//------------------------------------------------------------------------------

void Window::attach(Widget& w)
{
    begin();         // FTLK: begin attaching new Fl_Wigets to this window
    w.attach(*this); // let the Widget create its Fl_Wigits
    widgets.push_back(&w);
    end();           // FTLK: stop attaching new Fl_Wigets to this window
}

//------------------------------------------------------------------------------

void Window::detach(Widget& b)
{
    b.hide();
    for (vector<Widget*>::size_type i = widgets.size(); 0<i; --i)    
        if (widgets[i-1]==&b)
            widgets.erase(widgets.begin()+(i-1));
}

//------------------------------------------------------------------------------

void Window::detach(Shape& s)
    // guess that the last attached will be first released
{
	for (vector<Shape*>::size_type i = shapes.size(); 0<i; --i)    
        if (shapes[i-1]==&s)
            shapes.erase(shapes.begin()+(i-1));
}

//------------------------------------------------------------------------------

void Window::put_on_top(Shape& p) {
    for (int i=0; i<shapes.size(); ++i) {
        if (&p==shapes[i]) {
            for (++i; i<shapes.size(); ++i)
                shapes[i-1] = shapes[i];
            shapes[shapes.size()-1] = &p;
            return;
        }
    }
}

//------------------------------------------------------------------------------

int gui_main()
{
    return Fl::run();
}

//------------------------------------------------------------------------------

} // of namespace Graph_lib
