#include <cassert>
#include <clocale>
#include <csignal>
#include <ctime>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <ncurses.h>
#include <sstream>
#include <string>
#include <vector>

namespace doji {

// Because error logging is useful sometimes...
void err_log(std::string l) {
  std::cerr << "\033[31m[ERR]\033[0m\t" << l << std::endl;
}

// Simple statuses.
//
// Something is either OK or not OK. :3
enum Status { HEALTHY, UNHEALTHY };

// Memory representation of a single flashcard.
//
// Cards just have properties...
struct Card {
public: // Encapsulation is kinda dumb here I think...
  Status status;
  enum { TADOUSHI, JIDOUSHI } type;
  std::string word;
  std::string opp_jpn;
  std::string opp_eng;

  // Nothing burger constructor.
  Card() : status(UNHEALTHY) {}

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

  std::string type_to_string() {
    return type == TADOUSHI ? "TADOUSHI" : "JIDOUSHI";
  }

  void print() {
    std::string type_raw = type == TADOUSHI ? "T" : "J";
    std::cout << word << " " << type_raw << " " << opp_jpn << " " << opp_eng
              << std::endl;
  }
};

// Memory representation of the `deck.doji` file.
//
// It's basically just a set of cards.
struct Deck {
public:
  Status status;
  std::vector<Card> cards;

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
      if (c.status != HEALTHY) {
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
};

struct State {
public:
  Deck *deck;
  bool *card_used;
  size_t unused;

  State(Deck *deck) : deck(deck) {
    // Init all cards to unused.
    size_t num_cards = deck->cards.size();
    card_used = new bool[num_cards];
    for (size_t i = 0; i < num_cards; ++i)
      card_used[i] = false;
    unused = num_cards;

    // Randomize the state of the deck.
    std::srand(std::time(0));
  }

  // Determines if there is an unused card to give the user.
  bool have_next_card() { return unused != 0; }

  // Gets a random, unused card.
  Card get_next_card() {
    // Check if we are out of cards.
    // If so, the only sane thing is to return that we have no cards.
    // We can signal this via an unhealthy card.
    if (unused == 0) {
      // NOTE: This card is unhealthy by virtue.
      return Card();
    }

    // Generate a random number in the range [0, `unused`).
    size_t order = (size_t)std::rand() % unused;

    // Find the `order` unused card.
    //
    // This is linear with the size of the deck, but that's gonna be
    // pretty small for reasonable usecases of this program so thats
    // okie dokie.
    size_t target = 0;
    for (size_t i = 0; i < deck->cards.size(); ++i) {
      if (!card_used[i]) {
        order--;
      }
      if (order == 0) {
        target = i;
        break;
      }
    }

    // Update metadata
    Card next = deck->cards[target];
    card_used[target] = true;
    unused--;

    // o7
    return next;
  }
};

}; // namespace doji

// In case we need to wrap-up early.
//
// That is, a user interrupts `doji`.
static void heat_death(int signum) {
  endwin();
  exit(0);
}

int main(void) {
  // Init the deck in memory.
  doji::Deck d;
  if (d.status != doji::HEALTHY) {
    return -1;
  }

  // Init the state.
  doji::State s(&d);

  // The heat death of the universe.
  signal(SIGINT, heat_death);

  // Init the TUI.
  // https://stackoverflow.com/questions/9922528/how-to-make-ncurses-display-utf-8-chars-correctly-in-c
  setlocale(LC_CTYPE, "");
  initscr();
  printw("ドジ\nby migopp\n\nPress any key to continue...\n");
  refresh();
  getch(); // Wait.

  // `doji` loop here.
  //
  // The core gameplay loop is as follows:
  // 	1. Fetch an unused card + mark it as used
  // 	2. Feed the player info, ask for information
  // 	3. Mark answer correct/incorrect
  // 	4. End once all cards have been used
  while (s.have_next_card()) {
    // Reset the screen.
    clear();
    refresh();

    // Fetch the next card.
    doji::Card c = s.get_next_card();

    // Display info.
    printw(c.word.c_str());
    refresh();
    getch();

    // Display answer.
    printw(c.word.c_str());
    printw("\n");
    printw(c.type_to_string().c_str());
    printw("\n");
    printw(c.opp_jpn.c_str());
    printw("\n");
    printw(c.opp_eng.c_str());
    refresh();
    getch();
  }

  // GG.
  heat_death(0);
}
