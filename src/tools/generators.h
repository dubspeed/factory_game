#ifndef GENERATORS_H
#define GENERATORS_H

#include <iostream>
#include "../factory.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>


void generate_resources_header_file(const std::string &json_file_path, const std::string &output_file_path);
void generate_recipes_header_file(const std::string &json_file_path, const std::string &output_file_path);

#endif //GENERATORS_H
