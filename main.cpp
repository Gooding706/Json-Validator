#include "parser.hpp"
#include <iostream>

//TODO:error messages are not handled well

int main(int argc, char** argv)
{
    std::ifstream file;
    file.open(argv[1]);
    std::vector<ccjson::token> tokens = ccjson::tokenize(file);

    bool valid = ccjson::validate(tokens);    
    std::cout << (valid ? "true" : "false") << '\n';

    file.close(); 
}
