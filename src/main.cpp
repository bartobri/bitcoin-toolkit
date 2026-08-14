#include "cli/dispatcher.hpp"
#include "util/error.hpp"

#include <iostream>

int main(int argc, char** argv) {
    try {
        return dispatch(argc, argv);
    } catch (const BtkError& err) {
        print_error(err);
        return 1;
    } catch (const std::exception& ex) {
        std::cerr << "btk: " << ex.what() << '\n';
        return 1;
    }
}
