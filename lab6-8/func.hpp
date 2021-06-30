#ifndef FUNC_HPP
#define FUNC_HPP

#include <string>
#include <cstring>

using namespace std;

string new_ports(string & ports) {
	int ports2 = stoi(ports);
	ports2 += 1;
	ports = to_string(ports2);
	return ports;
}

#endif