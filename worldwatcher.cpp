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
            cout << pad << "\n";
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
        while(initials != 0 && strlen(initials) > 4)
            initials = (char*)fl_input("That's too long; please enter up to four characters.");
        if (initials != 0) {
            scores.push_back(Score{score, initials});
            write_highscores(scores);
            set_action(3);
        } else
            set_action(4);
    } else {
        if(!manual) {
            if(fl_ask("Game over! Play again?"))
                set_action(1);
            else
                set_action(4);
        }
    }
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
    time_remaining -= 5; // penalty
    update_sideinfo();
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
