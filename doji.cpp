#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <ncurses.h>
#include <sstream>
#include <string>
#include <vector>

namespace doji {

// O_O
void err_log(std::string l) {
  std::cerr << "\033[31m[ERR]\033[0m\t" << l << std::endl;
}

// Simple statuses.
//
// Something is either OK or not OK.
enum Status { HEALTHY, UNHEALTHY };

// Memory representation of a single flashcard.
//
// Cards just have properties...
struct Card {
private:
  Status status;
  enum { TADOUSHI, JIDOUSHI } type;
  std::string word;
  std::string opp_jpn;
  std::string opp_eng;

public:
  // Nothing burger constructor.
  Card() {}

  // Creates a card given a line of the form:
  // `<WORD> <TYPE> <OPPOSITE JPN> <OPPOSITE ENG>`
  Card(std::string line) : status(UNHEALTHY) {
    std::string type_raw;
    std::istringstream line_stream(line);

    // Extract raw components.
    line_stream >> word;
    line_stream >> type_raw;
    line_stream >> opp_jpn;
    line_stream >> opp_eng;

    // Sort typing.
    if (type_raw == "T") {
      type = Card::TADOUSHI;
    } else if (type_raw == "J") {
      type = Card::JIDOUSHI;
    } else {
      err_log(std::format("Invalid type {} for word {}", type_raw, word));
      return;
    }

    // We've gotten to the end, so we are healthy.
    status = HEALTHY;
  }

  // Returns the status of the card.
  Status get_status() { return status; }
};

// Memory representation of the `deck.doji` file.
//
// It's basically just a set of cards.
struct Deck {
private:
  Status status;
  std::vector<Card> cards;

public:
  // They taught us that RAII principles are good in school!
  //
  // So we should just hotload the deck whenever we create it,
  // I guess. I can't imagine a circumstance where this ends
  // up being inefficient.
  Deck() : status(UNHEALTHY), cards() {
    // Open deck file
    if (!std::filesystem::exists("deck.doji")) {
      err_log("`deck.doji` does not exist");
      return;
    }
    std::ifstream file("deck.doji");
    if (!file) {
      err_log("`deck.doji` could not be opened");
      return;
    }

    // Parse deck into cards.
    //
    // In the current setup of `deck.doji`, each line is a card
    // because I am using a custom format.
    //
    // In this case, we split on lines and then parse each card
    // from there.
    std::string curr_line;
    while (std::getline(file, curr_line)) {
      // We have a card, so let's parse it!
      Card c(curr_line);
      if (c.get_status() != HEALTHY) {
        err_log(std::format("Card from \"{}\" not healthy.", curr_line));
        return;
      }

      // Add to the list.
      //
      // We can just append, we will deal with randomization/sorting later.
      cards.push_back(c);
    }

    // Finish deck init proc.
    status = HEALTHY;
    return;
  }

  // Returns the status of the deck.
  //
  // In reality this is either `HEALTHY` or `UNHEALTHY`,
  // where the conventions are quite self-explanatory.
  Status get_status() { return status; }
};

}; // namespace doji

// ~~ magic ~~
int main(void) {
  // Init the deck in memory.
  doji::Deck d;
  if (d.get_status() != doji::HEALTHY) {
    return -1;
  }

  initscr();
  printw("Hello, world!");
  refresh();
  getch();
  endwin();
  return 0;
}
