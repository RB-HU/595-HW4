#include "./ParseFSM.hpp"

using std::nullopt;
using std::optional;
using std::string;

static bool LegalFSM(const string& data);

optional<FSM> ParseFSM(const string& data) {
  if (data.empty()) {
    return nullopt;
  }

  FSM result;
  string to_parse = data;

  if (to_parse.front() == '^') {
    to_parse.erase(0, 1);  // erase one character starting at index 0
    if (to_parse.empty()) {
      return nullopt;
    }
    result.MarkStartAnchor(true);
  }

  if (to_parse.back() == '$') {
    to_parse.pop_back();
    if (to_parse.empty()) {
      return nullopt;
    }
    result.MarkEndAnchor(true);
  }

  // check for legal amount of []
  if (!LegalFSM(to_parse)) {
    return nullopt;
  }

  StateID curr_id = 0;

  auto it = to_parse.begin();

  while (it != to_parse.end()) {
    char curr_char = *it;
    const StateID next_id = curr_id + 1;
    result.AddState(next_id);

    if (curr_char == '[') {
      ++it;
      curr_char = *it;
      while (curr_char != ']') {
        result.AddEdge(curr_id, curr_char, next_id);
        ++it;
        curr_char = *it;
      }
      ++it;  // move past the ]
    } else {
      result.AddEdge(curr_id, curr_char, next_id);
      ++it;
    }
    curr_id = next_id;
  }

  result.MarkEndState(curr_id);

  return result;
}

static bool LegalFSM(const string& data) {
  size_t counter = 0;
  for (const char c : data) {
    // start and end anchor
    if (c == '$' || c == '^') {
      return false;
    }

    // unspported characters
    if (c == '*' || c == '+' || c == '?') {
      return false;
    }

    // make sure that brackets are legal
    if (c == '[') {
      counter += 1;
      if (counter != 1) {
        // does not support nested [] at this time
        return false;
      }
    }
    if (c == ']') {
      if (counter == 0) {
        return false;
      }
      counter -= 1;
    }
  }

  return counter == 0;
}
