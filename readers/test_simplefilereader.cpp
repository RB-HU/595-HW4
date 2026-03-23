#include "./SimpleFileReader.hpp"
#include "catch.hpp"
#include <errno.h>
#include <fstream>
#include <string>
#include <sys/select.h>
#include <unistd.h>

using namespace std;

static constexpr const char *kHelloFileName = "./test_files/Hello.txt";
static constexpr const char *kByeFileName = "./test_files/Bye.txt";
static constexpr const char *kLongFileName = "./test_files/war_and_peace.txt";
static constexpr const char *kGreatFileName = "./test_files/mutual_aid.txt";

TEST_CASE("Basic", "[Test_SimpleFileReader]") {
  SimpleFileReader *sf = new SimpleFileReader(kHelloFileName);
  char c = sf->GetChar();
  REQUIRE('H' == c);

  // Delete SF to make sure destructor works, then award points
  // if it doesn't crash
  delete sf;
}

TEST_CASE("OpenClose", "[Test_SimpleFileReader]") {
  // close when already closed
  SimpleFileReader *sf = new SimpleFileReader(kHelloFileName);
  REQUIRE(sf->Good());
  sf->CloseFile();
  REQUIRE_FALSE(sf->Good());
  sf->CloseFile();
  REQUIRE_FALSE(sf->Good());
  // open when already opened
  sf->OpenFile(kByeFileName);
  REQUIRE(sf->Good());
  sf->OpenFile(kByeFileName);
  REQUIRE(sf->Good());
  sf->OpenFile(kByeFileName);
  REQUIRE(sf->Good());

  // open and close the same file over and over again
  for (size_t i = 0; i < 10; i++) {
    sf->OpenFile(kByeFileName);
    REQUIRE(sf->Good());
    sf->CloseFile();
    REQUIRE_FALSE(sf->Good());
    sf->CloseFile();
    REQUIRE_FALSE(sf->Good());
    sf->OpenFile(kByeFileName);
    REQUIRE(sf->Good());
  }
  // destructor on an already closed file
  delete sf;
}

TEST_CASE("GetChar", "[Test_SimpleFileReader]") {

  // file contents
  string kHelloContents{};
  string kByeContents{};
  string kLongContents{};
  string kGreatContents{};

  // flil the above strings with the actual contents of the file
  ifstream hello_ifs(kHelloFileName);
  kHelloContents.assign((std::istreambuf_iterator<char>(hello_ifs)),
                        (std::istreambuf_iterator<char>()));
  ifstream bye_ifs(kByeFileName);
  kByeContents.assign((std::istreambuf_iterator<char>(bye_ifs)),
                      (std::istreambuf_iterator<char>()));
  ifstream long_ifs(kLongFileName);
  kLongContents.assign((std::istreambuf_iterator<char>(long_ifs)),
                       (std::istreambuf_iterator<char>()));
  ifstream great_ifs(kGreatFileName);
  kGreatContents.assign((std::istreambuf_iterator<char>(great_ifs)),
                        (std::istreambuf_iterator<char>()));

  // Hello test case
  SimpleFileReader sf(kHelloFileName);
  string contents;
  char c;
  for (size_t i = 0; i < kHelloContents.length(); i++) {

    REQUIRE(sf.Tell() >= 0);
    REQUIRE(i == static_cast<size_t>(sf.Tell()));
    c = sf.GetChar();
    contents += c;
    REQUIRE(sf.Good());
  }
  REQUIRE(kHelloContents == contents);
  c = sf.GetChar();
  REQUIRE(static_cast<char>(EOF) == c);
  REQUIRE_FALSE(sf.Good());

  // Goodbye test case
  sf.CloseFile();
  sf.OpenFile(kByeFileName);
  contents.clear();
  for (size_t i = 0; i < kByeContents.length(); i++) {

    REQUIRE(sf.Tell() >= 0);
    REQUIRE(i == static_cast<size_t>(sf.Tell()));
    c = sf.GetChar();
    contents += c;
    REQUIRE(sf.Good());
  }
  REQUIRE(kByeContents == contents);
  c = sf.GetChar();
  REQUIRE(static_cast<char>(EOF) == c);
  REQUIRE_FALSE(sf.Good());
  REQUIRE(static_cast<size_t>(sf.Tell()) == kByeContents.length());

  // Long file test case
  contents.clear();
  sf.CloseFile();
  sf.OpenFile(kLongFileName);
  contents.reserve(kLongContents.length());
  for (size_t i = 0; i < kLongContents.length(); i++) {

    REQUIRE(sf.Tell() >= 0);
    REQUIRE(i == static_cast<size_t>(sf.Tell()));
    c = sf.GetChar();
    contents += c;
    REQUIRE(sf.Good());
  }
  REQUIRE(kLongContents == contents);
  c = sf.GetChar();
  REQUIRE(static_cast<char>(EOF) == c);
  REQUIRE_FALSE(sf.Good());
  REQUIRE(static_cast<size_t>(sf.Tell()) == kLongContents.length());

  // "Great" file test case
  contents.clear();
  sf.CloseFile();
  sf.OpenFile(kGreatFileName);
  contents.reserve(kGreatContents.length());
  for (size_t i = 0; i < kGreatContents.length(); i++) {

    REQUIRE(sf.Tell() >= 0);
    REQUIRE(i == static_cast<size_t>(sf.Tell()));
    c = sf.GetChar();
    contents += c;
    REQUIRE(sf.Good());
  }
  REQUIRE(kGreatContents == contents);
  c = sf.GetChar();
  REQUIRE(static_cast<char>(EOF) == c);
  REQUIRE_FALSE(sf.Good());
  REQUIRE(static_cast<size_t>(sf.Tell()) == kGreatContents.length());
}

TEST_CASE("GetChars", "[Test_SimpleFileReader]") {
  // file contents
  string kHelloContents{};
  string kByeContents{};
  string kLongContents{};
  string kGreatContents{};

  // flil the above strings with the actual contents of the file
  ifstream hello_ifs(kHelloFileName);
  kHelloContents.assign((std::istreambuf_iterator<char>(hello_ifs)),
                        (std::istreambuf_iterator<char>()));
  ifstream bye_ifs(kByeFileName);
  kByeContents.assign((std::istreambuf_iterator<char>(bye_ifs)),
                      (std::istreambuf_iterator<char>()));
  ifstream long_ifs(kLongFileName);
  kLongContents.assign((std::istreambuf_iterator<char>(long_ifs)),
                       (std::istreambuf_iterator<char>()));
  ifstream great_ifs(kGreatFileName);
  kGreatContents.assign((std::istreambuf_iterator<char>(great_ifs)),
                        (std::istreambuf_iterator<char>()));

  // Hello test case
  SimpleFileReader sf(kHelloFileName);
  string contents;
  string next;
  optional<string> opt;
  size_t n = 1;

  for (size_t i = 0; i < kHelloContents.length(); i += n, n++) {
    opt = sf.GetChars(n);
    REQUIRE(opt.has_value());
    next = opt.value();
    contents += next;
    if (i + n <= kHelloContents.length()) {
      REQUIRE(sf.Good());
    } else {
      REQUIRE_FALSE(sf.Good());
    }
  }
  REQUIRE(kHelloContents == contents);
  opt = sf.GetChars(1);
  REQUIRE_FALSE(opt.has_value());
  REQUIRE_FALSE(sf.Good());
  REQUIRE(static_cast<size_t>(sf.Tell()) == kHelloContents.length());

  // Bye test case
  contents.clear();
  sf.CloseFile();
  sf.OpenFile(kByeFileName);
  for (size_t i = 0; i < kByeContents.length(); i += n, n++) {
    opt = sf.GetChars(n);
    REQUIRE(opt.has_value());
    next = opt.value();
    contents += next;
    if (i + n <= kByeContents.length()) {
      REQUIRE(sf.Good());
    } else {
      REQUIRE_FALSE(sf.Good());
    }
  }
  REQUIRE(kByeContents == contents);
  opt = sf.GetChars(1);
  REQUIRE_FALSE(opt.has_value());
  REQUIRE_FALSE(sf.Good());
  REQUIRE(static_cast<size_t>(sf.Tell()) == kByeContents.length());

  // Long test case
  contents.clear();
  sf.CloseFile();
  sf.OpenFile(kLongFileName);
  for (size_t i = 0; i < kLongContents.length(); i += n, n++) {
    opt = sf.GetChars(n);
    REQUIRE(opt.has_value());
    next = opt.value();
    contents += next;
    if (i + n <= kLongContents.length()) {
      REQUIRE(sf.Good());
    } else {
      REQUIRE_FALSE(sf.Good());
    }
  }
  REQUIRE(kLongContents == contents);
  opt = sf.GetChars(1);
  REQUIRE_FALSE(opt.has_value());
  REQUIRE_FALSE(sf.Good());
  REQUIRE(static_cast<size_t>(sf.Tell()) == kLongContents.length());

  // Great test case
  contents.clear();
  sf.CloseFile();
  sf.OpenFile(kGreatFileName);
  for (size_t i = 0; i < kGreatContents.length(); i += n, n++) {
    opt = sf.GetChars(n);
    REQUIRE(opt.has_value());
    next = opt.value();
    contents += next;
    if (i + n <= kGreatContents.length()) {
      REQUIRE(sf.Good());
    } else {
      REQUIRE_FALSE(sf.Good());
    }
  }
  REQUIRE(kGreatContents == contents);
  opt = sf.GetChars(1);
  REQUIRE_FALSE(opt.has_value());
  REQUIRE_FALSE(sf.Good());
  REQUIRE(static_cast<size_t>(sf.Tell()) == kGreatContents.length());
}

TEST_CASE("Complex", "[Test_SimpleFileReader]") {
  // file contents
  string kHelloContents{};
  string kByeContents{};
  string kLongContents{};
  string kGreatContents{};

  // flil the above strings with the actual contents of the file
  ifstream hello_ifs(kHelloFileName);
  kHelloContents.assign((std::istreambuf_iterator<char>(hello_ifs)),
                        (std::istreambuf_iterator<char>()));
  ifstream bye_ifs(kByeFileName);
  kByeContents.assign((std::istreambuf_iterator<char>(bye_ifs)),
                      (std::istreambuf_iterator<char>()));
  ifstream long_ifs(kLongFileName);
  kLongContents.assign((std::istreambuf_iterator<char>(long_ifs)),
                       (std::istreambuf_iterator<char>()));
  ifstream great_ifs(kGreatFileName);
  kGreatContents.assign((std::istreambuf_iterator<char>(great_ifs)),
                        (std::istreambuf_iterator<char>()));

  // Hello test case
  SimpleFileReader sf(kHelloFileName);
  string contents;
  string next;
  optional<string> opt;
  char c;
  size_t n = 1;

  for (size_t i = 0; i < 3; i++) {
    for (size_t j = 0; j < kHelloContents.length(); n++) {
      if (j % 2) {
        c = sf.GetChar();
        contents += c;
        j++;
      } else {
        opt = sf.GetChars(n);
        REQUIRE(opt.has_value());
        next = opt.value();
        contents += next;
        j += n;
      }
      if (j <= kHelloContents.length()) {
        REQUIRE(sf.Good());
      } else {
        REQUIRE_FALSE(sf.Good());
      }
    }
    REQUIRE(kHelloContents == contents);
    c = sf.GetChar();
    REQUIRE(static_cast<char>(EOF) == c);
    REQUIRE_FALSE(sf.Good());
    sf.Rewind();
    contents.clear();
  }

  // Great test case

  // clear previous contents of "contents"
  contents.clear();

  // open and close repeatedly
  sf.CloseFile();
  sf.CloseFile();
  REQUIRE(sf.Tell() == -1);
  sf.CloseFile();
  REQUIRE(sf.Tell() == -1);
  sf.OpenFile(kGreatFileName);
  sf.CloseFile();
  REQUIRE(sf.Tell() == -1);
  sf.OpenFile(kGreatFileName);
  sf.CloseFile();
  REQUIRE(sf.Tell() == -1);
  sf.OpenFile(kGreatFileName);

  n = 0;
  for (size_t i = 0; i < kGreatContents.length(); n++) {
    if (!(i % 2)) {
      c = sf.GetChar();
      contents += c;
      i++;
    } else {
      opt = sf.GetChars(n);
      REQUIRE(opt.has_value());
      next = opt.value();
      contents += next;
      i += n;
    }
    if (i <= kGreatContents.length()) {
      REQUIRE(sf.Good());
    } else {
      REQUIRE_FALSE(sf.Good());
    }
  }
  REQUIRE(kGreatContents == contents);
  opt = sf.GetChars(1);
  REQUIRE_FALSE(opt.has_value());
  REQUIRE_FALSE(sf.Good());
  REQUIRE(static_cast<size_t>(sf.Tell()) == kGreatContents.length());
}
