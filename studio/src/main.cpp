#include "cc/core/Application.hpp"

int main (int argc, char** argv){

    //TODO:: Implement argument parser
    // auto parser_ = cc::core::ArgumentParser(argc, argv);
    // parser_.AddOption("--help", "-h", "Show help message");
    // parser_.AddOption("--version", "-v", "Show version");
    // parser_.Parse();


    cc::Application app;
    app.Run();

    return 0;
}
