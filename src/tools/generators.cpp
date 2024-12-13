#include <iostream>
#include "../factory.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>

using json = nlohmann::json;
using namespace Fac;

/** Function to generate a C** header file from a JSON file of the format
* {
"title": "Resources",
"type": "string",
"enum": [
"CopperOre",
"CopperIngot",
] }
* @param json_file_path path to the JSON file
* @param output_file_path path to the output C header file
*/
void generate_resources_header_file(const std::string &json_file_path, const std::string &output_file_path) {
    std::ifstream i(json_file_path);
    if (!i) {
        std::cerr << "Could not open file: " << json_file_path << std::endl;
        return;
    }
    // Read the whole file into a string stream
    std::stringstream buffer;
    buffer << i.rdbuf();
    // Print to cout
    std::cout << buffer.str();

    json j;
    buffer >> j;

    std::ofstream out(output_file_path);
    out << "#ifndef RESOURCES_H\n";
    out << "#define RESOURCES_H\n\n";
    out << "namespace Fac {\n";
    out << "enum class Resource {\n";
    for (const auto &resource: j["enum"]) {
        out << "\t" << resource.get<std::string>() << ",\n";
    }
    out << "};\n";
    out << "} // Fac\n";
    out << "#endif //RESOURCES_H\n";
}

/**
 *
 * Function that generates a C++ header file from a JSON file
 * JSON format:
* {
        "id": "Stator",
        "produced_in": "Assembler",
        "time": 12,
        "unlocked_by": "Tier 4 - Advanced Steel Production",
        "inputs": [
            {
                "resource": "SteelPipe",
                "amount": 3
            },
            {
                "resource": "Wire",
                "amount": 8
            }
        ],
        "products": [
            {
                "resource": "Stator",
                "amount": 1
            }
        ]
    }
 * The output will a map of the Recipe struct instances
 *
 */

void generate_recipes_header_file(const std::string &json_file_path, const std::string &output_file_path) {
    std::ifstream i(json_file_path);
    if (!i) {
        std::cerr << "Could not open file: " << json_file_path << std::endl;
        return;
    }
    // Read the whole file into a string stream
    std::stringstream buffer;
    buffer << i.rdbuf();
    // Print to cout
    std::cout << buffer.str();

    json j;
    buffer >> j;

    // now convert
    std::ofstream out(output_file_path);
    out << "#ifndef RECIPES_H\n";
    out << "#define RECIPES_H\n\n";
    out << "#include \"resources.h\"\n";
    out << "#include \"../core.h\"\n";
    out << "namespace Fac {\n";
    for (const auto &recipe: j) {
        out << "inline Recipe recipe_" << recipe["id"].get<std::string>() << " = {\n";
        out << "\t.inputs = {\n";
        for (const auto &input: recipe["inputs"]) {
            out << "\t\t{Resource::" << input["resource"].get<std::string>() << ", " << input["amount"].get<int>() << "},\n";
        }
        out << "\t},\n";
        out << "\t.products = {\n";
        for (const auto &product: recipe["products"]) {
            out << "\t\t{Resource::" << product["resource"].get<std::string>() << ", " << product["amount"].get<int>() << "},\n";
        }
        out << "\t},\n";
        out << "\t.processing_time_s = " << recipe["time"].get<int>() << "\n";
        out << "};\n";
    }

    out << "} // Fac\n";
    out << "#endif //RECIPES_H\n";
}
