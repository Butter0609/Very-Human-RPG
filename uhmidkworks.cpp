#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

int main() {
	const std::array<std::string, 4> messages = {
		"Man is a creature of habit, and he will always find a way to justify his actions.",
		"Despite the fact that we live in a world of constant change, there are some things that remain the same.",
		"For example...",
		"The DMV"
	};

	for (const std::string& message : messages) {
		std::system("cls");
		std::cout << message << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(7));
	}

	return 0;
}
