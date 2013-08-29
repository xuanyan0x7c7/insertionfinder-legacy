#pragma once
#include <string>
#include <utility>
#include <vector>


std::vector<std::pair<std::string, std::string>> Parse(int argc, char **argv);

std::vector<std::pair<std::string, std::string>>
Parse(const std::vector<std::string> &commandline);
