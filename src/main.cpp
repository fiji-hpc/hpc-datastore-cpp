#include <fmt/core.h>
#include <iostream>

namespace x {
int x = 4;
int foo() { return 5; }
} // namespace x

namespace y {
using namespace x;
int bar() { return foo(); }
} // namespace y

int main() {
	std::cout << fmt::format("Hello world with code: {}\n", y::foo());
}

