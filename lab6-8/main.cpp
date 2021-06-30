#include <iostream> 
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <thread>
#include "func.hpp"

using namespace std;
using namespace zmqpp;


void add_to_vector(vector<int> &v, int i, int id) {
	if (i == v.size() - 1) {
		v.push_back(id);
		return;
	}
	auto begin = v.begin();
	for (int j = 0; j < i; ++ j) {
		begin++;
	}
	v.insert(begin, id);
}

template <class T>
void delete_in_vector(vector<T> &v, int i) {
	if (i == v.size() - 1) {
		v.pop_back();
		return;
	}
	auto begin = v.begin();
	for (int j = 0; j < i; ++ j) {
		begin++;
	}
	v.erase(begin);
}

struct sock
{
	socket* sock;
	string port;
};

void delete_in_doible_vector(vector<vector <int>> &v, int i) {
	if (i == v.size() - 1) {
		v.pop_back();
		return;
	}
	auto begin = v.begin();
	for (int j = 0; j < i; ++ j) {
		begin++;
	}
	v.erase(begin);
}


void exec(int exec_id, int n, vector<int> v, vector<sock> &begins, vector<vector<int>> &indificators) {
	int go = -1;
	for (int i = 0; i < indificators.size(); ++i) {
		for (int j = 0; j < indificators[i].size(); ++j) {
			if (indificators[i][j] == exec_id){
				go = i;
			}
			if (go != -1) {
				break;
			}
		}
	}
	if (go == -1) {
		printf("Error: Not found\n");
	} else {
		message mgs;
		mgs << "exec" << exec_id << n;
		for (int i = 0; i < n; ++ i) {
			mgs << v[i];
		}
		v.clear();
		begins[go].sock->send(mgs);
		if (!begins[go].sock->receive(mgs)) {
			mgs = message();
			mgs << "ERROR";
		}
		string command;
		mgs >> command;
		if (command == "OK") {
			int id_ok;
			int sum;
			mgs >> id_ok >> sum;
			printf("OK:%d: %d\n", id_ok, sum);
		} else {
			printf("ERROR\n");
		}
	}
}


int main() {
	string s;
	vector <sock> begins;
	vector <vector <int>> indificators;
	context con_1;
	string ports = "49152";
	vector<int> v;
	int next = 0;
	
	while (cin >> s) {

		if (s == "create") {
			int id;
			cin >> id;
			int parent;
			cin >> parent;

			if (parent == -1) {

				socket* soc_1 = new socket(con_1, socket_type::pair);
				soc_1->set(socket_option::receive_timeout, 5000);
				ports = new_ports(ports);
				//printf("%s\n", ports.c_str());
				string connecting =  string("tcp://127.0.0.1:") + ports;
				soc_1->bind(connecting.c_str());
				begins.push_back({soc_1, ports});
				indificators.push_back(vector<int>());
				indificators[indificators.size() - 1].push_back(id);
				int id_pr;
				id_pr = fork(); 

				if (id_pr == -1) {
					printf("Ошибка в создании процесса\n");
					break;
				}

				if (id_pr == 0) {
					if (execl("cn","cn", to_string(id).c_str(), ports.c_str(), to_string(0), to_string(0), NULL) == -1) {
						printf("Ошибка в execl\n");
						break;
					}
				}

				message mgs;
				begins[begins.size() - 1].sock->receive(mgs);
				string result;
				int pid;
				mgs >> result >> pid;
				mgs = message();
				if (result == "OK") {
					printf("OK: %d\n", pid);
				} else {
					printf("Error: Create node");
				}

			} else {

				int go = -1;
				int exist = 0;

				for (int i = 0; i < indificators.size(); ++i) {
					for (int j = 0; j < indificators[i].size(); ++j) {

						if (indificators[i][j] == id) {
							exist = 1;
							break;
						}
					}

					if (exist == 1) {
						break;
					}
				}

				if (exist == 1) {
					printf("Error: Already exists\n");
				} else {

					for (int i = 0; i < indificators.size(); ++i) {
						for (int j = 0; j < indificators[i].size(); ++j) {
							if (indificators[i][j] == parent){
								go = i;
							}
							if (go != -1) {
								break;
							}
						}
					}

					if (go == -1 || begins.size() == 0) {
						printf("Error: Parent not found\n");
					} else {
						message mgs;
						ports = new_ports(ports);
						mgs << "new_node" << parent << id << ports;
						begins[go].sock->send(mgs);
						if (!begins[go].sock->receive(mgs)) {
							printf("Ошибка в создании процесса\n");
							fflush(stdout);
							break;
						}
						string result;
						pid_t pid;
						mgs >> result >> pid;
						mgs = message();
						if (result == "OK") {
							int add;
							for (int i = 0; i < indificators[go].size(); ++i) {
								if (indificators[go][i] == parent) {
									add = i;
									break;
								}
							}

							add_to_vector(indificators[go], add + 1, id);
							printf("OK: %d\n", pid);
						} else {
							printf("Error: Create node");
						}
					}
				}
			}	
		}

		if (s == "ping") {
			int id;
			cin >> id;
			int go = -1;
			for (int i = 0; i < indificators.size(); ++i) {
				for (int j = 0; j < indificators[i].size(); ++j) {
					if (indificators[i][j] == id){
						go = i;
					}
					if (go != -1) {
						break;
					}
				}
			}

			if (go == -1) {
				printf("Error: Not found\n");
			} else {
				message mgs;
				mgs << "ping_id" << id;
				begins[go].sock->send(mgs);
				if (!begins[go].sock->receive(mgs)) {
					mgs = message();
					mgs << "ERROR";
				}
				string command;
				mgs >> command;
				if (command == "OK") {
					printf("OK: 1\n");
				} else {
					printf("OK: 0\n");
				}
			}
		}

		if (s == "exec") {
			int exec_id;
			cin >> exec_id;
			int n;
			cin >> n;
			for (int i = 0; i < n; ++i) {
				int value;
				cin >> value;
				v.push_back(value);
			}
			thread results_3(exec, exec_id, n, v, ref(begins), ref(indificators));
			results_3.detach();
			v.clear();
		}

		if (s == "remove") {
			int id_r;
			cin >> id_r;
			int go = -1;
			int y;
			for (int i = 0; i < indificators.size(); ++i) {
				for (int j = 0; j < indificators[i].size(); ++j) {
					if (indificators[i][j] == id_r){
						go = i;
						y = j;
					}
					if (go != -1) {
						break;
					}
				}
			}
			if (go == -1) {
				printf("Error: Not found\n");
			} else {
				message mgs;
				mgs << "remove" << id_r;
				begins[go].sock->send(mgs);
				if (y == 0 && indificators[go].size() > 1) {
					begins[go].sock->unbind(string("tcp://127.0.0.1:") + begins[go].port);
					begins[go].sock->bind(string("tcp://127.0.0.1:") + begins[go].port);
				}
				if (!begins[go].sock->receive(mgs)) {
					printf("Ошибка в создании процесса\n");
					fflush(stdout);
					break;
				}
				string result;
				mgs >> result;
				if (result == "OK" || result == "last") {
					if (result == "last") {
						printf("last host\n");
						string portl;
						mgs >> portl;
						string connecting = string("tcp://127.0.0.1:") + portl;
						begins[go].sock->unbind(connecting);
					}
					int del;
					for (int i = 0; i < indificators[go].size(); ++i) {
						if (indificators[go][i] == id_r) {
							del = i;
							break;
						}
					} 
					if (del == 0 && indificators[go].size() == 1) {
						delete_in_vector(begins, go);
						delete_in_vector(indificators, go);
					} else {
						delete_in_vector(indificators[go], del);
					}

					printf("OK\n");
				} else {
					printf("Error: Node is unavailable\n");
				}
			}
		}

		if (s == "end") {
			break;
		}
	}

	message mgs;
	mgs << "end";
	for (int i = 0; i < begins.size(); ++i) {
		begins[i].sock->send(mgs);
	}

}
