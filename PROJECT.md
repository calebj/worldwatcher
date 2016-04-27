#World Watcher II
Authors:
* Caleb Johnson-Tasks 1,2,3,4,5,6,Extra
* Alex Kaiser-Tasks 1,2,4
* Jae Jun-Tasks 1,2,5,Extra
* Each member did a fair share of work.

Problem:

You are a secret agent attempting to calculate best positioning of satellites to survey the planet in its entirety with the most efficiency. In order to do this the secret agent must use his knowledge of spherical coordinates and general judgement to place the satellites around a mercator projection of earth. This project's purpose is to create a simple game using the C++ FLTK and GUI functionalities as well as the robust standard library of C++ commands.

Restrictions:

Restrictions include the fact that the secret agent only has access to 2-8 satellites and must position them accurately to survey the most possible land around sperical earth. The agent must place the multiple satellites on a mercator map as far apart as possible, with each satellite having a certain amount of fuel that slowly deminishes the amount of space each satellite can cover with each move. The programmers of the game are also not allowed to have a function longer than 24 lines in order to streamline any debugging necessary to make the game perform at its maximum.

Approach:

We began by using a service called GIT, which easily combines the coding of each individiual member into one master code. We created a master .cpp file titled worldwatcher which contained the master code for the window and multiple attatchments for the game. The FLTK files were modified slightly in order to accomidate certain aspects such as attatching a button over an image and PNG support.

Sample Run:

The code of the game is on difficulty 4 (4 satellites)



Results and Analysis:

The project was successful. All basic aspects of the project were covered as well as the extra items. The high scores menu contains the top 5 scores, the game works in all difficutlties and the spehrical coordinates/wraparound are incredibly accurate. The game clock properly concatinates to the difficulty level and the score multiplier is also on point. The hint button shows the user the best move to make due by shading the satellite that should be moved next to obtain a higher score. 

Conclusion:

This program shows that the FLTK/GUI aspects of C++ are capable of creating and running a simple game. We learned multiple C++ tactics such as subclassing, attatching objects and basic GUI controls. On top of this, we learned the powerful aspects of the FLTK program such including the many callbacks and robust functions it has to offer. As an introductory course to C++, this project was a good test of the power of C++ while learning the foundations of programming.

Instructions:

Complile the code using the makefile included in the CD/folder and run using ./worldwatcher when in the worldwatcher directory. The make file should contain the modified FLTK and GUI files as well as the worldwatcher.cpp file, if the code refuses to compile check the makefile to see if this holds true. All instructions on how to work the game itself are included in the "How to play" section.

Program Code:

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
const int NUM_SCORES = 5;

struct latlong {
    int latitude;
    int longitude;
};

vector<int> STEPS = {1,5,10,15,20};
int get_step(int moves_left) {
    int i = (moves_left - (moves_left % 10))/10;
    if(i+1>STEPS.size())
        i=STEPS.size()-1;
    return STEPS[i];
}

struct Score {
    int score;
    String initials;
    // Overload operators to allow sort
    inline bool operator> (const Score& rhs) const {return this->score>rhs.score;}
    inline bool operator< (const Score& rhs) const {return this->score<rhs.score;}
};


vector<Score> read_highscores() {
    vector<Score> scores;
    ifstream ifs {"scores.txt"};
    int i=0;
    Score s;
    while (ifs >> s.initials >> s.score and i<NUM_SCORES) {
        ++i;
        scores.push_back(s);
    }
    // Sort in descending order
    sort(scores.begin(), scores.end(),greater<Score>());
    return scores;
}

void write_highscores(vector<Score> scores) {
    ofstream ofs {"scores.txt"};
    if(!ofs)
        cout << "Error writing scores file\n";
    else {
        sort(scores.begin(), scores.end(),greater<Score>());
        if (scores.size() > NUM_SCORES)
            scores.resize(NUM_SCORES);
        for(Score s : scores)
            ofs << s.initials << " " << s.score << "\n";
        }
}

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
    void hint(bool);
    void select();

    int get_number() {return num;}
    latlong get_position() {return position;}
};

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

void Satellite::hint(bool on) {
    if(on)
        viz->color(SAT_HINT);
    else
        viz->color(SAT_COLOR);
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
    updateViz();
}


double greatcircledist(latlong p1, latlong p2) {
    // Uses Vincenty's formula for computing arc length. Returns central angle,
    // not arc length. Multiply with radius to get arc length.
    double lat1 = M_PI*p1.latitude/180.0;
    double long1 = M_PI*p1.longitude/180.0;
    double lat2 = M_PI*p2.latitude/180.0;
    double long2 = M_PI*p2.longitude/180.0;
    double dlongitude = abs(long2-long1);
    return atan2(
        sqrt(
            pow(cos(lat2)*sin(dlongitude),2) + 
            pow(cos(lat1)*sin(lat2) - 
            sin(lat1)*cos(lat2)*cos(dlongitude),2)
        ),( sin(lat1)*sin(lat2) + 
        cos(lat1)*cos(lat2)*cos(dlongitude)
        )
    );
}

int maxdist_satellites(vector<Satellite*> s) {
    if (s.size() < 2)
        return 0;
    double maxdist = 0;
    for (Satellite* i:s) {
        for (Satellite* j:s) {
            double d = greatcircledist(i->get_position(), j->get_position());
            if (d > maxdist)
                maxdist = d;
        }
    }
    return EARTH_RADIUS*maxdist;
}

int mindist_satellites(vector<Satellite*> s) {
    if (s.size() < 2)
        return 0;
    // initialize with first two
    double min = greatcircledist(s[0]->get_position(), s[1]->get_position());
    for (Satellite* i:s) {
        for (Satellite* j:s) {
            if(i!=j) {
                double d = greatcircledist(i->get_position(), j->get_position());
                if (d < min)
                    min = d;
            }
        }
    }
    return EARTH_RADIUS*min;
}

// return a pointer to the satellite that is closest to its neighbors
Satellite* hint_satellite (vector<Satellite*> s) {
    if (s.size() < 2)
        return 0;
    Satellite* candidate;
    double min;
    // initialize with first two
    double sumdist;
    for (Satellite* i:s) {
        sumdist = 0;
        for (Satellite* j:s) {
            sumdist += greatcircledist(i->get_position(), j->get_position());
        }
        if(i == s[0] || sumdist < min) {
            min = sumdist;
            candidate = i;
        }
    }
    return candidate;
}

struct Game_window : Graph_lib::Window {
    Game_window(Point xy, int w, int h, const string& title);
    int wait_for_button();
    void set_action(int);
    int get_action() { return action; }

    // View switching functions
    void display_home();
    void undisplay_home();
    void display_instructions();
    void undisplay_instructions();
    void display_scores();
    void undisplay_scores();
    void display_game(int);
    void undisplay_game();

    Vector<Satellite*> satellites;
    Satellite* selected_sat;

    void show_hint();
    void show_compass(Satellite*);
    void hide_compass();

    int difficulty = 2;
    int time_remaining;
    int moves_left;
    void update_sideinfo();
    void game_over();
    void game_over(bool);

private:
    int action = 4;

    Text timer_display, moves_left_display, score_display, best_score_display;
    Vector<Button*> compass;
    Vector<Text*> scorelines;
    Counter difficulty_widget;
    Image logo, bg, gamemap, instructions_text, difficulty_label;
    Button start_button, help_button, scores_button, mainmenu_button, quit_game, hint_button;

    static void cb_start(Address, Address);
    static void cb_help(Address, Address);
    static void cb_scores(Address, Address);
    static void cb_main(Address, Address);
    static void cb_difficulty(Address, Address);
    static void cb_endgame(Address, Address);
    static void cb_hint(Address, Address);
};

void Game_window::update_sideinfo() {
    int s = time_remaining % 60;
    int m = (time_remaining - s) / 60;
    stringstream ss;
    ss << setfill('0') << setw(2) << m << ":" << setfill('0') << setw(2) << s;
    timer_display.set_label(ss.str());

    ss.str("");
    ss << setfill('0') << setw(2) << moves_left << " moves remaining";
    moves_left_display.set_label(ss.str());

    ss.str("");
    ss << "Score:  " << setfill('0') << setw(5) << mindist_satellites(satellites)*difficulty;
    score_display.set_label(ss.str());

    Fl::redraw();
}

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
    int startx = 420;
    int starty = 160;
    int width = 12;
    int spacing = 60;
    vector<Score> scores = read_highscores();
    scorelines = {};
    if (scores.size() == 0)
        scorelines.push_back(new Text(Point{startx, starty}, "No high scores."));
    else {
        scorelines.push_back(new Text(Point{startx, starty}, "High scores:"));
        int y=starty + spacing;
        for (Score s:scores) {
            stringstream ss;
            int pad = width - to_string(s.score).size() - s.initials.size();
            ss << s.initials << setfill(' ') << setw(pad) << "" << s.score;
            Text* t = new Text(Point{startx, y}, ss.str());
            scorelines.push_back(t);
            y += spacing;
        }
    }
    for (Text* t : scorelines) {
        t->set_font(FL_COURIER_BOLD);
        t->set_font_size(64);
        attach(*t);
    }
    attach(mainmenu_button);
}

void Game_window::undisplay_scores() {
    for (Text* t:scorelines)
        detach(*t);
    detach(mainmenu_button);
}

void gametimer(void* pw) {
    Game_window* win = (Game_window*)pw;
    win->time_remaining -= 1;
    int s = win->time_remaining % 60;
    int m = (win->time_remaining-s) / 60;
    win->update_sideinfo();
    if(win->time_remaining <= 0) {
        win->game_over();
    } else
        Fl::repeat_timeout(1.0, gametimer, pw);
}

void Game_window::game_over() {
    game_over(false);
}
void Game_window::game_over(bool manual) {
    if(manual)
        if(!fl_ask("Are you sure?"))
            return;
    Fl::remove_timeout(gametimer);
    int score = mindist_satellites(satellites)*difficulty;
    vector<Score> scores = read_highscores();
    if ((!scores.empty() && score > scores.back().score) || scores.size() < NUM_SCORES) {
        char* initials = (char*)fl_input("Congratulations, you made a high score! Enter your initials to save it.");
        while(initials != 0 && (strlen(initials) > 3 || strlen(initials) == 0))
            initials = (char*)fl_input("Invalid entry. Please enter one to three characters.");
        if (initials != 0) {
            scores.push_back(Score{score, initials});
            write_highscores(scores);
            set_action(3);
            return;
        }
    } else {
        if(!manual and fl_ask("Game over! Play again?")) {
            set_action(1);
            return;
        }
    }
    set_action(4);
    Fl::redraw();
}

void Game_window::cb_endgame(Address, Address pw) {
    reference_to<Game_window>(pw).game_over(true);
}

void Game_window::cb_hint(Address, Address pw) {
    reference_to<Game_window>(pw).show_hint();
}

void Game_window::show_hint() {
    Satellite *hint = hint_satellite(satellites);
    hint->hint(true);
    time_remaining -= 5*difficulty; // penalty
    moves_left -= 1;
    update_sideinfo();
    if(time_remaining <= 0)
        game_over();
}

void Game_window::display_game(int difficulty) {
    attach(gamemap);
    attach(quit_game);
    attach(hint_button);
    satellites = {};
    for (int i=0; i<difficulty; ++i) {
        Satellite* sat = new Satellite {i+1};
        satellites.push_back(sat);
        latlong p = sat->get_position();
        Point xy = latlong_mercpoint(p, MAP_UL, MAP_W, MAP_H);
        attach(*sat->viz);
        sat->viz->color(SAT_COLOR);
        sat->viz->box(FL_OVAL_BOX);
    }

    moves_left = 50;
    time_remaining = 30*difficulty;

    vector<Text*> formatvec = {&timer_display, &moves_left_display, &score_display, &best_score_display};
    for(Text* t: formatvec) {
        t->set_color(FL_WHITE);
        t->set_font(FL_COURIER);
    }

    Fl::add_timeout(1, gametimer, this);
    timer_display.set_font_size(80);
    attach(timer_display);
    update_sideinfo();

    moves_left_display.set_font_size(21);
    attach(moves_left_display);

    score_display.set_font_size(30);
    attach(score_display);

    vector<Score> hs = read_highscores();
    if (hs.size() > 0) {
        stringstream ss;
        ss << "Best:   " << setfill('0') << setw(5) << hs[0].score;
        best_score_display.set_label(ss.str());
        best_score_display.set_font_size(30);
        attach(best_score_display);
    }
}

void Game_window::undisplay_game() {
    detach(gamemap);
    detach(quit_game);
    detach(hint_button);
    detach(timer_display);
    detach(moves_left_display);
    detach(score_display);
    detach(best_score_display);
    hide_compass();
    for(Satellite* s : satellites)
    {
        detach(*s->viz);
    }
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
    gamemap{MAP_UL, "mercator-projection.jpg", Graph_lib::Suffix::jpg},
    quit_game{Point{x_max() - (x_max() - MAP_W)/2 - 192/2, y_max() - (48/2 + 48)}, 192,48, "End game", cb_endgame},
    hint_button{Point{x_max() - (x_max() - MAP_W)/2 - 192/2, y_max() - 2*(48/2 + 32)}, 192,32, "Hint", cb_hint},
    timer_display{Point{MAP_W + 10, 72}, "--:--"},
    moves_left_display{Point{MAP_W + 10, 108}, "-- moves left"},
    score_display{Point{MAP_W + 10, 144}, "Score:  -----"},
    best_score_display{Point{MAP_W + 10, 180}, "Best:   -----"}
    {
        attach(bg);
        display_home();
}

void cb_compass(Address w, Address pw) {
    string dir(static_cast<Fl_Widget*>(w)->label());
    Game_window* win = (Game_window*)pw;
    Satellite* sat = win->selected_sat;
    char c = dir.c_str()[0];
    int m = win->moves_left;
    int step = get_step(m);
    switch(c) {
        case 'N':
            sat->move_north(step);
            break;
        case 'S':
            sat->move_south(step);
            break;
        case 'E':
            sat->move_east(step);
            break;
        case 'W':
            sat->move_west(step);
            break;
    }
    win->moves_left--;
    if(win->moves_left <= 0)
        win->game_over();
    win->update_sideinfo();
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
    for(Satellite* s : satellites)
        s->hint(false);
    selected_sat = sat;
    Point p = sat->getxy_offset();
    int sp = 32;
    int wh = 2*SAT_RADIUS;
    compass = {};
    compass.push_back(new Button(Point(p.x,p.y-sp),wh,wh,"N",cb_compass));
    compass.push_back(new Button(Point(p.x,p.y+sp),wh,wh,"S",cb_compass));
    compass.push_back(new Button(Point(p.x-sp,p.y),wh,wh,"W",cb_compass));
    compass.push_back(new Button(Point(p.x+sp,p.y),wh,wh,"E",cb_compass));
    for(Button* b:compass)
    {
        attach(*b);
    }
    Fl::redraw();
}

void Satellite::select() {
    hint(false);
}

void cb_sat_activate(Address w, Address pw) {
    string label(static_cast<Fl_Widget*>(w)->label());
    Game_window* win = (Game_window*)pw;
    int i = stoi(label)-1;
    Satellite* sat = win->satellites[i];
    sat->select();
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
//             if (action == 0)
//                 cout << "User quit\n";

            if(lastaction == 1)
                win.undisplay_game();
            if(action == 1)
                win.display_game(win.difficulty);

            if(lastaction == 2)
                win.undisplay_instructions();
            if(action == 2)
                win.display_instructions();

            if (lastaction == 3)
                win.undisplay_scores();
            if(action == 3)
                win.display_scores();

            if (lastaction == 4)
                win.undisplay_home();
            if(action == 4)
                win.display_home();

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

//
// This is a GUI support code to the chapters 12-16 of the book
// "Programming -- Principles and Practice Using C++" by Bjarne Stroustrup
//

#include <FL/Fl.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Simple_Counter.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Multiline_Output.H>
#include "GUI.h"

namespace Graph_lib {

//------------------------------------------------------------------------------

void Button::attach(Window& win)
{
    pw = new Fl_Button(loc.x, loc.y, width, height, label.c_str());
    pw->callback(reinterpret_cast<Fl_Callback*>(do_it), &win); // pass the window
    own = &win;
}

//------------------------------------------------------------------------------

void Counter::attach(Window& win)
{
    pv = new Fl_Simple_Counter(loc.x, loc.y, width, height, label.c_str());
    pw = (Fl_Widget*)pv;
    pw->callback(reinterpret_cast<Fl_Callback*>(do_it), &win); // pass the window
    own = &win;
}

//------------------------------------------------------------------------------

int In_box::get_int()
{
    Fl_Input& pi = reference_to<Fl_Input>(pw);
    // return atoi(pi.value());
    const char* p = pi.value();
    if (!isdigit(p[0])) return -999999;
    return atoi(p);
}

//------------------------------------------------------------------------------

string In_box::get_string()
{
	Fl_Input& pi = reference_to<Fl_Input>(pw);
	return string(pi.value());
}

//------------------------------------------------------------------------------

void In_box::attach(Window& win)
{
    pw = new Fl_Input(loc.x, loc.y, width, height, label.c_str());
    own = &win;
}

//------------------------------------------------------------------------------

void Out_box::put(const string& s)
{
    reference_to<Fl_Output>(pw).value(s.c_str());
}

//------------------------------------------------------------------------------

void Out_box::attach(Window& win)
{
    pw = new Fl_Output(loc.x, loc.y, width, height, label.c_str());
    own = &win;
}

//------------------------------------------------------------------------------

void Out_box_multi::put(const string& s)
{
    reference_to<Fl_Output>(pw).value(s.c_str());
}

//------------------------------------------------------------------------------

void Out_box_multi::attach(Window& win)
{
    pw = new Fl_Output(loc.x, loc.y, width, height, label.c_str());
    own = &win;
}

//------------------------------------------------------------------------------

int Menu::attach(Button& b)
{
    b.width = width;
    b.height = height;

    switch(k) {
    case horizontal:
        b.loc = Point(loc.x+offset,loc.y);
        offset+=b.width;
        break;
    case vertical:
        b.loc = Point(loc.x,loc.y+offset);
        offset+=b.height;
        break;
    }
    selection.push_back(b); // b is NOT OWNED: pass by reference
    return int(selection.size()-1);
}

//------------------------------------------------------------------------------

int Menu::attach(Button* p)
{
    Button& b = *p;
    b.width = width;
    b.height = height;

    switch(k) {
    case horizontal:
        b.loc = Point(loc.x+offset,loc.y);
        offset+=b.width;
        break;
    case vertical:
        b.loc = Point(loc.x,loc.y+offset);
        offset+=b.height;
        break;
    }
    selection.push_back(&b); // b is OWNED: pass by pointer
    return int(selection.size()-1);
}

//------------------------------------------------------------------------------

} // of namespace Graph_lib

//
// This is a GUI support code to the chapters 12-16 of the book
// "Programming -- Principles and Practice Using C++" by Bjarne Stroustrup
//

#ifndef GUI_GUARD
#define GUI_GUARD

#include <FL/Fl_Valuator.H>
#include "Window.h"
#include "Graph.h"

namespace Graph_lib {

//------------------------------------------------------------------------------

    typedef void* Address;    // Address is a synonym for void*
    typedef void(*Callback)(Address, Address);    // FLTK's required function type for all callbacks

//------------------------------------------------------------------------------

    template<class W> W& reference_to(Address pw)
    // treat an address as a reference to a W
    {
        return *static_cast<W*>(pw);
    }

//------------------------------------------------------------------------------

    class Widget {
    // Widget is a handle to an Fl_widget - it is *not* an Fl_widget
    // We try to keep our interface classes at arm's length from FLTK
    public:
        Widget(Point xy, int w, int h, const string& s, Callback cb)
            : loc(xy), width(w), height(h), label(s), do_it(cb)
            {}

        virtual void move(int dx,int dy) { hide(); pw->position(loc.x+=dx, loc.y+=dy); show(); }
        virtual void moveto(Point xy) { hide(); pw->position(xy.x, xy.y); show(); }
        virtual void color(Color bg) {pw->color(bg.as_int()); }
        virtual void hide() { pw->hide(); }
        virtual void show() { pw->show(); }
        virtual void attach(Window&) = 0;
        void redraw() { pw->redraw(); }
        void draw() { pw->draw(); }
        void box(Fl_Boxtype b) {pw->box(b);}

        Point loc;
        int width;
        int height;
        string label;
        Callback do_it;

        virtual ~Widget() { }

    protected:
        Window* own;    // every Widget belongs to a Window
        Fl_Widget* pw;  // connection to the FLTK Widget
    private:
        Widget& operator=(const Widget&); // don't copy Widgets
        Widget(const Widget&);
    };
    
    class Valuator : public Widget {
    public:
        Valuator(Point xy, int w, int h, const string& label, Callback cb)
            : Widget(xy,w,h,label,cb) {}

        void step(double s) { pv->step(s); };
        void bounds(double min, double max) {pv->bounds(min,max); } ;
        void value(double v) { pv->value(v); }
        double value() {return pv->value(); };
        void type(uchar t) { pv->type(t); }

    protected:
        Fl_Valuator* pv;
    };

//------------------------------------------------------------------------------

    struct Counter : Valuator {
        Counter(Point xy, int w, int h, const string& label, Callback cb)
        : Valuator(xy,w,h,label,cb)
        {}

        void attach(Window&);
    };

//------------------------------------------------------------------------------

    struct Button : Widget {
        Button(Point xy, int w, int h, const string& label, Callback cb)
            : Widget(xy,w,h,label,cb)
            {}

        void attach(Window&);
    };

//------------------------------------------------------------------------------

    struct In_box : Widget {
        In_box(Point xy, int w, int h, const string& s)
            :Widget(xy,w,h,s,0) { }
        int get_int();
        string get_string();

        void attach(Window& win);
    };

//------------------------------------------------------------------------------

    struct Out_box : Widget {
        Out_box(Point xy, int w, int h, const string& s)
            :Widget(xy,w,h,s,0) { }
        void put(int);
        void put(const string&);

        void attach(Window& win);
    };

    struct Out_box_multi : Widget {
        Out_box_multi(Point xy, int w, int h, const string& s)
        :Widget(xy,w,h,s,0) { }
        void put(int);
        void put(const string&);
        void attach(Window& win);
    };
//------------------------------------------------------------------------------

    struct Menu : Widget {
        enum Kind { horizontal, vertical };
        Menu(Point xy, int w, int h, Kind kk, const string& label)
            : Widget(xy,w,h,label,0), k(kk), offset(0)
        {}

        Vector_ref<Button> selection;
        Kind k;
        int offset;
        int attach(Button& b);      // Menu does not delete &b
        int attach(Button* p);      // Menu deletes p

        void show()                 // show all buttons
        {
            for (unsigned int i = 0; i<selection.size(); ++i)
                selection[i].show();
        }
        void hide()                 // hide all buttons
        {
            for (unsigned int i = 0; i<selection.size(); ++i) 
                selection[i].hide(); 
        }
        void move(int dx, int dy)   // move all buttons
        {
            for (unsigned int i = 0; i<selection.size(); ++i) 
                selection[i].move(dx,dy);
        }

        void attach(Window& win)    // attach all buttons
        {
            for (int i=0; i<selection.size(); ++i) win.attach(selection[i]);
            own = &win;
        }

    };

//------------------------------------------------------------------------------

} // of namespace Graph_lib

#endif // GUI_GUARD

/*
   Graph.cpp
   Minimally revised for C++11 features of GCC 4.6.3 or later
   Walter C. Daugherity		June 10, 2012
   Fixed bug in Axis::x label position	November 17, 2013
   Update for C++14			October 10, 2015
*/

//
// This is a GUI support code to the chapters 12-16 of the book
// "Programming -- Principles and Practice Using C++" by Bjarne Stroustrup
//

#include <FL/Fl_GIF_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_PNG_Image.H>
#include "Graph.h"
#include <cstring>

//------------------------------------------------------------------------------

namespace Graph_lib {

//------------------------------------------------------------------------------

Shape::Shape() : 
    lcolor(fl_color()),      // default color for lines and characters
    ls(0),                   // default style
    fcolor(Color::invisible) // no fill
{}

//------------------------------------------------------------------------------

void Shape::add(Point p)     // protected
{
    points.push_back(p);
}

//------------------------------------------------------------------------------

void Shape::set_point(int i,Point p)        // not used; not necessary so far
{
    points[i] = p;
}

//------------------------------------------------------------------------------

void Shape::draw_lines() const
{
    if (color().visibility() && 1<points.size())    // draw sole pixel?
        for (unsigned int i=1; i<points.size(); ++i)
            fl_line(points[i-1].x,points[i-1].y,points[i].x,points[i].y);
}

//------------------------------------------------------------------------------

void Shape::draw() const
{
    Fl_Color oldc = fl_color();
    // there is no good portable way of retrieving the current style
    fl_color(lcolor.as_int());            // set color
    fl_line_style(ls.style(),ls.width()); // set style
    draw_lines();
    fl_color(oldc);      // reset color (to previous)
    fl_line_style(0);    // reset line style to default
}

//------------------------------------------------------------------------------


void Shape::move(int dx, int dy)    // move the shape +=dx and +=dy
{
    for (int i = 0; i<points.size(); ++i) {
        points[i].x+=dx;
        points[i].y+=dy;
    }
}

//------------------------------------------------------------------------------

Line::Line(Point p1, Point p2)    // construct a line from two points
{
    add(p1);    // add p1 to this shape
    add(p2);    // add p2 to this shape
}

//------------------------------------------------------------------------------

void Lines::add(Point p1, Point p2)
{
    Shape::add(p1);
    Shape::add(p2);
}

//------------------------------------------------------------------------------

// draw lines connecting pairs of points
void Lines::draw_lines() const
{
    if (color().visibility())
        for (int i=1; i<number_of_points(); i+=2)
            fl_line(point(i-1).x,point(i-1).y,point(i).x,point(i).y);
}

//------------------------------------------------------------------------------

// does two lines (p1,p2) and (p3,p4) intersect?
// if se return the distance of the intersect point as distances from p1
inline pair<double,double> line_intersect(Point p1, Point p2, Point p3, Point p4, bool& parallel) 
{
    double x1 = p1.x;
    double x2 = p2.x;
    double x3 = p3.x;
    double x4 = p4.x;
    double y1 = p1.y;
    double y2 = p2.y;
    double y3 = p3.y;
    double y4 = p4.y;

    double denom = ((y4 - y3)*(x2-x1) - (x4-x3)*(y2-y1));
    if (denom == 0){
        parallel= true;
        return pair<double,double>(0,0);
    }
    parallel = false;
    return pair<double,double>( ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3))/denom,
                                ((x2-x1)*(y1-y3) - (y2-y1)*(x1-x3))/denom);
}

//------------------------------------------------------------------------------

//intersection between two line segments
//Returns true if the two segments intersect,
//in which case intersection is set to the point of intersection
bool line_segment_intersect(Point p1, Point p2, Point p3, Point p4, Point& intersection){
   bool parallel;
   pair<double,double> u = line_intersect(p1,p2,p3,p4,parallel);
   if (parallel || u.first < 0 || u.first > 1 || u.second < 0 || u.second > 1) return false;
   intersection.x = p1.x + u.first*(p2.x - p1.x);
   intersection.y = p1.y + u.first*(p2.y - p1.y);
   return true;
}

//------------------------------------------------------------------------------

void Polygon::add(Point p)
{
    int np = number_of_points();

    if (1<np) {    // check that thenew line isn't parallel to the previous one
        if (p==point(np-1)) error("polygon point equal to previous point");
        bool parallel;
        line_intersect(point(np-1),p,point(np-2),point(np-1),parallel);
        if (parallel)
            error("two polygon points lie in a straight line");
    }

    for (int i = 1; i<np-1; ++i) {    // check that new segment doesn't interset and old point
        Point ignore(0,0);
        if (line_segment_intersect(point(np-1),p,point(i-1),point(i),ignore))
            error("intersect in polygon");
    }
    

    Closed_polyline::add(p);
}

//------------------------------------------------------------------------------

void Polygon::draw_lines() const
{
    if (number_of_points() < 3) error("less than 3 points in a Polygon");
    Closed_polyline::draw_lines();
}

//------------------------------------------------------------------------------

void Open_polyline::draw_lines() const
{
    if (fill_color().visibility()) {
        fl_color(fill_color().as_int());
        fl_begin_complex_polygon();
        for(int i=0; i<number_of_points(); ++i){
            fl_vertex(point(i).x, point(i).y);
        }
        fl_end_complex_polygon();
        fl_color(color().as_int());    // reset color
    }
    
    if (color().visibility())
        Shape::draw_lines();
}

//------------------------------------------------------------------------------

void Closed_polyline::draw_lines() const
{
    Open_polyline::draw_lines();    // first draw the "open poly line part"
    // then draw closing line:
    if (color().visibility())
        fl_line(point(number_of_points()-1).x, 
        point(number_of_points()-1).y,
        point(0).x,
        point(0).y);
}

//------------------------------------------------------------------------------

void draw_mark(Point xy, char c)
{
    static const int dx = 4;
    static const int dy = 4;

    string m(1,c);
    fl_draw(m.c_str(),xy.x-dx,xy.y+dy);
}

//------------------------------------------------------------------------------

void Marked_polyline::draw_lines() const
{
    Open_polyline::draw_lines();
    for (int i=0; i<number_of_points(); ++i) 
        draw_mark(point(i),mark[i%mark.size()]);
}

//------------------------------------------------------------------------------

void Rectangle::draw_lines() const
{
    if (fill_color().visibility()) {    // fill
        fl_color(fill_color().as_int());
        fl_rectf(point(0).x,point(0).y,w,h);
        fl_color(color().as_int());    // reset color
    }

    if (color().visibility()) {    // lines on top of fill
        fl_color(color().as_int());
        fl_rect(point(0).x,point(0).y,w,h);
    }
}

//------------------------------------------------------------------------------
/*
Circle::Circle(Point p, int rr)    // center and radius
:r(rr)
{
    add(Point(p.x-r,p.y-r));       // store top-left corner
}
*/
//------------------------------------------------------------------------------

Point Circle::center() const
{
    return Point(point(0).x+r, point(0).y+r);
}

//------------------------------------------------------------------------------

void Circle::draw_lines() const
{
  	if (fill_color().visibility()) {	// fill
		fl_color(fill_color().as_int());
		fl_pie(point(0).x,point(0).y,r+r-1,r+r-1,0,360);
		fl_color(color().as_int());	// reset color
	}

	if (color().visibility()) {
		fl_color(color().as_int());
		fl_arc(point(0).x,point(0).y,r+r,r+r,0,360);
	}
}

//------------------------------------------------------------------------------

void Ellipse::draw_lines() const
{
   if (fill_color().visibility()) {	// fill
		fl_color(fill_color().as_int());
		fl_pie(point(0).x,point(0).y,w+w-1,h+h-1,0,360);
		fl_color(color().as_int());	// reset color
	}

	if (color().visibility()) {
		fl_color(color().as_int());
		fl_arc(point(0).x,point(0).y,w+w,h+h,0,360);
	}
}

//------------------------------------------------------------------------------

void Text::draw_lines() const
{
    int ofnt = fl_font();
    int osz = fl_size();
    fl_font(fnt.as_int(),fnt_sz);
    fl_draw(lab.c_str(),point(0).x,point(0).y);
    fl_font(ofnt,osz);
}

//------------------------------------------------------------------------------

Axis::Axis(Orientation d, Point xy, int length, int n, string lab) :
    label(Point(0,0),lab)
{
    if (length<0) error("bad axis length");
    switch (d){
    case Axis::x:
    {
        Shape::add(xy); // axis line
        Shape::add(Point(xy.x+length,xy.y));

        if (0<n) {      // add notches
            int dist = length/n;
            int x = xy.x+dist;
            for (int i = 0; i<n; ++i) {
                notches.add(Point(x,xy.y),Point(x,xy.y-5));
                x += dist;
            }
        }
        // label under the line
        label.move(xy.x+length/3,xy.y+20);
        break;
    }
    case Axis::y:
    {
        Shape::add(xy); // a y-axis goes up
        Shape::add(Point(xy.x,xy.y-length));

        if (0<n) {      // add notches
            int dist = length/n;
            int y = xy.y-dist;
            for (int i = 0; i<n; ++i) {
                notches.add(Point(xy.x,y),Point(xy.x+5,y));
                y -= dist;
            }
        }
        // label at top
        label.move(xy.x-10,xy.y-length-10);
        break;
    }
    case Axis::z:
        error("z axis not implemented");
    }
}

//------------------------------------------------------------------------------

void Axis::draw_lines() const
{
    Shape::draw_lines();
    notches.draw();  // the notches may have a different color from the line
    label.draw();    // the label may have a different color from the line
}

//------------------------------------------------------------------------------

void Axis::set_color(Color c)
{
    Shape::set_color(c);
    notches.set_color(c);
    label.set_color(c);
}

//------------------------------------------------------------------------------

void Axis::move(int dx, int dy)
{
    Shape::move(dx,dy);
    notches.move(dx,dy);
    label.move(dx,dy);
}

//------------------------------------------------------------------------------

Function::Function(Fct f, double r1, double r2, Point xy,
                   int count, double xscale, double yscale)
// graph f(x) for x in [r1:r2) using count line segments with (0,0) displayed at xy
// x coordinates are scaled by xscale and y coordinates scaled by yscale
{
    if (r2-r1<=0) error("bad graphing range");
    if (count <=0) error("non-positive graphing count");
    double dist = (r2-r1)/count;
    double r = r1;
    for (int i = 0; i<count; ++i) {
        add(Point(xy.x+int(r*xscale),xy.y-int(f(r)*yscale)));
        r += dist;
    }
}

//------------------------------------------------------------------------------

bool can_open(const string& s)
// check if a file named s exists and can be opened for reading
{
    ifstream ff(s.c_str());
    return bool(ff);
}

//------------------------------------------------------------------------------

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

Suffix::Encoding get_encoding(const string& s)
{
    struct SuffixMap 
    { 
        const char*      extension;
        Suffix::Encoding suffix;
    };

    static SuffixMap smap[] = {
        {".jpg",  Suffix::jpg},
        {".jpeg", Suffix::jpg},
        {".gif",  Suffix::gif},
        {".png",  Suffix::png},
    };

    for (int i = 0, n = ARRAY_SIZE(smap); i < n; i++)
    {
        int len = strlen(smap[i].extension);

        if (s.length() >= len && s.substr(s.length()-len, len) == smap[i].extension)
            return smap[i].suffix;
    }

    return Suffix::none;
}

//------------------------------------------------------------------------------

// somewhat over-elaborate constructor
// because errors related to image files can be such a pain to debug
Image::Image(Point xy, string s, Suffix::Encoding e)
    :w(0), h(0), fn(xy,"")
{
    add(xy);

    if (!can_open(s)) {    // can we open s?
        fn.set_label("cannot open \""+s+'\"');
        p = new Bad_image(30,20);    // the "error image"
        return;
    }

    if (e == Suffix::none) e = get_encoding(s);

    switch(e) {        // check if it is a known encoding
    case Suffix::jpg:
        p = new Fl_JPEG_Image(s.c_str());
        break;
    case Suffix::gif:
        p = new Fl_GIF_Image(s.c_str());
        break;
    case Suffix::png:
        p = new Fl_PNG_Image(s.c_str());
        break;
    default:    // Unsupported image encoding
        fn.set_label("unsupported file type \""+s+'\"');
        p = new Bad_image(30,20);    // the "error image"
    }
}

//------------------------------------------------------------------------------

void Image::draw_lines() const
{
    if (fn.label()!="") fn.draw_lines();

    if (w&&h)
        p->draw(point(0).x,point(0).y,w,h,cx,cy);
    else
        p->draw(point(0).x,point(0).y);
}

//------------------------------------------------------------------------------

} // of namespace Graph_lib

/*
   Graph.h
   Minimally revised for C++11 features of GCC 4.6.3 or later
   Walter C. Daugherity		June 10, 2012 
   Walter C. Daugherity		January 9, 2014
   Walter C. Daugherity		January 20, 2014
   Walter C. Daugherity		March 3, 2014
   Walter C. Daugherity		March 6, 2014
*/

//
// This is a GUI support code to the chapters 12-16 of the book
// "Programming -- Principles and Practice Using C++" by Bjarne Stroustrup
//

#ifndef GRAPH_GUARD
#define GRAPH_GUARD 1

#include <FL/fl_draw.H>
#include <FL/Fl_Image.H>
#include "Point.h"
#include "../std_lib_facilities_4.h"

namespace Graph_lib {

// defense against ill-behaved Linux macros:
#undef major
#undef minor

//------------------------------------------------------------------------------

// Color is the type we use to represent color. We can use Color like this:
//    grid.set_color(Color::red);
struct Color {
    enum Color_type : unsigned char {
        red=FL_RED,
        blue=FL_BLUE,
        green=FL_GREEN,
        yellow=FL_YELLOW,
        white=FL_WHITE,
        black=FL_BLACK,
        magenta=FL_MAGENTA,
        cyan=FL_CYAN,
        dark_red=FL_DARK_RED,
        dark_green=FL_DARK_GREEN,
        dark_yellow=FL_DARK_YELLOW,
        dark_blue=FL_DARK_BLUE,
        dark_magenta=FL_DARK_MAGENTA,
        dark_cyan=FL_DARK_CYAN
    };

    enum Transparency : unsigned char { invisible = 0, visible=255 };

    Color(Color_type cc) :v(visible), c(Fl_Color(cc)) { }
    Color(Color_type cc, Transparency vv) :v(vv), c(Fl_Color(cc)) { }
    Color(int cc) :v(visible), c(Fl_Color(cc)) { }
    Color(Transparency vv) :v(vv), c(Fl_Color()) { }    // default color

    int as_int() const { return c; }

    char visibility() const { return v; } 
    void set_visibility(Transparency vv) { v=vv; }
private:
    unsigned char v;    // invisible and visible for now
    Fl_Color c;
};

//------------------------------------------------------------------------------

struct Line_style {
    enum Line_style_type {
        solid=FL_SOLID,            // -------
        dash=FL_DASH,              // - - - -
        dot=FL_DOT,                // ....... 
        dashdot=FL_DASHDOT,        // - . - . 
        dashdotdot=FL_DASHDOTDOT,  // -..-..
    };

    Line_style(Line_style_type ss) :s(ss), w(0) { }
    Line_style(Line_style_type lst, int ww) :s(lst), w(ww) { }
    Line_style(int ss) :s(ss), w(0) { }

    int width() const { return w; }
    int style() const { return s; }
private:
    int s;
    int w;
};

//------------------------------------------------------------------------------

class Font {
public:
    enum Font_type {
        helvetica=FL_HELVETICA,
        helvetica_bold=FL_HELVETICA_BOLD,
        helvetica_italic=FL_HELVETICA_ITALIC,
        helvetica_bold_italic=FL_HELVETICA_BOLD_ITALIC,
        courier=FL_COURIER,
        courier_bold=FL_COURIER_BOLD,
        courier_italic=FL_COURIER_ITALIC,
        courier_bold_italic=FL_COURIER_BOLD_ITALIC,
        times=FL_TIMES,
        times_bold=FL_TIMES_BOLD,
        times_italic=FL_TIMES_ITALIC,
        times_bold_italic=FL_TIMES_BOLD_ITALIC,
        symbol=FL_SYMBOL,
        screen=FL_SCREEN,
        screen_bold=FL_SCREEN_BOLD,
        zapf_dingbats=FL_ZAPF_DINGBATS
    };

    Font(Font_type ff) :f(ff) { }
    Font(int ff) :f(ff) { }

    int as_int() const { return f; }
private:
    int f;
};

//------------------------------------------------------------------------------

template<class T> class Vector_ref {
    vector<T*> v;
    vector<T*> owned;
public:
    Vector_ref() {}
    Vector_ref(T& a) { push_back(a); }
    Vector_ref(T& a, T& b);
    Vector_ref(T& a, T& b, T& c);
    Vector_ref(T* a, T* b = 0, T* c = 0, T* d = 0)
    {
        if (a) push_back(a);
        if (b) push_back(b);
        if (c) push_back(c);
        if (d) push_back(d);
    }

    ~Vector_ref() { for (int i=0; i<owned.size(); ++i) delete owned[i]; }

    void push_back(T& s) { v.push_back(&s); }
    void push_back(T* p) { v.push_back(p); owned.push_back(p); }

    T& operator[](int i) { return *v[i]; }
    const T& operator[](int i) const { return *v[i]; }

    int size() const { return v.size(); }

private:	// prevent copying
	Vector_ref(const Vector<T>&);
	Vector_ref& operator=(const Vector<T>&);
};

//------------------------------------------------------------------------------

typedef double Fct(double);

class Shape  {        // deals with color and style, and holds sequence of lines 
public:
    void draw() const;                 // deal with color and draw lines
    virtual void move(int dx, int dy); // move the shape +=dx and +=dy

    void set_color(Color col) { lcolor = col; }
    Color color() const { return lcolor; }
    void set_style(Line_style sty) { ls = sty; }
    Line_style style() const { return ls; }
    void set_fill_color(Color col) { fcolor = col; }
    Color fill_color() const { return fcolor; }

    Point point(int i) const { return points[i]; } // read only access to points
    int number_of_points() const { return int(points.size()); }

    virtual ~Shape() { }
protected:
    Shape();    
    virtual void draw_lines() const;   // draw the appropriate lines
    void add(Point p);                 // add p to points
    void set_point(int i,Point p);     // points[i]=p;
private:
    vector<Point> points;              // not used by all shapes
    Color lcolor;                      // color for lines and characters
    Line_style ls; 
    Color fcolor;                      // fill color

    Shape(const Shape&);               // prevent copying
    Shape& operator=(const Shape&);
};

//------------------------------------------------------------------------------

struct Function : Shape {
    // the function parameters are not stored
    Function(Fct f, double r1, double r2, Point orig,
        int count = 100, double xscale = 25, double yscale = 25);    
};

//------------------------------------------------------------------------------

struct Line : Shape {            // a Line is a Shape defined by two Points
    Line(Point p1, Point p2);    // construct a line from two points
};

//------------------------------------------------------------------------------

struct Rectangle : Shape {

    Rectangle(Point xy, int ww, int hh) : h(hh), w(ww)
    {
        add(xy);
        if (h<=0 || w<=0) error("Bad rectangle: non-positive side");
    }

    Rectangle(Point x, Point y) : h(y.y-x.y), w(y.x-x.x)
    {
        add(x);
        if (h<=0 || w<=0) error("Bad rectangle: non-positive width or height");
    }
    void draw_lines() const;

    int height() const { return h; }
    int width() const { return w; }
private:
    int h;    // height
    int w;    // width
};

//------------------------------------------------------------------------------

struct Open_polyline : Shape {         // open sequence of lines
    void add(Point p) { Shape::add(p); }
    void draw_lines() const;
};

//------------------------------------------------------------------------------

struct Closed_polyline : Open_polyline { // closed sequence of lines
    void draw_lines() const;
};

//------------------------------------------------------------------------------

struct Polygon : Closed_polyline {    // closed sequence of non-intersecting lines
    void add(Point p);
    void draw_lines() const;
};

//------------------------------------------------------------------------------

struct Lines : Shape {                 // related lines
    void draw_lines() const;
    void add(Point p1, Point p2);      // add a line defined by two points
};

//------------------------------------------------------------------------------

struct Text : Shape {
    // the point is the bottom left of the first letter
    Text(Point x, const string& s) : lab(s), fnt(fl_font()),
    	fnt_sz((fl_size()<14)?14:fl_size()) { add(x); }

    void draw_lines() const;

    void set_label(const string& s) { lab = s; }
    string label() const { return lab; }

    void set_font(Font f) { fnt = f; }
    Font font() const { return Font(fnt); }

    void set_font_size(int s) { fnt_sz = s; }
    int font_size() const { return fnt_sz; }
private:
    string lab;    // label
    Font fnt;
    int fnt_sz;
};

//------------------------------------------------------------------------------

struct Axis : Shape {
    enum Orientation { x, y, z };
    Axis(Orientation d, Point xy, int length,
        int number_of_notches=0, string label = "");

    void draw_lines() const;
    void move(int dx, int dy);
    void set_color(Color c);

    Text label;
    Lines notches;
};

//------------------------------------------------------------------------------


struct Circle : Shape {
	Circle(Point p, int rr)	// center and radius
		:r(rr) { add(Point(p.x-r,p.y-r)); }

	void draw_lines() const;

	Point center() const;

        void moveto(Point p) {set_point(0,Point(p.x-r,p.y-r));}

	void set_radius(int rr) { set_point(0,Point(center().x-rr,center().y-rr)); r=rr;  }
	int radius() const { return r; }
private:
	int r;
};

//------------------------------------------------------------------------------

struct Ellipse : Shape {
	Ellipse(Point p, int ww, int hh)	// center, min, and max distance from center
		:w(ww), h(hh) { add(Point(p.x-ww,p.y-hh)); }

	void draw_lines() const;

	Point center() const { return Point(point(0).x+w,point(0).y+h); }
	Point focus1() const {
			if (h<=w)// foci are on the x-axis:
					return Point(center().x+int(sqrt(double(w*w-h*h))),center().y);
			else	// foci are on the y-axis:
					return Point(center().x,center().y+int(sqrt(double(h*h-w*w))));
	}

	Point focus2() const {
			if (h<=w)
					return Point(center().x-int(sqrt(double(w*w-h*h))),center().y);
			else
					return Point(center().x,center().y-int(sqrt(double(h*h-w*w))));
	}
	//Point focus2() const { return Point(center().x-int(sqrt(double(abs(w*w-h*h)))),center().y); }
	
	void set_major(int ww) { set_point(0,Point(center().x-ww,center().y-h)); w=ww; }
	int major() const { return w; }
	void set_minor(int hh) { set_point(0,Point(center().x-w,center().y-hh)); h=hh; }
	int minor() const { return h; }
private:
	int w;
	int h;
};

//------------------------------------------------------------------------------

struct Marked_polyline : Open_polyline {
    Marked_polyline(const string& m) :mark(m) { }
    void draw_lines() const;
private:
    string mark;
};

//------------------------------------------------------------------------------

struct Marks : Marked_polyline {
    Marks(const string& m) :Marked_polyline(m)
    {
        set_color(Color(Color::invisible));
    }
};

//------------------------------------------------------------------------------

struct Mark : Marks {
    Mark(Point xy, char c) : Marks(string(1,c))
    {
        add(xy);
    }
};

//------------------------------------------------------------------------------

struct Suffix {
    enum Encoding { none, jpg, gif, png  };
};

Suffix::Encoding get_encoding(const string& s);

//------------------------------------------------------------------------------

struct Image : Shape {
    Image(Point xy, string file_name, Suffix::Encoding e = Suffix::none);
    ~Image() { delete p; }
    void draw_lines() const;
    void set_mask(Point xy, int ww, int hh) { w=ww; h=hh; cx=xy.x; cy=xy.y; }
private:
    int w,h;  // define "masking box" within image relative to position (cx,cy)
    int cx,cy; 
    Fl_Image* p;
    Text fn;
};

//------------------------------------------------------------------------------

struct Bad_image : Fl_Image {
    Bad_image(int h, int w) : Fl_Image(h,w,0) { }
    void draw(int x,int y, int, int, int, int) { draw_empty(x,y); }
};

//------------------------------------------------------------------------------

} // of namespace Graph_lib

#endif

//
// This is a GUI support code to the chapters 12-16 of the book
// "Programming -- Principles and Practice Using C++" by Bjarne Stroustrup
//

#ifndef POINT_GUARD
#define POINT_GUARD

//------------------------------------------------------------------------------

struct Point {
    int x, y;
    Point(int xx, int yy) : x(xx), y(yy) { }
    Point() :x(0), y(0) { }
};

//------------------------------------------------------------------------------

inline bool operator==(Point a, Point b) { return a.x==b.x && a.y==b.y; } 

//------------------------------------------------------------------------------

inline bool operator!=(Point a, Point b) { return !(a==b); }

//------------------------------------------------------------------------------

#endif // POINT_GUARD

/*
   Simple_window.cpp
   Minimally revised for C++11 features of GCC 4.6.3 or later
   Walter C. Daugherity		June 10, 2012
*/

//
// This is a GUI support code to the chapters 12-16 of the book
// "Programming -- Principles and Practice Using C++" by Bjarne Stroustrup
//

#include "Simple_window.h"

using namespace Graph_lib;

//------------------------------------------------------------------------------

Simple_window::Simple_window(Point xy, int w, int h, const string& title) :
    Window(xy,w,h,title),
    next_button(Point(x_max()-70,0), 70, 20, "Next", cb_next),
    button_pushed(false)
{
    attach(next_button);
}

//------------------------------------------------------------------------------

bool Simple_window::wait_for_button()
// modified event loop:
// handle all events (as per default), quit when button_pushed becomes true
// this allows graphics without control inversion
{
    show();
    button_pushed = false;
#if 1
    // Simpler handler
    while (!button_pushed) Fl::wait();
    Fl::redraw();
#else
    // To handle the case where the user presses the X button in the window frame
    // to kill the application, change the condition to 0 to enable this branch.
    Fl::run();
#endif
    return button_pushed;
}

//------------------------------------------------------------------------------

void Simple_window::cb_next(Address, Address pw)
// call Simple_window::next() for the window located at pw
{  
    reference_to<Simple_window>(pw).next();    
}

//------------------------------------------------------------------------------

void Simple_window::next()
{
    button_pushed = true;
    hide();
}

//------------------------------------------------------------------------------

//
// This is a GUI support code to the chapters 12-16 of the book
// "Programming -- Principles and Practice Using C++" by Bjarne Stroustrup
//

#ifndef SIMPLE_WINDOW_GUARD
#define SIMPLE_WINDOW_GUARD 1

#include "GUI.h"    // for Simple_window only (doesn't really belong in Window.h)
#include "Graph.h"

using namespace Graph_lib;

//------------------------------------------------------------------------------

struct Simple_window : Graph_lib::Window {
    Simple_window(Point xy, int w, int h, const string& title );

    bool wait_for_button(); // simple event loop

private:
    Button next_button;     // the "next" button
    bool button_pushed;     // implementation detail

    static void cb_next(Address, Address); // callback for next_button
    void next();            // action to be done when next_button is pressed

};

//------------------------------------------------------------------------------

#endif // SIMPLE_WINDOW_GUARD

//
// This is a GUI support code to the chapters 12-16 of the book
// "Programming -- Principles and Practice Using C++" by Bjarne Stroustrup
//

#include "Window.h"
#include "Graph.h"
#include "GUI.h"

//------------------------------------------------------------------------------

namespace Graph_lib {

Window::Window(int ww, int hh, const string& title)
    :Fl_Window(ww,hh,title.c_str()),w(ww),h(hh)
{
    init();
}

//------------------------------------------------------------------------------

Window::Window(Point xy, int ww, int hh, const string& title)
    :Fl_Window(xy.x,xy.y,ww,hh,title.c_str()),w(ww),h(hh)
{ 
    init();
}

//------------------------------------------------------------------------------

void Window::init()
{
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

/*
   Window.h
   Minimally revised for C++11 features of GCC 4.6.3 or later
   Walter C. Daugherity		June 10, 2012 and January 9, 2014
*/

//
// This is a GUI support code to the chapters 12-16 of the book
// "Programming -- Principles and Practice Using C++" by Bjarne Stroustrup
//

#ifndef WINDOW_GUARD
#define WINDOW_GUARD

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include "../std_lib_facilities_4.h"
#include "Point.h"

namespace Graph_lib
{
    class Shape;   // "forward declare" Shape
    class Widget;

//------------------------------------------------------------------------------

    class Window : public Fl_Window { 
    public:
        // let the system pick the location:
        Window(int w, int h, const string& title);
        // top left corner in xy
        Window(Point xy, int w, int h, const string& title);    

        virtual ~Window() { }

        int x_max() const { return w; }
        int y_max() const { return h; }

        void resize(int ww, int hh) { w=ww, h=hh; size(ww,hh); }

        void set_label(const string& s) { copy_label(s.c_str()); }

        void attach(Widget& w);
        void attach(Shape& s) { shapes.push_back(&s); }

        void detach(Widget& w);    // remove w from window (deactivates callbacks)
        void detach(Shape& s);     // remove s from shapes 

        void put_on_top(Shape& p); // put p on top of other shapes

    protected:
        void draw();

    private:
        vector<Shape*> shapes;     // shapes attached to window
        vector<Widget*> widgets;     // widgets attached to window
        int w,h;                   // window size

        void init();
    };

//------------------------------------------------------------------------------

           int gui_main(); // invoke GUI library's main event loop
    inline int x_max() { return Fl::w(); } // width of screen in pixels
    inline int y_max() { return Fl::h(); } // height of screen in pixels

} // of namespace Graph_lib

#endif // WINDOW_GUARD

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
	
	#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS	//temp
/*
   std_lib_facilities_4.h
   Minimally revised for C++11 features of GCC 4.6.3 or later
   Walter C. Daugherity		June 10, 2012 and January 9, 2014
*/

/*
	simple "Programming: Principles and Practice using C++" course header to
	be used for the first few weeks.
	It provides the most common standard headers (in the global namespace)
	and minimal exception/error support.
	Students: please don't try to understand the details of headers just yet.
	All will be explained. This header is primarily used so that you don't have
	to understand every concept all at once.
	Revised April 25, 2010: simple_error() added
*/

#ifndef H112
#define H112 201401L

#define GCC_VERSION (__GNUC__*10000 + __GNUC_MINOR__*100 + __GNUC_PATCHLEVEL__)

#if GCC_VERSION >= 40603
//New C++11 headers in GCC 4.6.3 or later
#include <array>
#include <regex>
#include <thread>
#include <mutex>
#include <forward_list>
#include <ratio>
#include <tuple>
#include <chrono>
#include <random>
#endif

#include<iostream>
#include<fstream>
#include<sstream>
#include<cmath>
#include<cstdlib>
#include<string>
#include<list>
#include<vector>
#include<algorithm>
#include<stdexcept>

//------------------------------------------------------------------------------

#if GCC_VERSION >= 40603
#include <unordered_map>
#include <unordered_set>
#else
#define unordered_map hash_map
#ifdef _MSC_VER
#include <hash_map>
using stdext::hash_map;
#else
#include <ext/hash_map>
using __gnu_cxx::hash_map;

namespace __gnu_cxx {

    template<> struct hash<std::string>
    {
        size_t operator()(const std::string& s) const
        {
            return hash<char*>()(s.c_str());
        }
    };

} // of namespace __gnu_cxx
#endif //_MSC_VER
#endif //GCC_VERSION >= 40603

//------------------------------------------------------------------------------

typedef long Unicode;

//------------------------------------------------------------------------------

using namespace std;

template<class T> string to_string(const T& t)
{
	ostringstream os;
	os << t;
	return os.str();
}

struct Range_error : out_of_range {	// enhanced vector range error reporting
	int index;
	Range_error(int i) :out_of_range("Range error: "+to_string(i)), index(i) { }
};


// trivially range-checked vector (no iterator checking):
template< class T> struct Vector : public std::vector<T> {
	typedef typename std::vector<T>::size_type size_type;

	Vector() { }
	explicit Vector(size_type n) :std::vector<T>(n) {}
	Vector(size_type n, const T& v) :std::vector<T>(n,v) {}
	template <class I>
	Vector(I first, I last) :std::vector<T>(first,last) {}
#if GCC_VERSION >= 40603
	Vector(initializer_list<T> list) :std::vector<T>(list) {}
#endif

	T& operator[](unsigned int i) // rather than return at(i);
	{
		if (this->size()<=i) throw Range_error(i);
		return std::vector<T>::operator[](i);
	}
	const T& operator[](unsigned int i) const
	{
		if (this->size()<=i) throw Range_error(i);
		return std::vector<T>::operator[](i);
	}
};

// disgusting macro hack to get a range checked vector:
#define vector Vector

// trivially range-checked string (no iterator checking):
struct String : std::string {
	
	String() { }
	String(const char* p) :std::string(p) {}
	String(const string& s) :std::string(s) {}
	template<class S> String(S s) :std::string(s) {}
	String(int sz, char val) :std::string(sz,val) {}
	template<class Iter> String(Iter p1, Iter p2) : std::string(p1,p2) { }

	char& operator[](unsigned int i) // rather than return at(i);
	{
		if (size()<=i) throw Range_error(i);
		return std::string::operator[](i);
	}

	const char& operator[](unsigned int i) const
	{
		if (size()<=i) throw Range_error(i);
		return std::string::operator[](i);
	}
};

#ifndef _MSC_VER
#if GCC_VERSION >= 40603
namespace std {

    template<> struct hash<String>
    {
        size_t operator()(const String& s) const
        {
            return hash<std::string>()(s);
        }
    };

} // of namespace std
#else
namespace __gnu_cxx {

    template<> struct hash<String>
    {
        size_t operator()(const String& s) const
        {
            return hash<std::string>()(s);
        }
    };

} // of namespace __gnu_cxx
#endif //GCC_VERSION >= 40603
#endif //_MSC_VER


struct Exit : runtime_error {
	Exit(): runtime_error("Exit") {}
};

// error() simply disguises throws:
inline void error(const string& s)
{
	throw runtime_error(s);
}

inline void error(const string& s, const string& s2)
{
	error(s+s2);
}

inline void error(const string& s, int i)
{
	ostringstream os;
	os << s <<": " << i;
	error(os.str());
}

#if _MSC_VER<1500
	// disgusting macro hack to get a range checked string:
	#define string String
	// MS C++ 9.0 have a built-in assert for string range check
	// and uses "std::string" in several places so that macro substitution fails
#endif

template<class T> char* as_bytes(T& i)	// needed for binary I/O
{
	void* addr = &i;	// get the address of the first byte
						// of memory used to store the object
	return static_cast<char*>(addr); // treat that memory as bytes
}


inline void keep_window_open()
{
	cin.clear();
	cout << "Please enter a character to exit\n";
	char ch;
	cin >> ch;
	return;
}

inline void keep_window_open(string s)
{
	if (s=="") return;
	cin.clear();
	cin.ignore(120,'\n');
	for (;;) {
		cout << "Please enter " << s << " to exit\n";
		string ss;
		while (cin >> ss && ss!=s)
			cout << "Please enter " << s << " to exit\n";
		return;
	}
}



// error function to be used (only) until error() is introduced in Chapter 5:
inline void simple_error(string s)	// write ``error: s and exit program
{
	cerr << "error: " << s << '\n';
	keep_window_open();		// for some Windows environments
	exit(1);
}

// make std::min() and std::max() accessible:
#undef min
#undef max

#include<iomanip>
inline ios_base& general(ios_base& b)	// to augment fixed and scientific
{
	b.setf(ios_base::fmtflags(0),ios_base::floatfield);
	return b;
}

// run-time checked narrowing cast (type conversion):
template<class R, class A> R narrow_cast(const A& a)
{
	R r = R(a);
	if (A(r)!=a) error(string("info loss"));
	return r;
}


inline int randint(int max) { return rand()%max; }

inline int randint(int min, int max) { return randint(max-min)+min; }

inline double sqrt(int x) { return sqrt(double(x)); }	// to match C++0x

#endif //H112

Bibliography:

No references used.




