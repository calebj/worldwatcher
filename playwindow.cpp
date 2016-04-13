#include "std_lib_facilities_4.h"
#include "Graph.h"
#include "Window.h"
#include "GUI.h"
#include "board.cpp"

using namespace Graph_lib;

struct play_window : Graph_lib::Window 
{
    play_window(Point xy, int w, int h, const string& title2);
    Vector_ref<Rectangle> board;
    Vector_ref<Text> num;
    //Vector_ref<Text> chars;
	void initiate();


private:
    bool wait_for_button();
    bool button_pushed;
	vector<Board> subboard;


	bool turn = true; //true = player 1, false = player 2
	int next_board = -1;
	
	Button quit_button;
	Button nextpoint_button;
	int coordinate_to_board(int x, int y);
	int coordinate_to_subboard(int x, int y);
	bool valid_coords();
	static void cb_quit(Address, Address window); 
	void quit();
	static void cb_nextpoint(Address, Address window);
	void nextpoint();
	void coord_error(bool e);
	In_box next_x;
	In_box next_y;
	Out_box player_turn;
};

void play_window::initiate(){
	for(int i = 20; i <= 580; i += 70)
	{
		for(int j = 70; j <= 630; j += 70)
		{
			board.push_back(new Rectangle(Point(j,i),70,70));
		}
	}
	Text nine(Point(35,605), "9");
	Text eight(Point(35,535), "8");
	Text seven(Point(35,465), "7");
	Text six(Point(35,395), "6");
	Text five(Point(35,325), "5");
	Text four(Point(35,255), "4");
	Text three(Point(35,185), "3");
	Text two(Point(35,115), "2");
	Text one(Point(35,45), "1");
	for(int i = 0; i < num.size(); i++){
		num[i].set_font_size(35);
	}
	
	for (int i = 0; i < 9; i++)
	{
		subboard.push_back(*(new Board()));
	}

	for(int i = 0; i < board.size(); i++){
		board[i].set_color(Color::black);
		if(i/27 == 0){
			if((i/3)%3 == 0){		
				board[i].set_fill_color(Color::yellow);
			}
			else if((i/3)%3 == 1){
				board[i].set_fill_color(Color::green);
			}
			else if((i/3)%3 == 2){
				board[i].set_fill_color(Color::red);
			}
		}
		else if(i/27 == 1){
			if((i/3)%3 == 0){		
				board[i].set_fill_color(Color::red);
			}
			else if((i/3)%3 == 1){
				board[i].set_fill_color(Color::yellow);
			}
			else if((i/3)%3 == 2){
				board[i].set_fill_color(Color::green);
			}
		}
		else{
			if((i/3)%3 == 0){		
				board[i].set_fill_color(Color::green);
			}
			else if((i/3)%3 == 1){
				board[i].set_fill_color(Color::red);
			}
			else if((i/3)%3 == 2){
				board[i].set_fill_color(Color::yellow);
			}
		}
	}
	
}

bool play_window::wait_for_button(){
	show();
    button_pushed = false;
    while (!button_pushed) Fl::wait();
    Fl::redraw();
    return button_pushed;
}


void play_window::cb_quit(Address, Address pw)
	{
		reference_to<play_window>(pw).quit();
	} 

void play_window::quit(){
	Fl_Window::hide();
}

void play_window::cb_nextpoint(Address, Address pw){
	reference_to<play_window>(pw).nextpoint();
}

int play_window::coordinate_to_board(int x, int y)
{
	x=x/3;
	y=y/3;
	int num=0;
	num+=x;
	num+=(3*y);
	// cout<<"C to bor"<<num<<endl;
	return num;
}

int play_window::coordinate_to_subboard(int x, int y)
{
	int num=0;
	num+=x;
	num+=(3*y);
	// cout<<"C to sub"<<num<<endl;
	return num;
}

bool play_window::valid_coords()
{
	try
	{
		int x=next_x.get_int();
		int y=next_y.get_int();
		if (next_board==0||next_board==3||next_board==6)
		{
			if (x<1||x>3) return false;
		}
		else if (next_board==1||next_board==4||next_board==7)
		{
			if (x<4||x>6) return false;
		}
		else if (next_board==2||next_board==5||next_board==8)
		{
			if (x<7||x>9) return false;
		}
		if (next_board==0||next_board==1||next_board==2)
		{
			if (y<1||y>3) return false;
		}
		else if (next_board==3||next_board==4||next_board==5)
		{
			if (y<4||y>6) return false;
		}
		else if (next_board==6||next_board==7||next_board==8)
		{
			if (y<7||y>9) return false;
		}
		if (next_board==-1)
		{
			if ((x<1||x>9)||(y<1||y>9)) return false;
		}
		return true;
	}
	catch(...)
	{
		return false;
	}
}

void play_window::coord_error(bool e)//error message at top of screen should show when true and dissapear when false
{
	
}

void play_window::nextpoint(){ //grab coordinates and draw. change turn.
	Fl::wait();
	
	try
	{
		if (valid_coords())
		{
			char symbol;
			if (turn)
			{
				symbol = 'x';
			}
			else
			{
				symbol = 'o';
			}
			int x = next_x.get_int()-1;
			int y = next_y.get_int()-1;
			int x_sub = x%3;
			int y_sub = y%3;
			int s = coordinate_to_board(x,y);
			int s_sub = coordinate_to_subboard(x_sub,y_sub);
			if (subboard[s].set_choice(s_sub,symbol))
			{
				next_board=s_sub;
				coord_error(false);
				
				//
				//place box around next board
				
				//			
				//place x or o
				
				//
				if (subboard[s].calc_win())// if the current player won the game, do stuff
				{
					/////////////////////////////
				}
//testing code
for (int i = 0; i < subboard.size(); i++)
{
	cout<<i<<endl;
	for (int j = 0; j < 9; j++)
	{
		cout<<subboard[i].getChoice(j);
		if (j%3==2) cout<<endl;
	}
}
				if (turn) //swaps turn
				{
					turn = false;
					player_turn.put("player 2 - O");
				}
				else
				{
					turn = true;
					player_turn.put("player 1 - X");
				}
			}
			else
			{
				coord_error(true);
			}
		}
		else //put out error message
		{
			coord_error(true);
		}
	}
	catch (...) {}
}

play_window::play_window(Point xy, int w, int h, const string& title2)
	:Window(xy,w,h,title2),
	player_turn(Point(110,0), 100, 20, "Turn: "),
	quit_button(Point(0,0), 70, 20, "Quit", cb_quit),
	next_x(Point(x_max()-310,0), 50, 20, "next x: "),
	next_y(Point(x_max()-210,0), 50, 20, "next y: "),
	nextpoint_button(Point(x_max()-100,0), 70, 20, "next", cb_nextpoint)
	{
		attach(quit_button);
		initiate();
		
		
		attach(nextpoint_button);
		attach(next_x);
		attach(next_y);
		attach(player_turn);
		player_turn.put("player 1 - X");
		for(int i = 0; i < num.size(); i++){
			attach(num[i]);
		}
	}