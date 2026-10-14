#include "Player.hpp" // members Card, Suit, MAX_HAND_SIZE
#include <algorithm>  // ONLY for sort()
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

/*
We can write some helper functions and make them static.
spec says: "Write helper functions in Card.cpp, Pack.cpp, and Player.cpp as non-member functions in the .cpp files and declare them static"
for human player we have to have a helper function, the spec says so: "Add a private member function that prints out the human player's hand."

remember to only use the algorithm library for sort() only.
*/

// helper functions below //

// what is this for
// this returns the num of cards in hand that are face or ace of the trump suit
// specifically (is_trump also counts the left bower) - sue
static int count_face_trump(const vector<Card> &hand, Suit trump) {
  int count = 0;
  for (size_t i = 0; i < hand.size(); ++i) {
    if (hand[i].is_face_or_ace() && hand[i].is_trump(trump)) {
      ++count;
    }
  }
  return count;
}

//what is this for
// this one removes the card at the index from hand and then returns it - sue
static Card remove_card(vector<Card> &hand, size_t i) {
  const Card removed = hand[i]; // save the card first, erase destroys it
  hand.erase(hand.begin() + static_cast<int>(i));
  return removed;
}

// ** simple player ** //

class SimplePlayer : public Player {
public:
  SimplePlayer(const string &name_in) : name(name_in) {}

  const string & get_name() const override {
    return name;
  }

  void add_card(const Card &c) override {
    assert(static_cast<int>(hand.size()) < MAX_HAND_SIZE);
    hand.push_back(c);
  } 

  // Error message: create rme for this &upward
  //REQUIRES round is 1 or 2
  //MODIFIES order_up_suit
  //EFFECTS If Player wishes to order up a trump suit then return true and
  //  change order_up_suit to desired suit.  If Player wishes to pass, then do
  //  not modify order_up_suit and return false.
  bool make_trump(const Card &upcard, bool is_dealer,
                  int round, Suit &order_up_suit) const override {
    assert(round == 1 || round == 2);

    Suit suit_to_check = upcard.get_suit();
    int cards_needed = 2;

    if (round == 2) {
      suit_to_check = Suit_next(upcard.get_suit());
      cards_needed = 1;
    }

    bool screw_the_dealer = (round == 2 && is_dealer);

    if (count_face_trump(hand, suit_to_check) >= cards_needed
        || screw_the_dealer) {
      order_up_suit = suit_to_check;
      return true;
    }
    return false;
  }

  void add_and_discard(const Card &upcard) override {
    assert(!hand.empty());
    const Suit trump = upcard.get_suit();
    hand.push_back(upcard);

    size_t lowest = 0;
    for (size_t i = 1; i < hand.size(); ++i) {
      if (Card_less(hand[i], hand[lowest], trump)) {
        lowest = i;
      }
    }
    remove_card(hand, lowest);
  }

  Card lead_card(Suit trump) override {
    assert(!hand.empty());
    const size_t none = hand.size(); 

    size_t best = none;
    for (size_t i = 0; i < hand.size(); ++i) {
      if (!hand[i].is_trump(trump) &&
          (best == none || Card_less(hand[best], hand[i], trump))) {
        best = i;
      }
    }

    if (best == none) {
      best = 0;
      for (size_t i = 1; i < hand.size(); ++i) {
        if (Card_less(hand[best], hand[i], trump)) {
          best = i;
        }
      }
    }
    return remove_card(hand, best);
  }

  Card play_card(const Card &led_card, Suit trump) override {
    assert(!hand.empty());
    const Suit led_suit = led_card.get_suit(trump);  
    const size_t none = hand.size();                  

    // highest card that follows suit
    size_t best = none;
    for (size_t i = 0; i < hand.size(); ++i) {
      if (hand[i].get_suit(trump) == led_suit &&
          (best == none ||
           Card_less(hand[best], hand[i], led_card, trump))) {
        best = i;
      }
    }

    if (best == none) {
      best = 0;
      for (size_t i = 1; i < hand.size(); ++i) {
        if (Card_less(hand[i], hand[best], led_card, trump)) {
          best = i;
        }
      }
    }
    return remove_card(hand, best);
  }

private:
  string name;
  vector<Card> hand;
};

// ** human player ** //

class HumanPlayer : public Player {
public:
  HumanPlayer(const string &name_in) : name(name_in) {}

  const string & get_name() const override {
    return name;
  }

  void add_card(const Card &c) override {
    assert(static_cast<int>(hand.size()) < MAX_HAND_SIZE);
    hand.push_back(c);
    sort(hand.begin(), hand.end());   // keep hand sorted; not in a const function
  }

  bool make_trump(const Card &, bool, int round,
                  Suit &order_up_suit) const override {
    assert(round == 1 || round == 2);
    print_hand();
    cout << "Human player " << name << ", please enter a suit, or \"pass\":\n";

    string decision;
    cin >> decision;

    if (decision != "pass") {
      order_up_suit = string_to_suit(decision);
      return true;
    }
    return false;
  }

  void add_and_discard(const Card &upcard) override {
    assert(!hand.empty());
    print_hand();
    cout << "Discard upcard: [-1]\n";
    cout << "Human player " << name << ", please select a card to discard:\n";

    int choice;
    cin >> choice;

    if (choice != -1) {
      remove_card(hand, static_cast<size_t>(choice));
      add_card(upcard);  
    }
  }

  Card lead_card(Suit) override {
    return choose_card();
  }

  Card play_card(const Card &, Suit) override {
    return choose_card();
  }

private:
  string name;
  vector<Card> hand;

  // prints each card in hand with its index
  void print_hand() const {
    for (size_t i = 0; i < hand.size(); ++i) {
      cout << "Human player " << name << "'s hand: "
           << "[" << i << "] " << hand[i] << "\n";
    }
  }

  Card choose_card() {
    print_hand();
    cout << "Human player " << name << ", please select a card:\n";

    int choice;
    cin >> choice;
    return remove_card(hand, static_cast<size_t>(choice));
  }
};


// ** factory and operator ** //

Player * Player_factory(const string &name, const string &strategy) {
  if (strategy == "Simple") {
    return new SimplePlayer(name);
  }
  if (strategy == "Human") {
    return new HumanPlayer(name);
  }
  assert(false);
  return nullptr;
}

ostream & operator<<(ostream &os, const Player &p) {
  os << p.get_name();
  return os;
}