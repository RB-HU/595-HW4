#include <iostream>
#include <string>
#include <vector>

#include "./FSM.hpp"
#include "./ParseFSM.hpp"
#include "./catch.hpp"

using std::cerr;
using std::endl;
using std::nullopt;
using std::optional;
using std::string;
using std::vector;

// test cases

TEST_CASE("Binary Even Length", "[Test_FSM]") {
  // All binary strings of even length
  const vector<string> tests = {
      "101100101101",    "0",        "",
      "101101110111011", "10111011", "00000000000000000010"};

  const vector<bool> answers = {true, false, true, false, true, true};

  // construct the FSM
  FSM fsm;
  fsm.MarkStartAnchor(true);
  fsm.MarkEndAnchor(true);
  fsm.AddState(1);
  fsm.AddEdge(0, '0', 1);
  fsm.AddEdge(0, '1', 1);
  fsm.AddEdge(1, '0', 0);
  fsm.AddEdge(1, '1', 0);
  fsm.MarkEndState(0);

  cerr << "Testing FSM matching all even length binary strings" << endl;
  for (size_t i = 0; i < tests.size(); ++i) {
    cerr << "\tTesting Input: " << tests.at(i) << endl;
    auto opt = fsm.Match(tests.at(i));
    REQUIRE(answers.at(i) == opt.has_value());
    if (opt.has_value()) {
      REQUIRE(opt.value() == 0);
    }
  }
}

TEST_CASE("Binary Even 1's", "[Test_FSM]") {
  // All binary strings with an even number of 1's
  const vector<string> tests = {
      "101100101101",    "0",        "",
      "101101110111011", "10111011", "000000000000000000110"};

  const vector<bool> answers = {false, true, true, false, true, true};

  // construct the FSM
  FSM fsm;
  fsm.MarkStartAnchor(true);
  fsm.MarkEndAnchor(true);
  fsm.AddState(3034);
  fsm.AddEdge(0, '0', 0);
  fsm.AddEdge(0, '1', 3034);
  fsm.AddEdge(3034, '0', 3034);
  fsm.AddEdge(3034, '1', 0);
  fsm.MarkEndState(0);

  cerr << "Testing FSM matching all binary strings with an even number of 1's"
       << endl;
  for (size_t i = 0; i < tests.size(); ++i) {
    cerr << "\tTesting Input: " << tests.at(i) << endl;
    auto opt = fsm.Match(tests.at(i));
    REQUIRE(answers.at(i) == opt.has_value());
    if (opt.has_value()) {
      REQUIRE(opt.value() == 0);
    }
  }
}

TEST_CASE("Binary with 11", "[Test_FSM]") {
  // All binary strings containing 11
  const vector<string> tests = {
      "101100101101",    "0",        "",
      "101101110111011", "10111011", "000000000000000000110"};

  const vector<bool> answers = {true, false, false, true, true, true};

  // construct the FSM
  FSM fsm;
  fsm.MarkStartAnchor(true);
  fsm.MarkEndAnchor(true);
  REQUIRE(fsm.AddState(4562));
  REQUIRE(fsm.AddState(8085));
  REQUIRE(fsm.AddEdge(0, '0', 0));
  REQUIRE(fsm.AddEdge(0, '1', 8085));
  REQUIRE(fsm.AddEdge(8085, '0', 0));
  REQUIRE(fsm.AddEdge(8085, '1', 4562));
  REQUIRE(fsm.AddEdge(4562, '0', 4562));
  REQUIRE(fsm.AddEdge(4562, '1', 4562));
  REQUIRE(fsm.MarkEndState(4562));

  cerr << "Testing FSM matching all binary strings containing 11" << endl;
  for (size_t i = 0; i < tests.size(); ++i) {
    cerr << "\tTesting Input: " << tests.at(i) << endl;
    auto opt = fsm.Match(tests.at(i));
    REQUIRE(answers.at(i) == opt.has_value());
    if (opt.has_value()) {
      REQUIRE(opt.value() == 4562);
    }
  }
}

TEST_CASE("Binary with 11 & start anchor", "[Test_FSM]") {
  // All binary strings containing 11
  const vector<string> tests = {
      "101100101101",    "0",        "",
      "101101110111011", "10111011", "000000000000000000110"};

  const vector<bool> answers = {true, false, false, true, true, true};

  // construct the FSM
  FSM fsm;
  fsm.MarkStartAnchor(true);
  REQUIRE(fsm.AddState(4562));
  REQUIRE(fsm.AddState(8085));
  REQUIRE(fsm.AddEdge(0, '0', 0));
  REQUIRE(fsm.AddEdge(0, '1', 8085));
  REQUIRE(fsm.AddEdge(8085, '0', 0));
  REQUIRE(fsm.AddEdge(8085, '1', 4562));
  REQUIRE(fsm.MarkEndState(4562));

  cerr << "Testing FSM matching all binary strings containing 11" << endl;
  cerr << "And only with a start anchor, no end anchor" << endl;
  for (size_t i = 0; i < tests.size(); ++i) {
    cerr << "\tTesting Input: " << tests.at(i) << endl;
    auto opt = fsm.Match(tests.at(i));
    REQUIRE(answers.at(i) == opt.has_value());
    if (opt.has_value()) {
      REQUIRE(opt.value() == 4562);
    }
  }
}

TEST_CASE("Binary with 11 & no anchor", "[Test_FSM]") {
  // All binary strings containing 11
  const vector<string> tests = {
      "101100101101",    "0",        "",
      "101101110111011", "10111011", "000000000000000000110"};

  const vector<bool> answers = {true, false, false, true, true, true};

  // construct the FSM
  FSM fsm;
  REQUIRE(fsm.AddState(2400));
  REQUIRE(fsm.AddState(3034));
  REQUIRE(fsm.AddEdge(0, '1', 2400));
  REQUIRE(fsm.AddEdge(2400, '1', 3034));
  REQUIRE(fsm.MarkEndState(3034));

  cerr << "Testing FSM matching all binary strings containing 11" << endl;
  cerr << "And no anchors at all" << endl;
  for (size_t i = 0; i < tests.size(); ++i) {
    cerr << "\tTesting Input: " << tests.at(i) << endl;
    auto opt = fsm.Match(tests.at(i));
    REQUIRE(answers.at(i) == opt.has_value());
    if (opt.has_value()) {
      REQUIRE(opt.value() == 3034);
    }
  }
}

TEST_CASE("Binary equal 01 and 10", "[Test_FSM]") {
  // All binary strings containing an equal amount of 01's and 10's
  const vector<string> tests = {
      "101100101101",      "0",        "",
      "10110111011101110", "10111010", "000000000000000000110"};

  const vector<optional<StateID>> answers = {
      4, 2, 0, nullopt, nullopt, 2,
  };

  // construct the FSM
  FSM fsm;
  fsm.MarkStartAnchor(true);
  fsm.MarkEndAnchor(true);
  REQUIRE(fsm.AddState(2));
  REQUIRE(fsm.AddState(4));
  REQUIRE(fsm.AddState(6));
  REQUIRE(fsm.AddState(8));
  REQUIRE(fsm.AddEdge(0, '0', 2));
  REQUIRE(fsm.AddEdge(0, '1', 4));
  REQUIRE(fsm.AddEdge(2, '0', 2));
  REQUIRE(fsm.AddEdge(2, '1', 6));
  REQUIRE(fsm.AddEdge(4, '0', 8));
  REQUIRE(fsm.AddEdge(4, '1', 4));
  REQUIRE(fsm.AddEdge(6, '0', 2));
  REQUIRE(fsm.AddEdge(6, '1', 6));
  REQUIRE(fsm.AddEdge(8, '0', 8));
  REQUIRE(fsm.AddEdge(8, '1', 4));
  REQUIRE(fsm.MarkEndState(0));
  REQUIRE(fsm.MarkEndState(2));
  REQUIRE(fsm.MarkEndState(4));

  cerr << "Testing FSM matching all binary strings containing an equal amount "
          "of 01's and 10's"
       << endl;

  for (size_t i = 0; i < tests.size(); ++i) {
    cerr << "\tTesting Input: " << tests.at(i) << endl;
    auto opt = fsm.Match(tests.at(i));
    REQUIRE(answers.at(i).has_value() == opt.has_value());
    if (opt.has_value()) {
      REQUIRE(answers.at(i).value() == opt.value());
    }
  }
}

TEST_CASE("Smileys", "[Test_FSM]") {
  // All binary strings containing an equal amount of 01's and 10's
  const vector<string> tests = {":B", "X3",       "8(",  ":-,/", "='''''''''|",
                                "(:", ":smiley:", ":\\", ":D",   "83"};

  const vector<optional<StateID>> answers = {
      nullopt, 2, 2, nullopt, nullopt, nullopt, nullopt, 2, 2, 2,
  };

  // construct the FSM
  auto opt = ParseFSM("^[:;=X8][)D(O03VCS/\\|><P]$");
  REQUIRE(opt.has_value());
  FSM fsm = opt.value();

  cerr << "Testing FSM matching various smiley faces" << endl;

  for (size_t i = 0; i < tests.size(); ++i) {
    cerr << "\tTesting Input: " << tests.at(i) << endl;
    auto opt = fsm.Match(tests.at(i));
    REQUIRE(answers.at(i).has_value() == opt.has_value());
    if (opt.has_value()) {
      REQUIRE(answers.at(i).value() == opt.value());
    }
  }
}

TEST_CASE("cpp extension", "[Test_FSM]") {
  // All strings ending with cpp or hpp
  const vector<string> tests = {"dean.py",
                                "I LOVE MY COMPUTER",
                                "ninajirachi",
                                "hat.h",
                                "cap.cpp",
                                "test_fsm.cpp",
                                "cpp",
                                "bucket.hpp",
                                "hpp",
                                "final_file.cpp.cpp",
                                "please(2).cpp.pdf",
                                "cpp_is_better.py"};

  const vector<optional<StateID>> answers = {
      nullopt, nullopt, nullopt, nullopt, 4,       4,
      nullopt, 4,       nullopt, 4,       nullopt, nullopt};

  // construct the FSM
  auto opt = ParseFSM(".[ch]pp$");
  REQUIRE(opt.has_value());
  FSM fsm = opt.value();

  cerr << "Testing FSM files ending in cpp or hpp" << endl;

  for (size_t i = 0; i < tests.size(); ++i) {
    cerr << "\tTesting Input: " << tests.at(i) << endl;
    auto opt = fsm.Match(tests.at(i));
    REQUIRE(answers.at(i).has_value() == opt.has_value());
    if (opt.has_value()) {
      REQUIRE(answers.at(i).value() == opt.value());
    }
  }
}

TEST_CASE("courses", "[Test_FSM]") {
  // Strings that roughly match format for a upenn course code
  const vector<string> tests = {
      "rees 0275",
      "CIS3990",
      "cis 3034",
      "EE 539",
      "PHIL 1439",
      "ANTH 2620",
      "cbe 4215",
      "cse333",
      "writing 0760",
      "Capitalism, Socialism, and Crisis in the 20th Century Americas",
      "CIT 5950",
      "CSCE 52840"};

  const vector<optional<StateID>> answers = {
      8, nullopt, 8, nullopt, 8, 8, 8, nullopt, 8, nullopt, 8, nullopt};

  // construct the FSM
  auto opt = ParseFSM("... [01234567][0123456789][0123456789][0123456789]$");
  REQUIRE(opt.has_value());
  FSM fsm = opt.value();

  cerr << "Testing FSM that matches strings that look roughly like courses"
       << endl;

  for (size_t i = 0; i < tests.size(); ++i) {
    cerr << "\tTesting Input: " << tests.at(i) << endl;
    auto opt = fsm.Match(tests.at(i));
    REQUIRE(answers.at(i).has_value() == opt.has_value());
    if (opt.has_value()) {
      REQUIRE(answers.at(i).value() == opt.value());
    }
  }
}
