#include <cstdlib>
#include <filesystem>
#include <iostream>

#define ANSI_RESET "\033[0m"
#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"

int main(void) {
  // Create build target directory
  if (std::filesystem::create_directory("target")) {
    std::cout << ANSI_GREEN << "[BUILD]\t" << ANSI_RESET
              << "Created `target` directory" << std::endl;
  } else {
    std::cerr << ANSI_RED << "[ERR]\t" << ANSI_RESET
              << "Failed to create `target` directory" << std::endl;
    return -1;
  }

  // Compile `doji`
  int rc = std::system("g++ -o ./target/doji doji.cpp");
  if (rc < 0) {
    std::cerr << ANSI_RED << "[ERR]\t" << ANSI_RESET
              << "Failed to compile `doji`" << std::endl;
    return -1;
  }
  return 0;
}
