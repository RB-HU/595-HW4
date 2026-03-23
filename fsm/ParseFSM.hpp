#ifndef PARSE_FSM_HPP_
#define PARSE_FSM_HPP_

#include <optional>
#include <string>

#include "./FSM.hpp"

std::optional<FSM> ParseFSM(const std::string& data);

#endif  // PARSE_FSM_HPP_
