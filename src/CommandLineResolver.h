#pragma once
#include <string>
#include <utility>
#include <vector>


std::vector<std::pair<std::string, std::string>> Resolve(int, char**);

std::vector<std::pair<std::string, std::string>>
Resolve(const std::vector<std::string>&);

