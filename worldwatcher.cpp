#include "std_lib_facilities_4.h"
#include "fltk/Graph.h"
#include "fltk/Simple_window.h"
#include "fltk/Window.h"
#include "fltk/GUI.h"
// #include <FL/Fl.H>
// #include <FL/Fl_Window.H>
// #include <FL/Fl_Shared_Image.H>
// #include <FL/Fl_PNG_Image.H>
// #include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_Box.H>

struct Splash_window : Graph_lib::Window {
    Splash_window(Point xy, int w, int h, const string& title);
    int wait_for_button();
private:
    Image logo, bg;
    int action;
    Button start_button, help_button, scores_button;
    static void cb_start(Address, Address);
    static void cb_help(Address, Address);
    static void cb_scores(Address, Address);
    void set_action(int);
};

void Splash_window::set_action(int i) {
    action = i;
//     hide();
}

int Splash_window::wait_for_button() {
    show();
    do
        Fl::wait();
    while(action == 0);
    return action;
}

void Splash_window::cb_start(Address, Address pw) {
    reference_to<Splash_window>(pw).set_action(1);
}

void Splash_window::cb_help(Address, Address pw) {
    reference_to<Splash_window>(pw).set_action(2);
}

void Splash_window::cb_scores(Address, Address pw) {
    reference_to<Splash_window>(pw).set_action(3);
}

Splash_window::Splash_window(Point xy, int w, int h, const string& title):
    Window{xy,w,h,title},
    action(0),
    // logo is 512px wide
    logo{Point{x_max()/2-512/2,96}, "logo.png", Graph_lib::Suffix::png},
    bg{Point{0,0}, "world-map-background.jpg", Graph_lib::Suffix::jpg},
    start_button{Point{x_max()/2 - 192/2, y_max() - 2*(48/2 + 48)}, 192, 48, "Start game", cb_start},
    help_button{Point{x_max()/2 - 88 - 8, y_max() - 48/2 - 48}, 88, 24, "How to play", cb_help},
    scores_button{Point{x_max()/2 + 8, y_max() - 48/2 - 48}, 88, 24, "High scores", cb_scores} {
        attach(bg);
        attach(logo);
        attach(start_button);
        attach(help_button);
        attach(scores_button);
    }

int main() {
    try {
        const int win_width = 1280;
        const int win_height = 720;
        Splash_window win(Point(100,200),win_width,win_height,"WorldWatcher 2");
        int action = win.wait_for_button();
        if(action == 1)
            cout << "Start game here\n";
        else if(action == 2)
            cout << "Show instructions\n";
        else if(action == 3)
            cout << "Show scores\n";
        else
            cout << "User quit\n";
        return 0;
    }
    catch(exception& e) {
        cerr << "error: " << e.what() << '\n';
        return 1;
    }
    catch (...) {
        cerr << "Oops: unknown exception!\n";
        return 2;
    }
}
