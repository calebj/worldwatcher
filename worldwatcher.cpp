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
#include <FL/fl_ask.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Multiline_Output.H>

struct Game_window : Graph_lib::Window {
    Game_window(Point xy, int w, int h, const string& title);
    int wait_for_button();
    void set_action(int);
    void display_home();
    void undisplay_home();
    void display_instructions();
    void undisplay_instructions();
    void display_scores();
    void undisplay_scores();
private:
    Vector<Shape*> scorelines;
    Image instructions_text;
    Image logo, bg;
    int action;
    Button start_button, help_button, scores_button, mainmenu_button;
    static void cb_start(Address, Address);
    static void cb_help(Address, Address);
    static void cb_scores(Address, Address);
    static void cb_main(Address, Address);
};

void Game_window::set_action(int i) {
    action = i;
    Fl::check();
}

int Game_window::wait_for_button() {
    show();
    int a = action;
    while(a == action) {
        Fl::wait();
    }
    return action;
}

void window_callback(Fl_Widget* widget, void*) {
    if (fl_ask("Do you really want to exit?")) {
        ((Game_window*)widget)->set_action(0);
        ((Fl_Window*)widget)->hide();
    }
}

void Game_window::display_home() {
    attach(logo);
    attach(start_button);
    attach(help_button);
    attach(scores_button);
}

void Game_window::undisplay_home() {
    detach(logo);
    detach(start_button);
    detach(help_button);
    detach(scores_button);
}

void Game_window::display_instructions() {
    attach(instructions_text);
    attach(mainmenu_button);
}

void Game_window::undisplay_instructions() {
    detach(instructions_text);
    detach(mainmenu_button);
}

void Game_window::display_scores() {
    ifstream ifs {"scores.txt"};
    vector<String> lines;
    int startx = 512;
    int starty = 160;
    fl_font(FL_HELVETICA_BOLD, 48);
    if (!ifs)
        scorelines.push_back(new Text(Point{startx, starty}, "No high scores recorded."));
    else {
        string line;
        int i=0;
        while (getline(ifs, line) and i<5) {
            lines.push_back(line);
        }
        int y=starty;
        for (String l:lines) {
            scorelines.push_back(new Text(Point{startx, y}, l));
            y += 56;
        }
    }
    for (Shape* s:scorelines)
        attach(*s);
    attach(mainmenu_button);
}

void Game_window::undisplay_scores() {
    for (Shape* s:scorelines)
        detach(*s);
    detach(mainmenu_button);
}

void Game_window::cb_start(Address, Address pw) {
    reference_to<Game_window>(pw).set_action(1);
}


void Game_window::cb_help(Address, Address pw) {
    reference_to<Game_window>(pw).set_action(2);
}

void Game_window::cb_scores(Address, Address pw) {
    reference_to<Game_window>(pw).set_action(3);
}

void Game_window::cb_main(Address, Address pw) {
    reference_to<Game_window>(pw).set_action(4);
}


Game_window::Game_window(Point xy, int w, int h, const string& title):
    Window{xy,w,h,title},
    action(4),
    // logo is 920px wide
    logo{Point{x_max()/2-920/2,96}, "logo.png", Graph_lib::Suffix::png},
    bg{Point{0,0}, "world-map-background.jpg", Graph_lib::Suffix::jpg},
    start_button{Point{x_max()/2 - 192/2, y_max() - 2*(48/2 + 48)}, 192, 48, "Start game", cb_start},
    help_button{Point{x_max()/2 - 88 - 8, y_max() - 48/2 - 48}, 88, 24, "How to play", cb_help},
    scores_button{Point{x_max()/2 + 8, y_max() - 48/2 - 48}, 88, 24, "High scores", cb_scores},
    mainmenu_button{Point{x_max()/2 - 192/2, y_max() - (48/2 + 48)}, 192, 48, "Main menu", cb_main},
    instructions_text{Point{48,48}, "instructions.png", Graph_lib::Suffix::png} {
        attach(bg);
        display_home();
    }

int main() {
    try {
        const int win_width = 1280;
        const int win_height = 720;
        Game_window win(Point(100,200),win_width,win_height,"WorldWatcher 2");
        win.callback(window_callback);
        int action = 4; // homescreen
        while (action != 0) {
            int lastaction = action;
            action = win.wait_for_button();
            if (action == 0)
                cout << "User quit\n";

            if(action == 1)
                cout << "Start game here\n";

            if(action == 2)
                win.display_instructions();
            else if(lastaction == 2)
                win.undisplay_instructions();

            if(action == 3)
                win.display_scores();
            else if (lastaction == 3)
                win.undisplay_scores();

            if(action == 4)
                win.display_home();
            else if (lastaction == 4)
                win.undisplay_home();
            Fl::redraw();
        }
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
