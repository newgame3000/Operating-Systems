#include <ncurses.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <time.h> 
#include <unistd.h>
#include <zmqpp/zmqpp.hpp>

using namespace std;
using namespace zmqpp;

struct mat_rix
{
	int value = 0;
	int color = 0;
};

struct player {
	int x = 0;
	int y = 0;
	int direction; //0 - вниз, 1 - вверх, 2 - влево, 3 - вправо
	int color;
};

struct coord {
	int x;
	int y;
};


class game {
public:

	bool dealer;
	player player_1;
	vector <coord> pl_1_way;
	WINDOW* win_game;
	context context_1;
	socket socket_1 =  socket(context_1, socket_type::push);
	socket socket_2 =  socket(context_1, socket_type::push);
	vector <vector <mat_rix>> matrix;
	int indificator_player = 2;
	int pl_col = 2;
	int q = 0;

	void generation_player(player& player_1, vector<vector<mat_rix>> & matrix, vector<coord> & pl_1_way,int sx, int sy) {
		
		player_1.x = rand() % sx;
		player_1.y = rand() % sy;
		player_1.direction = rand() % 4;

		while (matrix[player_1.x][player_1.y].value != 0) {
			player_1.x = rand() % COLS;
			player_1.y = rand() % LINES;
			player_1.direction = rand() % 4;
		}

		if (player_1.direction == 0) {
			mvwprintw(win_game,player_1.y, player_1.x, "ᐯ");
			matrix[player_1.x][player_1.y].value = 3;
		}

		if (player_1.direction == 1) {
			mvwprintw(win_game,player_1.y, player_1.x, "ᐱ");
			matrix[player_1.x][player_1.y].value = 2;
		}

		if (player_1.direction == 2) {
			mvwprintw(win_game,player_1.y, player_1.x, "<");
			matrix[player_1.x][player_1.y].value = 4;
		}

		if (player_1.direction == 3) {
			mvwprintw(win_game,player_1.y, player_1.x, ">");
			matrix[player_1.x][player_1.y].value = 5;
		}

		matrix[player_1.x][player_1.y].color = player_1.color;
		pl_1_way.push_back({player_1.x, player_1.y});
	}

	game(bool d, string port, string s2) {

		dealer = d;

		int size_x;
		int size_y;

		int port2 = stoi(port);
		port2 += 1;
		string port_2 = to_string(port2);

		init_pair(2, COLOR_BLUE, COLOR_BLACK);
		init_pair(3, COLOR_YELLOW, COLOR_BLACK);
		init_pair(4, COLOR_GREEN, COLOR_BLACK);
		init_pair(5, COLOR_RED, COLOR_BLACK);
	
		if (dealer) {

			port = string("tcp://127.0.0.1:") + port;
			port_2 = string("tcp://127.0.0.1:") + port_2;

			socket_1 = socket(context_1, socket_type::pub);
			socket_2 = socket(context_1, socket_type::pull);

			socket_2.set(zmqpp::socket_option::receive_timeout, 1000);

			socket_1.bind(port.c_str());
			socket_2.bind(port_2.c_str());

			size_x = COLS + 1;
			size_y = LINES + 1;

			win_game = newwin(size_y,size_x,0,0);
			keypad(win_game, TRUE);
			nodelay(win_game, TRUE);
			start_color();
			noecho();
			curs_set(0);
			wrefresh(win_game);
			srand(time(NULL));

			for (int i = 0; i < size_x; ++i) {
				matrix.push_back(vector<mat_rix>());
				for (int j = 0; j< size_y; ++j) {
					matrix[i].push_back({0,0});
				}
			} //0 - пусто, 1 - дорога, 2 - стрелочка вверх
			  //3 - вниз, 4 - влево, 5 - вправо
			player_1.color = pl_col;
			wattron(win_game,COLOR_PAIR(player_1.color));
			generation_player(player_1, matrix, pl_1_way, COLS, LINES);
			wattron(win_game,COLOR_PAIR(player_1.color));

		} else {

			port = string("tcp://") + s2 + string(":") + port;
			port_2 = string("tcp://") + s2 + string(":") + port_2;


			socket_1 = socket(context_1, socket_type::sub);
			socket_2 = socket(context_1, socket_type::push);

			socket_1.set(socket_option::subscribe,"");

			socket_1.connect(port.c_str());
			socket_2.connect(port_2.c_str());
			socket_1.set(zmqpp::socket_option::receive_timeout, 1000);

			//Говорим серверу, что мы подключились, и ждём от него индификатор и матрицу состояния игры

			message mes;
			mes << "alive";
			if(!socket_2.send(mes)) {
				exit(-999);
			}

			if(!socket_1.receive(mes)) {
				exit(0);
			} 

			string patch;
			mes >> patch;

			while (patch != "new_player") {

				if(!socket_1.receive(mes)) {
					
				}

				mes >> patch;

			}

			if (patch == "new_player") {

				mes >> indificator_player;
				mes >> size_x >> size_y;
				for (int i = 0; i < size_x; ++i) {
					matrix.push_back(vector<mat_rix>());
					for (int j = 0; j < size_y; ++j) {
						int f;
						int f2;
						mes >> f;
						mes >> f2;
						matrix[i].push_back({f,f2});
					}
				}
			}

			mes = message();
			win_game = newwin(size_y,size_x,0,0);
			keypad(win_game, TRUE);
			nodelay(win_game, TRUE);
			start_color();
			noecho();
			curs_set(0);
			wrefresh(win_game);
			srand(time(NULL));

			for (int i = 0; i < size_x; ++i) {
				for (int j = 0; j < size_y; ++j) {

					if (matrix[i][j].value == 0) {
						continue;
					}

					if (matrix[i][j].value == 1) {
						wattron(win_game,COLOR_PAIR(matrix[i][j].color));
						mvwprintw(win_game,j, i, "0");
						wattron(win_game,COLOR_PAIR(matrix[i][j].color));
					}

					if (matrix[i][j].value == 2) {
						wattron(win_game,COLOR_PAIR(matrix[i][j].color));
						mvwprintw(win_game,j, i, "ᐱ");
						wattron(win_game,COLOR_PAIR(matrix[i][j].color));
					}

					if (matrix[i][j].value == 3) {
						wattron(win_game,COLOR_PAIR(matrix[i][j].color));
						mvwprintw(win_game,j, i, "ᐯ");
						wattron(win_game,COLOR_PAIR(matrix[i][j].color));
					}

					if (matrix[i][j].value == 4) {
						wattron(win_game,COLOR_PAIR(matrix[i][j].color));
						mvwprintw(win_game,j, i, "<");
						wattron(win_game,COLOR_PAIR(matrix[i][j].color));
					}

					if (matrix[i][j].value == 5) {
						wattron(win_game,COLOR_PAIR(matrix[i][j].color));
						mvwprintw(win_game,j, i, ">");
						wattron(win_game,COLOR_PAIR(matrix[i][j].color));
					}

				}
			}

			player_1.color = indificator_player;
			wattron(win_game,COLOR_PAIR(player_1.color));
			generation_player(player_1, matrix, pl_1_way, size_x - 1, size_y - 1);
			wattroff(win_game,COLOR_PAIR(player_1.color));
			mes = message();
			mes << "start_player" << player_1.x << player_1.y << player_1.color << player_1.direction;
			socket_2.send(mes);
			mes = message();
			wrefresh(win_game);
		}
	}


	void clear(vector<vector<mat_rix>> & m, vector<coord> & c) {
		for (int i = 0; i < c.size(); ++i) {
			m[c[i].x][c[i].y].value = 0;
			m[c[i].x][c[i].y].color = 0;
			mvwprintw(win_game,c[i].y, c[i].x, " ");
		}
		c.clear();
	}

	void play() {

		vector<bool> colors_pl(6);
		colors_pl[0] = true;
		colors_pl[1] = true;
		colors_pl[2] = true;
		for (int i = 3; i < 6; ++i) {
			colors_pl[i] = false;
		}

		init_pair(2, COLOR_BLUE, COLOR_BLACK);
		init_pair(3, COLOR_YELLOW, COLOR_BLACK);
		init_pair(4, COLOR_GREEN, COLOR_BLACK);
		init_pair(5, COLOR_RED, COLOR_BLACK);

		wrefresh(win_game);
		bool play_game = true;
		int input;
		int move;

		while (play_game) {


			string command = "";
			message m;

			if (dealer) {

				if(q == 0 && socket_2.receive(m, true)) {
					m >> command;

					//Поключился новый игрok
					if (command == "alive") {
						pl_col = 3;
						while (pl_col < 6 && colors_pl[pl_col] == true ) {
							pl_col += 1;
						}
						colors_pl[pl_col] = true;

						m = message();
						q += 1;

						m << "new_player" << pl_col << COLS + 1 << LINES + 1;

						for (int i = 0; i < COLS + 1; ++i) {
							for (int j = 0; j < LINES + 1; ++j) {
								m << matrix[i][j].value;
								m << matrix[i][j].color;
							}
						}

						if(!socket_1.send(m)) {
							exit (-99);
						}
						m = message();
						command = "";

					}  
				}	

				int e = 0;
				if (q != 0) {
					for (int i = 0; i < q; ++i) {

						if (!socket_2.receive(m)) {
							m = message();
							m << "end_game";
							socket_1.send(m);
							e = 1;
							break;
						}

						m >> command;

						//Поключился новый игрok
						if (command == "alive") {
							//sleep(10);
							pl_col = 3;
							while (pl_col < 6 && colors_pl[pl_col] == true ) {
								pl_col += 1;
							}

							colors_pl[pl_col] = true;
							m = message();
							q += 1;
							i -= 1;

							m << "new_player" << pl_col << COLS + 1 << LINES + 1;

							for (int i = 0; i < COLS + 1; ++i) {
								for (int j = 0; j < LINES + 1; ++j) {
									m << matrix[i][j].value;
									m << matrix[i][j].color;
								}
							}
							if(!socket_1.send(m)) {
								exit (-99);
							}
							m = message();
							command = "";
							continue;
						}


						if (command == "delete_player") {

							long unsigned int vsize;
							int x;
							int y;
							m >> vsize;
							vector <coord> v(vsize);
							for (int g = 0; g < vsize; ++g) {
								m >> x;
								m >> y;
								v[g].x = x;
								v[g].y = y;
							}
							clear(matrix, v);
							wrefresh(win_game);
							q -= 1;
							i -= 1;
							int c;
							m >> c;
							colors_pl[c] = false;
						}

						//Игрок поменял направление
						if (command == "turn") {
							int pi;
							int pj;
							int pcolor;
							int pdirection;
							m >> pi >> pj >> pcolor >> pdirection;
							matrix[pi][pj].color = pcolor;
							
							wattron(win_game,COLOR_PAIR(pcolor));

							if (pdirection == 0) {
								matrix[pi][pj].value = 3;
								mvwprintw(win_game, pj, pi,"ᐯ");
							}
							if (pdirection == 1) {
								matrix[pi][pj].value = 2;
								mvwprintw(win_game, pj, pi,"ᐱ");
							}
							if (pdirection == 2) {
								matrix[pi][pj].value = 4;
								mvwprintw(win_game, pj, pi,"<");
							}
							if (pdirection == 3) {
								matrix[pi][pj].value = 5;
								mvwprintw(win_game, pj, pi,">");
							}
							wattroff(win_game,COLOR_PAIR(pcolor));
							m = message();
							socket_2.receive(m);
							m >> command;
						}

						//Получили движение игрока
						if (command == "move_player") {
							int pi;
							int pj;
							int pcolor;
							int pdirection;

							int pi2;
							int pj2;
							int pcolor2;
							int pdirection2;

							string mode;

							m >> pi2 >> pj2 >> pcolor2 >> pdirection2 >> mode;

							if (mode == "move") {
								m >> pi >> pj >> pcolor >> pdirection;
								
								wattron(win_game,COLOR_PAIR(pcolor));

								if (pdirection2 == -1) {
									matrix[pi2][pj2].value = 1;
									mvwprintw(win_game, pj2, pi2,"0");
								}

								matrix[pi2][pj2].color = pcolor;
								matrix[pi][pj].color = pcolor;

								if (pdirection == 0) {
									matrix[pi][pj].value = 3;
									mvwprintw(win_game, pj, pi,"ᐯ");
								}
								if (pdirection == 1) {
									matrix[pi][pj].value = 2;
									mvwprintw(win_game, pj, pi,"ᐱ");
								}
								if (pdirection == 2) {
									matrix[pi][pj].value = 4;
									mvwprintw(win_game, pj, pi,"<");
								}
								if (pdirection == 3) {
									matrix[pi][pj].value = 5;
									mvwprintw(win_game, pj, pi,">");
								}
								wattroff(win_game,COLOR_PAIR(pcolor));
								m = message();
								command = "";
								continue;

							} else 

							if (mode == "delete") {

								long unsigned int vsize;
								int x;
								int y;
								m >> vsize;

								vector <coord> v(vsize);
								for (int g = 0; g < vsize; ++g) {
									m >> x;
									m >> y;
									v[g].x = x;
									v[g].y = y;
								}


								clear(matrix, v);

								wrefresh(win_game);

								int pi;
								int pj;
								int pcolor;
								int pdirection;
								m >> pi >> pj >> pcolor >> pdirection;
								matrix[pi][pj].color = pcolor;
								
								wattron(win_game,COLOR_PAIR(pcolor));

								if (pdirection == 0) {
									matrix[pi][pj].value = 3;
									mvwprintw(win_game, pj, pi,"ᐯ");
								}
								if (pdirection == 1) {
									matrix[pi][pj].value = 2;
									mvwprintw(win_game, pj, pi,"ᐱ");
								}
								if (pdirection == 2) {
									matrix[pi][pj].value = 4;
									mvwprintw(win_game, pj, pi,"<");
								}
								if (pdirection == 3) {
									matrix[pi][pj].value = 5;
									mvwprintw(win_game, pj, pi,">");
								}
								wattroff(win_game,COLOR_PAIR(pcolor));
								m = message();
								command = "";
								continue;
							}
						}
						

						//Получили позицию старта игрока
						if (command == "start_player") {
							int pi;
							int pj;
							int pcolor;
							int pdirection;
							m >> pi >> pj >> pcolor >> pdirection;
							matrix[pi][pj].color = pcolor;
							
							wattron(win_game,COLOR_PAIR(pcolor));

							if (pdirection == 0) {
								matrix[pi][pj].value = 3;
								mvwprintw(win_game, pj, pi,"ᐯ");
							}
							if (pdirection == 1) {
								matrix[pi][pj].value = 2;
								mvwprintw(win_game, pj, pi,"ᐱ");
							}
							if (pdirection == 2) {
								matrix[pi][pj].value = 4;
								mvwprintw(win_game, pj, pi,"<");
							}
							if (pdirection == 3) {
								matrix[pi][pj].value = 5;
								mvwprintw(win_game, pj, pi,">");
							}
							if (pdirection == -1) {
								matrix[pi][pj].value = 1;
								mvwprintw(win_game, pj, pi,"0");
							}
							wattroff(win_game,COLOR_PAIR(pcolor));
							m = message();
							command = "";
						} 
					}

					if (e == 1) {
						break;
					}

					if (socket_2.receive(m, true)) {
						m >> command;
						//Поключился новый игрok
						if (command == "alive") {
							pl_col = 3;
							while (pl_col < 6 && colors_pl[pl_col] == true ) {
								pl_col += 1;
							}
							colors_pl[pl_col] = true;

							m = message();
							q += 1;

							m << "new_player" << pl_col << COLS + 1 << LINES + 1;

							for (int i = 0; i < COLS + 1; ++i) {
								for (int j = 0; j < LINES + 1; ++j) {
									m << matrix[i][j].value;
									m << matrix[i][j].color;
								}
							}
							if(!socket_1.send(m)) {
								exit (-99);
							}
							m = message();
							command = "";

							socket_2.receive(m);
							m >> command;

							if (command == "start_player") {
								int pi;
								int pj;
								int pcolor;
								int pdirection;
								m >> pi >> pj >> pcolor >> pdirection;
								matrix[pi][pj].color = pcolor;
								wattron(win_game,COLOR_PAIR(pcolor));

								if (pdirection == 0) {
									matrix[pi][pj].value = 3;
									mvwprintw(win_game, pj, pi,"ᐯ");
								}
								if (pdirection == 1) {
									matrix[pi][pj].value = 2;
									mvwprintw(win_game, pj, pi,"ᐱ");
								}
								if (pdirection == 2) {
									matrix[pi][pj].value = 4;
									mvwprintw(win_game, pj, pi,"<");
								}
								if (pdirection == 3) {
									matrix[pi][pj].value = 5;
									mvwprintw(win_game, pj, pi,">");
								}
								if (pdirection == -1) {
									matrix[pi][pj].value = 1;
									mvwprintw(win_game, pj, pi,"0");
								}
								wattroff(win_game,COLOR_PAIR(pcolor));
								m = message();
								command = "";
							} 

						}  else 
						exit (-88);
					}

				} 
			}

			int end = 0;

			if (dealer) {

				clock_t now = clock();
				clock_t times = clock();
				while (now - times < CLOCKS_PER_SEC * 0.1) {

					input = wgetch(win_game);

					if (input == 10) {
						if (q != 0) {
							message mgs;
							mgs << "end_game";
							socket_1.send(mgs);
							mgs = message();
						}
						end = 1;
						break;
					}

					if (input == KEY_DOWN && player_1.direction != 1 && move == 0) {
						move = 1;
						wattron(win_game,COLOR_PAIR(player_1.color));
						mvwprintw(win_game,player_1.y, player_1.x, "ᐯ");
						matrix[player_1.x][player_1.y].value = 3;
						player_1.direction = 0;
						wattroff(win_game,COLOR_PAIR(player_1.color));
					}

					if (input == KEY_UP && player_1.direction != 0 && move == 0) {
						move = 1;
						wattron(win_game,COLOR_PAIR(player_1.color));
						mvwprintw(win_game,player_1.y, player_1.x, "ᐱ");
						matrix[player_1.x][player_1.y].value = 2;
						player_1.direction = 1;
						wattroff(win_game,COLOR_PAIR(player_1.color));
					}

					if (input == KEY_LEFT && player_1.direction != 3 && move == 0) {
						move = 1;
						wattron(win_game,COLOR_PAIR(player_1.color));
						mvwprintw(win_game,player_1.y, player_1.x, "<");
						matrix[player_1.x][player_1.y].value = 4;
						player_1.direction = 2;
						wattroff(win_game,COLOR_PAIR(player_1.color));
					}

					if (input == KEY_RIGHT && player_1.direction != 2 && move == 0) {
						move = 1;
						wattron(win_game,COLOR_PAIR(player_1.color));
						mvwprintw(win_game,player_1.y, player_1.x, ">");
						matrix[player_1.x][player_1.y].value = 5;
						player_1.direction = 3;
						wattroff(win_game,COLOR_PAIR(player_1.color));
					}	
					wrefresh(win_game);
					now = clock();
				}

				if (end == 1) {
					break;
				}
				
				move = 0;
				wattron(win_game,COLOR_PAIR(player_1.color));
				if (player_1.direction == 0) {
					mvwprintw(win_game,player_1.y, player_1.x, "0");
					matrix[player_1.x][player_1.y].value = 1;
					matrix[player_1.x][player_1.y].color = player_1.color;
					if (player_1.y == LINES - 1) {
						player_1.y = 0;
					} else {
						player_1.y += 1;
					}
					if (matrix[player_1.x][player_1.y].value > 0) {
						clear(matrix,pl_1_way);
						generation_player(player_1, matrix, pl_1_way, COLS, LINES);
					} else {
						pl_1_way.push_back({player_1.x, player_1.y});
						matrix[player_1.x][player_1.y].value = 3;
						matrix[player_1.x][player_1.y].color = player_1.color;
						mvwprintw(win_game,player_1.y, player_1.x, "ᐯ");
					}
				} else

				if (player_1.direction == 1) {
					mvwprintw(win_game,player_1.y, player_1.x, "0");
					matrix[player_1.x][player_1.y].value = 1;
					matrix[player_1.x][player_1.y].color = player_1.color;
					if (player_1.y == 0) {
						player_1.y = LINES;
					} else {
						player_1.y -= 1;
					}
					if (matrix[player_1.x][player_1.y].value > 0) {
						clear(matrix,pl_1_way);
						generation_player(player_1, matrix, pl_1_way, COLS, LINES);
					} else {
						pl_1_way.push_back({player_1.x, player_1.y});
						matrix[player_1.x][player_1.y].value = 2;
						matrix[player_1.x][player_1.y].color = player_1.color;
						mvwprintw(win_game,player_1.y, player_1.x, "ᐱ");
					}
				} else

				if (player_1.direction == 2) {
					mvwprintw(win_game,player_1.y, player_1.x, "0");
					matrix[player_1.x][player_1.y].value = 1;
					matrix[player_1.x][player_1.y].color = player_1.color;
					if (player_1.x == 0) {
						player_1.x = COLS;
					} else {
						player_1.x -= 1;
					}
					if (matrix[player_1.x][player_1.y].value > 0) {
						clear(matrix,pl_1_way);
						generation_player(player_1, matrix, pl_1_way, COLS, LINES);
					} else {
						pl_1_way.push_back({player_1.x, player_1.y});
						matrix[player_1.x][player_1.y].value = 4;
						matrix[player_1.x][player_1.y].color = player_1.color;
						mvwprintw(win_game,player_1.y, player_1.x, "<");
					}
				}	else

				if (player_1.direction == 3) {
					mvwprintw(win_game,player_1.y, player_1.x, "0");
					matrix[player_1.x][player_1.y].value = 1;
					matrix[player_1.x][player_1.y].color = player_1.color;
					if (player_1.x == COLS -1) {
						player_1.x = 0;
					} else {
						player_1.x += 1;
					}
					if (matrix[player_1.x][player_1.y].value > 0) {
						clear(matrix,pl_1_way);
						generation_player(player_1, matrix, pl_1_way, COLS, LINES);
					} else {
						pl_1_way.push_back({player_1.x, player_1.y});
						matrix[player_1.x][player_1.y].value = 5;
						matrix[player_1.x][player_1.y].color = player_1.color;
						mvwprintw(win_game,player_1.y, player_1.x, ">");
					}
				}	

				wattroff(win_game,COLOR_PAIR(player_1.color));

				if (q != 0) {
					m << "update";

					m << COLS + 1 << LINES + 1;
					for (int i = 0; i < COLS + 1; ++i) {
						for (int j = 0; j < LINES + 1; ++j) {
							m << matrix[i][j].value;
							m << matrix[i][j].color;
						}
					}

					m << "OK";
					socket_1.send(m);
					m = message();
					command = "";
				}
			
			} else {

				message mesa;

				if (!socket_1.receive(mesa)) {
					break;
				}

				string str, str1;
				mesa >> str;
				int sx;
				int sy;
				int end2 = 0;

				while (str != "update") {
					if (str == "end_game") {
						break;
					}

					if (!socket_1.receive(mesa)){
						break;
					}
					mesa >> str;
				}

				if (str == "end_game") {
					break;
				}

				if (str == "update") {

					mesa >> sx >> sy;
					for (int i = 0; i < sx; ++i) {
						for (int j = 0; j < sy; ++j) {
							int f;
							int f2;
							mesa >> f;
							mesa >> f2;
							matrix[i][j].value = f;
							matrix[i][j].color = f2;

						}
					}

					wclear(win_game);

					for (int i = 0; i < sx; ++i) {
						for (int j = 0; j < sy; ++j) {

						if (matrix[i][j].value == 0) {
							continue;
						}

						if (matrix[i][j].value == 1) {
							wattron(win_game,COLOR_PAIR(matrix[i][j].color));
							mvwprintw(win_game,j, i, "0");
							wattron(win_game,COLOR_PAIR(matrix[i][j].color));
						}

						if (matrix[i][j].value == 2) {
							wattron(win_game,COLOR_PAIR(matrix[i][j].color));
							mvwprintw(win_game,j, i, "ᐱ");
							wattron(win_game,COLOR_PAIR(matrix[i][j].color));
						}

						if (matrix[i][j].value == 3) {
							wattron(win_game,COLOR_PAIR(matrix[i][j].color));
							mvwprintw(win_game,j, i, "ᐯ");
							wattron(win_game,COLOR_PAIR(matrix[i][j].color));
						}

						if (matrix[i][j].value == 4) {
							wattron(win_game,COLOR_PAIR(matrix[i][j].color));
							mvwprintw(win_game,j, i, "<");
							wattron(win_game,COLOR_PAIR(matrix[i][j].color));
						}

						if (matrix[i][j].value == 5) {
							wattron(win_game,COLOR_PAIR(matrix[i][j].color));
							mvwprintw(win_game,j, i, ">");
							wattron(win_game,COLOR_PAIR(matrix[i][j].color));
						}

					}
				}
				wrefresh(win_game);

				} 

				mesa >> str;

				if (str == "OK"){

					input = wgetch(win_game);

					if (input == 10) {
						message mgs;
						mgs << "delete_player";
						mgs << pl_1_way.size();
						for (int i = 0; i < pl_1_way.size(); ++i) {
							mgs << pl_1_way[i].x;
							mgs << pl_1_way[i].y;
						} 
						mgs << player_1.color;
						socket_2.send(mgs);
						break;
					}


					if (input == KEY_DOWN && player_1.direction != 1 && move == 0) {
						move = 1;
						wattron(win_game,COLOR_PAIR(player_1.color));
						mvwprintw(win_game,player_1.y, player_1.x, "ᐯ");
						matrix[player_1.x][player_1.y].value = 3;
						player_1.direction = 0;
						wattroff(win_game,COLOR_PAIR(player_1.color));
						message mes;
						mes << "turn" << player_1.x << player_1.y << player_1.color << player_1.direction;
						if (!socket_2.send(mes)) {
							exit(228);
						}
						mes = message();
					}

					if (input == KEY_UP && player_1.direction != 0 && move == 0) {
						move = 1;
						wattron(win_game,COLOR_PAIR(player_1.color));
						mvwprintw(win_game,player_1.y, player_1.x, "ᐱ");
						matrix[player_1.x][player_1.y].value = 2;
						player_1.direction = 1;
						wattroff(win_game,COLOR_PAIR(player_1.color));
						message mes;
						mes << "turn" << player_1.x << player_1.y << player_1.color << player_1.direction;
						socket_2.send(mes);
						mes = message();
					}

					if (input == KEY_LEFT && player_1.direction != 3 && move == 0) {
						move = 1;
						wattron(win_game,COLOR_PAIR(player_1.color));
						mvwprintw(win_game,player_1.y, player_1.x, "<");
						matrix[player_1.x][player_1.y].value = 4;
						player_1.direction = 2;
						wattroff(win_game,COLOR_PAIR(player_1.color));
						message mes;
						mes << "turn" << player_1.x << player_1.y << player_1.color << player_1.direction;
						socket_2.send(mes);
						mes = message();
					}

					if (input == KEY_RIGHT && player_1.direction != 2 && move == 0) {
						move = 1;
						wattron(win_game,COLOR_PAIR(player_1.color));
						mvwprintw(win_game,player_1.y, player_1.x, ">");
						matrix[player_1.x][player_1.y].value = 5;
						player_1.direction = 3;
						wattroff(win_game,COLOR_PAIR(player_1.color));
						message mes;
						mes << "turn" << player_1.x << player_1.y << player_1.color << player_1.direction;
						socket_2.send(mes);
						mes = message();
					}	

					move = 0;

					wattron(win_game,COLOR_PAIR(player_1.color));

					if (player_1.direction == 0) {
						message mg;
						mg << "move_player";
						mg << player_1.x << player_1.y << player_1.color << -1;
						mvwprintw(win_game,player_1.y, player_1.x, "0");
						matrix[player_1.x][player_1.y].value = 1;
						matrix[player_1.x][player_1.y].color = player_1.color;

						if (player_1.y == sy - 2) {
							player_1.y = 0;
						} else {
							player_1.y += 1;
						}

						if (matrix[player_1.x][player_1.y].value > 0) {

							mg << "delete" << pl_1_way.size();

							for (int i = 0; i < pl_1_way.size(); ++i) {
								mg << pl_1_way[i].x;
								mg << pl_1_way[i].y;
							}

							clear(matrix,pl_1_way);

							generation_player(player_1, matrix, pl_1_way, sx - 2, sy - 2);

							mg << player_1.x << player_1.y << player_1.color << player_1.direction;

							socket_2.send(mg);
							mg = message();

						} else {

							pl_1_way.push_back({player_1.x, player_1.y});
							matrix[player_1.x][player_1.y].value = 3;
							matrix[player_1.x][player_1.y].color = player_1.color;
							mvwprintw(win_game,player_1.y, player_1.x, "ᐯ");

							mg << "move" << player_1.x << player_1.y << player_1.color << player_1.direction;
							socket_2.send(mg);
							mg = message();
						}
					} else

					if (player_1.direction == 1) {
						message mg;
						mg << "move_player";
						mg << player_1.x << player_1.y << player_1.color << -1;
						mvwprintw(win_game,player_1.y, player_1.x, "0");
						matrix[player_1.x][player_1.y].value = 1;
						matrix[player_1.x][player_1.y].color = player_1.color;

						if (player_1.y == 0) {
							player_1.y = sy - 1;
						} else {
							player_1.y -= 1;
						}

						if (matrix[player_1.x][player_1.y].value > 0) {
							mg << "delete" << pl_1_way.size();

							for (int i = 0; i < pl_1_way.size(); ++i) {
								mg << pl_1_way[i].x;
								mg << pl_1_way[i].y;
							}

							clear(matrix,pl_1_way);
							generation_player(player_1, matrix, pl_1_way, sx - 2, sy - 2);

							mg << player_1.x << player_1.y << player_1.color << player_1.direction;

							socket_2.send(mg);
							mg = message();

						} else {

							pl_1_way.push_back({player_1.x, player_1.y});
							matrix[player_1.x][player_1.y].value = 2;
							matrix[player_1.x][player_1.y].color = player_1.color;
							mvwprintw(win_game,player_1.y, player_1.x, "ᐱ");
							mg << "move" << player_1.x << player_1.y << player_1.color << player_1.direction;
							socket_2.send(mg);
							mg = message();
						}
					} else	

					if (player_1.direction == 2) {
						message mg;
						mg << "move_player";
						mg << player_1.x << player_1.y << player_1.color << -1;
						mvwprintw(win_game,player_1.y, player_1.x, "0");
						matrix[player_1.x][player_1.y].value = 1;
						matrix[player_1.x][player_1.y].color = player_1.color;

						if (player_1.x == 0) {
							player_1.x = sx - 1;
						} else {
							player_1.x -= 1;
						}

						if (matrix[player_1.x][player_1.y].value > 0) {
							mg << "delete" << pl_1_way.size();

							for (int i = 0; i < pl_1_way.size(); ++i) {
								mg << pl_1_way[i].x;
								mg << pl_1_way[i].y;
							}

							clear(matrix,pl_1_way);
							generation_player(player_1, matrix, pl_1_way, sx - 2, sy - 2);

							mg << player_1.x << player_1.y << player_1.color << player_1.direction;

							socket_2.send(mg);
							mg = message();

						} else {

							pl_1_way.push_back({player_1.x, player_1.y});
							matrix[player_1.x][player_1.y].value = 4;
							matrix[player_1.x][player_1.y].color = player_1.color;
							mvwprintw(win_game,player_1.y, player_1.x, "<");
							mg << "move" << player_1.x << player_1.y << player_1.color << player_1.direction;
							socket_2.send(mg);
							mg = message();
						}
					} else	

					if (player_1.direction == 3) {
						message mg;
						mg << "move_player";
						mg << player_1.x << player_1.y << player_1.color << -1;
						mvwprintw(win_game,player_1.y, player_1.x, "0");
						matrix[player_1.x][player_1.y].value = 1;
						matrix[player_1.x][player_1.y].color = player_1.color;
						if (player_1.x == sx - 2) {
							player_1.x = 0;
						} else {
							player_1.x += 1;
						}
						if (matrix[player_1.x][player_1.y].value > 0) {
							mg << "delete" << pl_1_way.size();

							for (int i = 0; i < pl_1_way.size(); ++i) {
								mg << pl_1_way[i].x;
								mg << pl_1_way[i].y;
							}

							clear(matrix,pl_1_way);
							generation_player(player_1, matrix, pl_1_way, sx - 2, sy - 2);

							mg << player_1.x << player_1.y << player_1.color << player_1.direction;

							socket_2.send(mg);
							mg = message();

						} else {

							pl_1_way.push_back({player_1.x, player_1.y});
							matrix[player_1.x][player_1.y].value = 5;
							matrix[player_1.x][player_1.y].color = player_1.color;
							mvwprintw(win_game,player_1.y, player_1.x, ">");
							mg << "move" << player_1.x << player_1.y << player_1.color << player_1.direction;
							socket_2.send(mg);
							mg = message();
						}
					}	
					wattroff(win_game,COLOR_PAIR(player_1.color));	
				}	
				wrefresh(win_game);
			}
		}
	}
};