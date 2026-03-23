#ifndef FSM_HPP_
#define FSM_HPP_

#include <optional>
#include <string>

using StateID = uint64_t;

///////////////////////////////////////////////////////////////////////////////
// FSM is a class to represent a Finite State Machine.
///////////////////////////////////////////////////////////////////////////////
class FSM {
 public:
  // constructs an FSM with a single start state (StateID 0)
  FSM();

  ~FSM();

  // rule of 5
  FSM(const FSM& other);
  FSM& operator=(const FSM& other);
  FSM(FSM&& other) noexcept;
  FSM& operator=(FSM&& other) noexcept;

  // Adds a state; returns false if it already exists
  bool AddState(StateID id);

  // Adds a transition from->to on input edge;
  // returns false if either state doesn't exist or edge already exists
  bool AddEdge(StateID from, char edge, StateID to);

  // Marks a state as an accept state; returns false if state doesn't exist
  bool MarkEndState(StateID id);

  // Returns the StateID of the first accept state reached, or nullopt.
  // Checks all substrings of state_changes unless anchors are set.
  std::optional<StateID> Match(const std::string& state_changes);

  // If anchored, only consider substrings starting at index 0
  void MarkStartAnchor(bool anchored);

  // If anchored, only consider substrings ending at the last character
  void MarkEndAnchor(bool anchored);

 private:
  struct Impl;   // forward declare pimpl struct (defined in FSM.cpp)
  Impl* m_impl;  // pointer to implementation
};

#endif  // FSM_HPP_
