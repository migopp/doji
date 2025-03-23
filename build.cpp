#include <cstdlib>
#include <filesystem>
#include <iostream>

#define ANSI_RESET "\033[0m"
#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"

void build_log(std::string l) {
  std::cout << ANSI_GREEN << "[BUILD]\t" << ANSI_RESET << l << std::endl;
}

void err_log(std::string l) {
  std::cout << ANSI_RED << "[ERR]\t" << ANSI_RESET << l << std::endl;
}

int main(void) {
  // Determine if need to configure build env
  if (!std::filesystem::exists("./target/doji")) {
    // Create build target directory
    if (!std::filesystem::create_directory("target")) {
      err_log("Failed to create `target` directory");
      return -1;
    }
    build_log("Created `target` directory");
  } else {
    build_log("`target` directory already exists, skipping...");
  }

  // Compile `doji`
  if (std::system("g++ -std=c++20 -o ./target/doji doji.cpp -lncurses") < 0) {
    err_log("Failed to compile `doji`");
    return -1;
  }
  build_log("Compiled `doji` @ ./target/doji");

  return 0;
}
