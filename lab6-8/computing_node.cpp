#include <iostream> 
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <vector>
#include <unistd.h>
#include <cstring>
#include <thread>
#include "func.hpp"

using namespace std;
using namespace zmqpp;

void new_node(int parent, int id_ch, string port, string port2, int id, socket &soc_1, socket &soc_2, bool &child) {
	
	message mgs;

	if (parent == id) {
		int indificator;
		if (child == true){
			string connecting;
			connecting = string("tcp://127.0.0.1:") + port2;
			soc_2.unbind(connecting);
			connecting = string("tcp://127.0.0.1:") + port;
			soc_2.bind(connecting);
		} else {
			string connecting =  string("tcp://127.0.0.1:") + port;
			soc_2.bind(connecting.c_str());
		}

		indificator = fork();

		if (indificator == -1) {
			mgs << "ERROR";
			soc_1.send(mgs);
			return;
		} 

		if (indificator == 0) {
			if (child == false) {
				if (execl("cn","cn", to_string(id_ch).c_str(), port.c_str(), to_string(0), to_string(0), NULL) == -1) {
					printf("Ошибка в execl\n");
					mgs << "ERROR";
					soc_1.send(mgs);
					return;
				}
			} else {
				if (execl("cn","cn", to_string(id_ch).c_str(), port.c_str(), "1", port2.c_str(), NULL) == -1) {
					printf("Ошибка в execl\n");
					mgs << "ERROR";
					soc_1.send(mgs);
					return;
				}
			}
		}

		child = true;
		pid_t pid;
		string t;
		soc_2.receive(mgs);
		mgs >> t >> pid;
		mgs = message();
		mgs << "OK" << pid;
		soc_1.send(mgs);
	} else {
		child = true;
		mgs << "new_node" << parent << id_ch << port;
		soc_2.send(mgs);
		soc_2.set(socket_option::receive_timeout, 3000);
		if (!soc_2.receive(mgs)) {
			mgs = message();
			mgs << "ERROR";
		}
		soc_1.send(mgs);
	}
}

void ping_id(int id, int id_ping, socket &soc_1, socket &soc_2){
	if (id == id_ping) {
		message mgs;
		mgs << "OK";
		soc_1.send(mgs);
	} else {
		message mgs;
		mgs << "ping_id" << id_ping;
		soc_2.send(mgs);
		soc_2.set(socket_option::receive_timeout, 3000);
		if (!soc_2.receive(mgs)) {
			mgs = message();
			mgs << "ERROR";
			soc_1.send(mgs);
			return;
		} else {
			soc_1.send(mgs);
		}
	}
}

void exec(int id, int exec_id, int n, vector<int> v, socket &soc_1, socket &soc_2) {

	if (id == exec_id) {
		int sum = 0;
		for (int i = 0; i < n; ++i) {
			sum += v[i];
		}
		message mgs;
		mgs << "OK" << id << sum;
		soc_1.send(mgs);
	} else {
		message mgs;
		mgs << "exec" << exec_id << n;
		for (int i = 0; i < n; ++i) {
			mgs << v[i];
		}
		soc_2.send(mgs);
		soc_2.set(socket_option::receive_timeout, 3000);
		if (!soc_2.receive(mgs)) {
			mgs = message();
			mgs << "ERROR";
			soc_1.send(mgs);
			return;
		} else {
			soc_1.send(mgs);
		}
	}
}

void removed(int id, int id_r, socket &soc_1, socket &soc_2, bool & child, bool & alive, string port, string port2) {
	if (id == id_r) {
		if (child == true) {
			message mgs;
			mgs << "new_bind";
			soc_1.send(mgs);
			mgs = message();
			string connecting =  string("tcp://127.0.0.1:") + port;
			soc_1.disconnect(connecting);
			printf("disconnect port %d %s\n", id, port.c_str());
			mgs << "new_connect" << port;
			soc_2.send(mgs);
			connecting =  string("tcp://127.0.0.1:") + port2;
			soc_2.unbind(connecting);
			printf("unbind port %d %s\n", id, port2.c_str());
			alive = false;
		} else {
			message mgs;
			mgs << "last" << port;
			soc_1.send(mgs);
			string connecting = string("tcp://127.0.0.1:") + port;
			soc_1.disconnect(connecting);
			alive = false;
		}

	} else {
		message mgs;
		mgs << "remove" << id_r;
		soc_2.send(mgs);
		soc_2.set(socket_option::receive_timeout, 3000);
		if (!soc_2.receive(mgs)) {
			mgs = message();
			mgs << "ERROR";
		}

		string command;
		mgs >> command;

		if (command == "new_bind") {
			soc_2.unbind(string("tcp://127.0.0.1:") + port2);
			soc_2.bind(string("tcp://127.0.0.1:") + port2);
			if (!soc_2.receive(mgs)) {
				mgs = message();
				mgs << "ERROR";
			}
		}

		if (command == "last") {
			child = false;
			string port;
			mgs >> port;
			string connecting = string("tcp://127.0.0.1:") + port;
			soc_2.unbind(connecting);
			mgs = message();
			mgs << "OK";
		}
		soc_1.send(mgs);
	}
}

int main(int argc, char** argv) {

	bool child = false;
	context con_1;
	int id = stoi(argv[1]);
	string port(argv[2]);
	socket soc_1 = socket(con_1, socket_type::pair);
	string connecting =  string("tcp://127.0.0.1:") + port;
	soc_1.connect(connecting.c_str());

	message m;
	pid_t p = getpid();
	m << "OK" << p;
	soc_1.send(m);
	string port2;
	socket soc_2 = socket(con_1, socket_type::pair);

	string flag = argv[3];
	if (flag == "1") {
		string new_port = argv[4];
		port2 = new_port;
		string connecting =  string("tcp://127.0.0.1:") + new_port;
		soc_2.bind(connecting.c_str());
		child = true;
	}

	message mgs;
	vector<int> v;
	bool alive = true;

	while (alive) {
		if (soc_1.receive(mgs)) {
			string command;
			mgs >> command;
			pid_t p = getpid();

			if (command == "new_node") {
				int parent;
				int id_ch;
				string port;
				mgs >> parent >> id_ch >> port;
				thread thread_1(new_node, parent, id_ch, port, port2, id, ref(soc_1), ref(soc_2), ref(child));
				thread_1.detach();
				port2 = port;
			}

			if (command == "ping_id") {
				int id_ping;
				mgs >> id_ping;
				thread thread_2(ping_id, id, id_ping, ref(soc_1), ref(soc_2));
				thread_2.detach();
			}

			if (command == "exec") {
				int exec_id;
				mgs >> exec_id;
				int n;
				mgs >> n;
				for (int i = 0; i < n; ++i) {
					int value;
					mgs >> value;
					v.push_back(value);
				}
				thread thread_3(exec, id, exec_id, n, v, ref(soc_1), ref(soc_2));
				thread_3.detach();
				v.clear();
			}

			if (command == "remove") {
				int id_r;
				mgs >> id_r;
				thread thread_4(removed, id, id_r, ref(soc_1), ref(soc_2), ref(child), ref(alive), port, port2);
				thread_4.detach();
			}

			if (command == "new_connect") {
				string new_port;
				mgs >> new_port;
				string connecting = string("tcp://127.0.0.1:") + port;
				printf("disconnect port %d %s\n", id, port.c_str());
				soc_1.disconnect(connecting);
				sleep(1);
				connecting = string("tcp://127.0.0.1:") + new_port;
				printf("connect port %d %s\n", id, new_port.c_str());
				soc_1.connect(connecting);
				port = new_port;
				mgs = message();
				mgs << "OK";
				soc_1.send(mgs);
			}

			if (command == "end") {
				if (child) {
					mgs = message();
					mgs << "end";
					soc_2.send(mgs);
				}
				alive = false;
			}
		}
	}
}
