#ifndef FSM_HPP_
#define FSM_HPP_

#include <optional>
#include <string>

// "Define" a new type called StateID
using StateID = uint64_t;

///////////////////////////////////////////////////////////////////////////////
// FSM is a class to represent a Finite State Machine.
///////////////////////////////////////////////////////////////////////////////
class FSM {
 public:
  // 0-arg Constructor:
  // constructs a Finite State Machine that starts with a single state:
  // the start state, which will have a StateID value of 0
  FSM();

  // Destructor:
  // destructs the Finite State Machine
  ~FSM();

  // Adds a state with the specified StateID to the FSM
  // returns false if a state with the specified ID already exists.
  // returns true if the state didn't exist before and is properly added.
  bool AddState(StateID id);

  // Adds an edge going from the `from` state to the `to` state on input `edge`.
  // returns false if either `from` or `to` doesn't exist, or there is already
  // an identical edge in the FSM. returns true on success
  bool AddEdge(StateID from, char edge, StateID to);

  // Marks the specified State as being an "end" or "accept" state.
  // returns true on success, or false on error (the specified state doesn't
  // exist)
  bool MarkEndState(StateID id);

  // Match looks to see if the FSM can reach an accept state from start state
  // based on the inputs specified in `state_changes`
  //
  // Notably, your Match function should "succeed" if any substring of
  // `state_changes` would result in reaching an accept state from the start
  // state. This is explained in more detail in the assignment specification.
  //
  // Argument:
  // - state_changes: a string where each character is an input to the FSM to
  // prompt a change in state
  //
  // Return Value:
  // - std::nullopt if no end state was reached
  // - the StateID of the first accept state reached during the Match()
  std::optional<StateID> Match(const std::string& state_changes);

  // Marks the FSM to either anchor to the start or disable
  // anchoring to the start.
  //
  // a FSM that is start anchored will only match on a state_changes string
  // that can reach the accept state starting from the first character of the
  // state_changes string. In other words: Match will only look at substrings
  // that start at the beginning of the state_changes string.
  //
  // Argument:
  // - anchored: whether the start should be anchored or not
  void MarkStartAnchor(bool anchored);

  // Marks the FSM to either anchor to the end or disable anchoring to the end.
  //
  // a FSM that is end anchored will only match on a state_changes string
  // that can reach the accept state with the last transtion being from the
  // last character of the state_changes string.
  // In other words: Match will only look at substrings
  // that end at the end of the state_changes string.
  //
  // Argument:
  // - anchored: whether the start should be anchored or not
  void MarkEndAnchor(bool anchored);

 private:
  // TODO: add fields here
  // can also add helper functions or helper structs here if you like.
};

#endif  // FSM_HPP_
