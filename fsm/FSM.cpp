#include "./FSM.hpp"

#include <unordered_map>

// holds transitions and accept flag for a single state
struct State {
  std::unordered_map<char, StateID> transitions;
  bool is_end = false;
};

// pimpl struct: keeps implementation details out of the header
struct FSM::Impl {
  std::unordered_map<StateID, State> states;
  bool start_anchored = false;
  bool end_anchored = false;
};

FSM::FSM() : m_impl(new Impl()) {
  m_impl->states[0] = State{};
}

FSM::~FSM() {
  delete m_impl;
}

FSM::FSM(const FSM& other) : m_impl(new Impl(*other.m_impl)) {}

FSM& FSM::operator=(const FSM& other) {
  if (this == &other) {
    return *this;
  }
  delete m_impl;
  m_impl = new Impl(*other.m_impl);
  return *this;
}

FSM::FSM(FSM&& other) noexcept : m_impl(other.m_impl) {
  other.m_impl = nullptr;
}

FSM& FSM::operator=(FSM&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  delete m_impl;
  m_impl = other.m_impl;
  other.m_impl = nullptr;
  return *this;
}

bool FSM::AddState(StateID id) {
  if (m_impl->states.count(id) > 0) {
    return false;
  }
  m_impl->states[id] = State{};
  return true;
}

bool FSM::AddEdge(StateID from, char edge, StateID to) {
  if ((m_impl->states.count(from) == 0) || (m_impl->states.count(to) == 0)) {
    return false;
  }
  auto& transitions = m_impl->states[from].transitions;
  if (transitions.count(edge) > 0) {
    return false;
  }
  transitions[edge] = to;
  return true;
}

bool FSM::MarkEndState(StateID id) {
  if (m_impl->states.count(id) == 0) {
    return false;
  }
  m_impl->states[id].is_end = true;
  return true;
}

void FSM::MarkStartAnchor(bool anchored) {
  m_impl->start_anchored = anchored;
}

void FSM::MarkEndAnchor(bool anchored) {
  m_impl->end_anchored = anchored;
}

// Runs the FSM on input[start..end). Tries exact character match first,
// then falls back to the '.' wildcard. Returns the accept StateID if the
// final state is an accept state, otherwise returns nullopt.
static std::optional<StateID> RunFSM(
    const std::unordered_map<StateID, State>& states,
    const std::string& input,
    size_t start,
    size_t end) {
  StateID current = 0;
  for (size_t i = start; i < end; i++) {
    const char c = input[i];
    const auto& transitions = states.at(current).transitions;
    auto it = transitions.find(c);
    if (it == transitions.end()) {
      it = transitions.find('.');
    }
    if (it == transitions.end()) {
      return std::nullopt;
    }
    current = it->second;
  }
  if (states.at(current).is_end) {
    return current;
  }
  return std::nullopt;
}

// Searches all valid substrings of state_changes for a match.
// start/end anchors restrict which substrings are considered.
std::optional<StateID> FSM::Match(const std::string& state_changes) {
  const auto& states = m_impl->states;
  const size_t len = state_changes.size();

  const size_t start_end = m_impl->start_anchored ? 1 : len + 1;
  const size_t end_begin = m_impl->end_anchored ? len : 0;

  for (size_t start = 0; start < start_end; start++) {
    const size_t inner_start = (end_begin > start) ? end_begin : start;
    for (size_t end = inner_start; end <= len; end++) {
      const auto result = RunFSM(states, state_changes, start, end);
      if (result.has_value()) {
        return result;
      }
    }
  }
  return std::nullopt;
}
