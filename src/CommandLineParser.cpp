#include "CommandLineParser.h"
using std::make_pair;
using std::pair;
using std::string;
using std::vector;


vector<pair<string, string>> Parse(int argc, char **argv) {
	vector<string> commandline;
	for (int i = 1; i < argc; ++i) {
		commandline.push_back(argv[i]);
	}
	return Parse(commandline);
}

vector<pair<string, string>> Parse(const vector<string>& commandline) {
	vector<pair<string, string>> command;
	for (const string& s: commandline) {
		if (s[0] != '-') {
			command.push_back(make_pair("", s));
		} else if (s.length() > 1) {
			if (s[1] == '-') {
				string str = s.substr(2);
				size_t pos = str.find('=');
				if (pos == string::npos) {
					command.push_back(make_pair(str, ""));
				} else {
					command.push_back(make_pair(str.substr(0, pos),
						str.substr(pos + 1)));
				}
			} else {
				for (auto iter = s.cbegin() + 1; iter != s.cend(); ++iter) {
					command.push_back(make_pair(string(1, *iter), ""));
				}
			}
		}
	}
	return command;
}
