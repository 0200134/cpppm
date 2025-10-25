#include <iostream>
#include <cassert>

int main() {
    std::cout << "[cpppm] basic smoke test running...\n";
    // 간단한 기본 assert
    assert(true && "Basic test passed");
    std::cout << "[cpppm] basic smoke test passed!\n";
    return 0;
}
