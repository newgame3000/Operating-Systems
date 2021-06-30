#include <ncurses.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include "game.hpp"

using namespace std;

int main(){
	setlocale(LC_ALL, "");
	initscr();
	raw();
	noecho();

	WINDOW* menu = newwin(LINES, COLS, 0, 0);
	vector < string > menu_string = {
		"Создать игру",
		"Присоединиться к игре",
		"Выход"
	};

	keypad(menu, TRUE);
	curs_set(0);

	refresh();

	box(menu,0,0);

	start_color();
 	init_pair(1, COLOR_WHITE,  COLOR_BLUE);

	char game_name[] = "ИГРА ТРОН";
    int id = 0;
    int input;
    bool game_play = true;
    while(game_play) {

    	wattron(menu, A_BLINK | A_BOLD | COLOR_PAIR(1));
    	mvwaddstr(menu, LINES/2 - 5, COLS/2 - 4, game_name);

    	for (int i = 0; i < 3; ++i) {
    		if (i == id) {
				attron(A_STANDOUT);
			}
			mvprintw(LINES/2 + i, COLS / 2 - 10, menu_string[i].c_str());
			attroff(A_STANDOUT);
    	}

    	wrefresh(menu);
    	refresh();

    	input = wgetch(menu);

    		switch (input) {
        		case KEY_UP:
          			if (id  != 0) {
          				id -=1;
          			}
          		break;
            	case KEY_DOWN:  
          			if (id != 2) {
          				id += 1;
          			} 
          		break;
          		case 10:

          			if (id == 2) {
          				game_play = false;
          			}

          			if (id == 1) {

          				WINDOW* ip_win = newwin(LINES,COLS,0,0);
          				char question1[] = "Введите ip";
          				mvwaddstr(ip_win, LINES/2, COLS/2 - 4, question1);
          				string s2;
          				input = 0;
          				int t = -3;
          				int end = 0;
          				echo();
          				while (input != 10) {

          					if (input == 27) {
          						s2 = "";
          						end  = 1;
          						break;
          					}

          					input = mvwgetch(ip_win, LINES/2 + 1, COLS/2 + t);
          					if (input != 10) {
          						s2 += (char)input;
          						t += 1;
          					}

          					wrefresh(ip_win);
          				}
          				noecho();
          				clear();
          				delwin(ip_win);

          				if (end == 0) {
	          				WINDOW* port_win = newwin(LINES,COLS,0,0);
	          				char question[] = "Введите порт";
	          				mvwaddstr(port_win, LINES/2, COLS/2 - 4, question);
	       					input = 0;
	          				string s = "";
	          				echo();
	          				t = 0;
	          				while (input != 10) {

	          					if (input == 27) {
	          						s = "";
	          						break;
	          					}


	          					input = mvwgetch(port_win, LINES/2 + 1, COLS/2 + t);
	          					if (input != 10) {
	          						s += (char)input;
	          						t += 1;
	          					}
	          				}

	          				if (s != "" && s2 != "") {
	          					//mvwaddstr(port_win, LINES/3, COLS/2 - 4, s.c_str());
	          					// wrefresh(port_win);
	          					// sleep(10);
	          					game newgame(false, s, s2);
	          					newgame.play();
	          				}

	         				clear();
	          				noecho();
	          				delwin(port_win);
	          			}
          			}

          			if (id == 0) {
          				WINDOW* port_win = newwin(LINES,COLS,0,0);
          				char question[] = "Введите порт";
          				mvwaddstr(port_win, LINES/2, COLS/2 - 4, question);
          				input = 0;
          				string s = "";
          				echo();
          				int t = 0;
          				while (input != 10) {

          					if (input == 27) {
          						s = "";
          						break;
          					}

          					input = mvwgetch(port_win, LINES/2 + 1, COLS/2 + t);
          					if (input != 10) {
          						s += (char)input;
          						t += 1;
          					}
          				}

          				if (s != "") {
          					game newgame(true, s, "");
          					newgame.play();
          				}

         				clear();
          				noecho();
          				delwin(port_win);
          			}
          		break;
        	}

    	wrefresh(menu);
    	refresh();
    	// if (getch()) {
    	// 	delwin(menu);
    	// 	break;
    	// }
    }
	
	endwin();
}