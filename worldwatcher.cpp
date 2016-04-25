#include "std_lib_facilities_4.h"
#include "fltk/Graph.h"
#include "fltk/Window.h"
#include "fltk/GUI.h"
#include <FL/Fl.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Box.H>
#include <math.h>
#include <string>

using namespace Graph_lib;

const Point MAP_UL = {0,0};
Color SAT_COLOR = FL_WHITE;
Color SAT_BORDER = FL_BLACK;
Color SAT_HINT = FL_GREEN;
const int MAX_LAT = 82;
const double MERC_MAGIC = 16/3.0071727; // Scale for 82deg truncated mercator
const int MAP_W = 1024;
const int MAP_H = 720;
const int SAT_RADIUS = 12;
const double EARTH_RADIUS = 3958.8;

struct latlong {
    int latitude;
    int longitude;
};

void cb_sat_activate(void*, void*); // Declare only

Point latlong_mercpoint(latlong p, Point ul, int w, int h) {
    // longitude is linear
    int x = (p.longitude+180)*(w/360.0) + ul.x;
    // latitude is a bit trickier
    double mercN = log(tan((M_PI/4)+(p.latitude*M_PI/360.0)));
    int y = (h/2)-(h*mercN/MERC_MAGIC) + ul.y;
    return Point{x,y};
}

class Satellite {
    latlong position;
    int num;
    int moves_left;
    void updateViz();
public:
    Button* viz;
    Satellite(int satnum) {
        position.latitude = rand() % (2*MAX_LAT) - MAX_LAT;
        position.longitude = rand() % 360 - 180;
        num = satnum;
        viz = new Button {getxy_offset(), 2*SAT_RADIUS, 2*SAT_RADIUS, to_string(num), cb_sat_activate};
    }
    Point getxy();
    Point getxy_offset();
    virtual ~Satellite() {}
    void move_north(int a) {move(a, 0);}
    void move_south(int a) {move(-a, 0);}
    void move_east(int a)  {move(0, a);}
    void move_west(int a)  {move(0, -a);}
    void move(int, int);
    int get_number() {return num;}
    latlong get_position() {return position;}
};

// void Satellite::showhint(bool hint) {
//     if(hint)
//         viz.set_fill_color(SAT_HINT);
//     else
//         viz.set_fill_color(SAT_COLOR);
// }

Point Satellite::getxy() {
    return latlong_mercpoint(position, MAP_UL, MAP_W, MAP_H);
}

Point Satellite::getxy_offset() {
    Point p = getxy();
    return Point{p.x-SAT_RADIUS, p.y-SAT_RADIUS};
}

void Satellite::updateViz() {
    viz->moveto(getxy_offset());
    Fl::redraw();
}

void Satellite::move(int d_latitude, int d_longitude) {
    position.latitude += d_latitude;
    // Can't go more than +-90 latitude
    if (position.latitude > MAX_LAT)
        position.latitude = MAX_LAT;
    else if (position.latitude < 0-MAX_LAT)
        position.latitude = -MAX_LAT;

    // Modulo 360 after shifting 180 degrees, then unshift
    if (180+position.longitude+d_longitude < 0)
        position.longitude = 180-(-(180+position.longitude+d_longitude) % 360);
    else
        position.longitude = ((180+position.longitude+d_longitude) % 360)-180;
    cout << position.latitude << " " << position.longitude << "\n";
    updateViz();
}


double greatcircledist(latlong p1, latlong p2) {
    // Uses Vincenty's formula for computing arc length. Returns central angle,
    // not arc length. Multiply with radius to get arc length.
    double dlongitude = abs(p2.longitude-p1.longitude);
    return atan(
        sqrt(
            pow(cos(p2.latitude)*sin(dlongitude),2) + 
            pow(cos(p1.latitude)*sin(p2.latitude) - 
            sin(p1.latitude)*cos(p2.latitude)*cos(dlongitude),2)
        )/( sin(p1.latitude)*sin(p2.latitude) + 
        cos(p1.latitude)*cos(p2.latitude)*cos(dlongitude)
        )
    );
}

double maxdist_satellites(vector<Satellite*> s) {
    double maxdist = 0;
    for (Satellite* i:s) {
        for (Satellite* j:s) {
            double d = greatcircledist(i->get_position(), j->get_position());
            d *= EARTH_RADIUS;
            if (d > maxdist)
                maxdist = d;
        }
    }
    return maxdist;
}

struct Game_window : Graph_lib::Window {
    Game_window(Point xy, int w, int h, const string& title);
    int wait_for_button();
    void set_action(int);
    int get_action() { return action; }
    void display_home();
    void undisplay_home();
    void display_instructions();
    void undisplay_instructions();
    void display_scores();
    void undisplay_scores();
    void display_game(int);
    void undisplay_game();
    int get_difficulty() {return difficulty;}
    Vector<Satellite*> get_satellites() {return satellites;}
    void show_compass(Satellite*);
    void hide_compass();
    Satellite* get_selected_sat() {return selected_sat;}
private:
    Satellite* selected_sat;
    Vector<Button*> compass;
    Vector<Satellite*> satellites;
    int difficulty = 2;
    Counter difficulty_widget;
    int action = 4;
    Vector<Shape*> scorelines;
    Image logo, bg, gamemap, instructions_text, difficulty_label;
    Button start_button, help_button, scores_button, mainmenu_button;
    static void cb_start(Address, Address);
    static void cb_help(Address, Address);
    static void cb_scores(Address, Address);
    static void cb_main(Address, Address);
    static void cb_difficulty(Address, Address);
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
    bool exit = true;
    if (((Game_window*)widget)->get_action() == 1) // if in-game
        exit = fl_ask("Do you really want to exit?");
    if (exit) {
        ((Game_window*)widget)->set_action(0);
        ((Fl_Window*)widget)->hide();
    }
}

void Game_window::display_home() {
    attach(logo);
    attach(start_button);
    attach(help_button);
    attach(scores_button);
    attach(difficulty_widget);
    attach(difficulty_label);

    //These can't be done in the window constructor because the widget doesn't
    // exist until it's attached (a quirk of the glue code).
    difficulty_widget.step(1);
    difficulty_widget.value(difficulty);
    difficulty_widget.bounds(2, 8);
}

void Game_window::undisplay_home() {
    detach(logo);
    detach(start_button);
    detach(help_button);
    detach(scores_button);
    detach(difficulty_widget);
    detach(difficulty_label);
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

void Game_window::display_game(int difficulty) {
    attach(gamemap);
    for (int i=0; i<difficulty; ++i) {
        Satellite* sat = new Satellite {i+1};
        satellites.push_back(sat);
        latlong p = sat->get_position();
        cout << sat->get_number() << " " << p.longitude << "\n";
        Point xy = latlong_mercpoint(p, MAP_UL, MAP_W, MAP_H);
        attach(*sat->viz);
        sat->viz->color(SAT_COLOR);
        sat->viz->box(FL_OVAL_BOX);
    }
    cout << maxdist_satellites(satellites) << "\n";
}

void Game_window::undisplay_game() {
    detach(gamemap);
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

void Game_window::cb_difficulty(Address w, Address pw) {
    Fl_Valuator* slider = (Fl_Valuator*)w;
    int d = slider->value();
    reference_to<Game_window>(pw).difficulty = d;
}

Game_window::Game_window(Point xy, int w, int h, const string& title):
    Window{xy,w,h,title},
    // logo is 920px wide
    logo{Point{x_max()/2-920/2,96}, "logo.png", Graph_lib::Suffix::png},
    bg{Point{0,0}, "world-map-background.jpg", Graph_lib::Suffix::jpg},
    start_button{Point{x_max()/2 - 192/2, y_max() - 2*(48/2 + 48)}, 88, 48, "Start game", cb_start},
    help_button{Point{x_max()/2 - 88 - 8, y_max() - 48/2 - 48}, 88, 24, "How to play", cb_help},
    scores_button{Point{x_max()/2 + 8, y_max() - 48/2 - 48}, 88, 24, "High scores", cb_scores},
    mainmenu_button{Point{x_max()/2 - 192/2, y_max() - (48/2 + 48)}, 192, 48, "Main menu", cb_main},
    instructions_text{Point{48,48}, "instructions.png", Graph_lib::Suffix::png},
    difficulty_widget{Point{x_max()/2 - 192/2 + 192 - 88, y_max() - 2*(48/2 + 48) + 20}, 88, 24, "Difficulty", cb_difficulty},
    difficulty_label{Point{x_max()/2 - 192/2 + 192 - 88, y_max() - 2*(48/2 + 48)}, "difficulty_label.png",Graph_lib::Suffix::png},
    gamemap{MAP_UL, "mercator-projection.jpg", Graph_lib::Suffix::jpg}
    {
        attach(bg);
        display_home();
}

void cb_compass(Address w, Address pw) {
    string dir(static_cast<Fl_Widget*>(w)->label());
    Game_window* win = (Game_window*)pw;
    Satellite* sat = win->get_selected_sat();
    char c = dir.c_str()[0];
    switch(c) {
        case 'N':
            sat->move_north(15);
            break;
        case 'S':
            sat->move_south(15);
            break;
        case 'E':
            sat->move_east(15);
            break;
        case 'W':
            sat->move_west(15);
            break;
    }
    win->hide_compass();
    win->show_compass(sat);
}

void Game_window::hide_compass() {
    for(Button* b : compass)
    {
        detach(*b);
    }
    Fl::redraw();
}

void Game_window::show_compass(Satellite* sat) {
    hide_compass();
    selected_sat = sat;
    Point p = sat->getxy_offset();
    vector<Button*> newcompass;
    int sp = 32;
    int wh = 2*SAT_RADIUS;
    newcompass.push_back(new Button(Point(p.x,p.y-sp),wh,wh,"N",cb_compass));
    newcompass.push_back(new Button(Point(p.x,p.y+sp),wh,wh,"S",cb_compass));
    newcompass.push_back(new Button(Point(p.x-sp,p.y),wh,wh,"W",cb_compass));
    newcompass.push_back(new Button(Point(p.x+sp,p.y),wh,wh,"E",cb_compass));
    for(Button* b:newcompass)
    {
        attach(*b);
    }
    compass = newcompass;
    Fl::redraw();
}

void cb_sat_activate(Address w, Address pw) {
    string label(static_cast<Fl_Widget*>(w)->label());
    Game_window* win = (Game_window*)pw;
    int i = stoi(label)-1;
    Satellite* sat = win->get_satellites()[i];
    win->hide_compass();
    win->show_compass(sat);
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
                win.display_game(win.get_difficulty());
            else if(lastaction == 1)
                win.undisplay_game();

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
