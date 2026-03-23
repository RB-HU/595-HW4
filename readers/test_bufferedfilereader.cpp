#include "./BufferChecker.hpp"
#include "./BufferedFileReader.hpp"
#include "catch.hpp"
#include <errno.h>
#include <fstream>
#include <string>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

static constexpr const char *kHelloFileName = "./test_files/Hello.txt";
static constexpr const char *kByeFileName = "./test_files/Bye.txt";
static constexpr const char *kLongFileName = "./test_files/war_and_peace.txt";
static constexpr const char *kGreatFileName = "./test_files/mutual_aid.txt";
static constexpr const char *kNumbersFileName = "./test_files/numbers.txt";

// helper functions

static bool verify_token(const string &actual, const string &expected_contents,
                         const string &delims, off_t *offset, bool lstrip) {
  off_t off = *offset;

  if (lstrip) {
    while (off < expected_contents.size() && isspace(expected_contents.at(off))) {
      off += 1;
    }
  }

  string expected = expected_contents.substr(off, actual.length());
  if (actual != expected) {
    *offset = off;
    return false;
  }

  off += actual.length();
  if (off >= static_cast<off_t>(expected_contents.length())) {
    // eof reached
    *offset = off;
    return true;
  }

  if (delims.find(expected_contents.at(off)) == string::npos) {
    *offset = off + 1;
    return false;
  }

  // off++;

  *offset = off;
  return true;
}

TEST_CASE("Basic", "[Test_BufferedFileReader]") {
  BufferedFileReader *bf = new BufferedFileReader(kHelloFileName);
  char c = bf->GetChar();
  REQUIRE('H' == c);

  // Check that the buffer has 'h' in it
  BufferChecker bc(*bf);
  string expected("H");
  REQUIRE_FALSE(bc.check_token_errors(expected, 0, false));

  // Delete SF to make sure destructor works, then award points
  // if it doesn't crash
  delete bf;
}

TEST_CASE("OpenClose", "[Test_BufferedFileReader]") {
  // close when already closed
  BufferedFileReader *bf = new BufferedFileReader(kHelloFileName);
  REQUIRE(bf->Good());
  bf->CloseFile();
  REQUIRE_FALSE(bf->Good());
  REQUIRE(bf->Tell() == -1);
  bf->CloseFile();
  REQUIRE_FALSE(bf->Good());

  // open when already opened
  bf->OpenFile(kByeFileName);
  REQUIRE(bf->Good());
  bf->OpenFile(kByeFileName);
  REQUIRE(bf->Good());
  bf->OpenFile(kByeFileName);
  REQUIRE(bf->Good());

  // open and close the same file over and over again
  for (size_t i = 0; i < 10; i++) {
    bf->OpenFile(kByeFileName);
    REQUIRE(bf->Good());
    bf->CloseFile();
    REQUIRE_FALSE(bf->Good());
    bf->CloseFile();
    REQUIRE_FALSE(bf->Good());
    bf->OpenFile(kByeFileName);
    REQUIRE(bf->Good());
  }

  // destructor on an already closed file
  delete bf;
}

TEST_CASE("GetChar", "[Test_BufferedFileReader]") {

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
  BufferedFileReader bf(kHelloFileName);
  BufferChecker bc(bf);
  string contents;
  char c;
  for (size_t i = 0; i < kHelloContents.length(); i++) {
    REQUIRE(bf.Tell() >= 0);
    REQUIRE(i == static_cast<size_t>(bf.Tell()));
    c = bf.GetChar();
    contents += c;
    REQUIRE(bf.Good());
    REQUIRE_FALSE(bc.check_char_errors(c, i));
  }
  REQUIRE(kHelloContents == contents);
  c = bf.GetChar();
  REQUIRE(static_cast<char>(EOF) == c);
  REQUIRE_FALSE(bf.Good());

  // Goodbye test case
  bf.CloseFile();
  bf.OpenFile(kByeFileName);
  contents.clear();
  for (size_t i = 0; i < kByeContents.length(); i++) {
    REQUIRE(bf.Tell() >= 0);
    REQUIRE(i == static_cast<size_t>(bf.Tell()));
    c = bf.GetChar();
    contents += c;
    REQUIRE(bf.Good());
    REQUIRE_FALSE(bc.check_char_errors(c, i));
  }
  REQUIRE(kByeContents == contents);
  c = bf.GetChar();
  REQUIRE(static_cast<char>(EOF) == c);
  REQUIRE_FALSE(bf.Good());

  // Long file test case
  contents.clear();
  bf.CloseFile();
  REQUIRE(bf.Tell() == -1);
  bf.OpenFile(kLongFileName);
  contents.reserve(kLongContents.length());
  for (size_t i = 0; i < kLongContents.length(); i++) {
    REQUIRE(bf.Tell() >= 0);
    REQUIRE(i == static_cast<size_t>(bf.Tell()));
    c = bf.GetChar();
    contents += c;
    REQUIRE(bf.Good());
    REQUIRE_FALSE(bc.check_char_errors(c, i));
  }
  REQUIRE(kLongContents == contents);
  c = bf.GetChar();
  REQUIRE(static_cast<char>(EOF) == c);
  REQUIRE_FALSE(bf.Good());

  // "Great" file test case
  contents.clear();
  bf.CloseFile();
  REQUIRE(bf.Tell() == -1);
  bf.OpenFile(kGreatFileName);
  contents.reserve(kGreatContents.length());
  for (size_t i = 0; i < kGreatContents.length(); i++) {
    REQUIRE(bf.Tell() >= 0);
    REQUIRE(i == static_cast<size_t>(bf.Tell()));
    c = bf.GetChar();
    contents += c;
    REQUIRE(bf.Good());
    REQUIRE_FALSE(bc.check_char_errors(c, i));
  }
  REQUIRE(kGreatContents == contents);
  c = bf.GetChar();
  REQUIRE(static_cast<char>(EOF) == c);
  REQUIRE_FALSE(bf.Good());
}

TEST_CASE("op_string", "[Test_BufferedFileReader]") {
  string token{};
  bool res{false};
  string delims{" \r\v\n\t\f"};
  off_t offset{0};
  // file contents
  string kHelloContents{};
  string kLongContents{};

  // flil the above strings with the actual contents of the file
  ifstream hello_ifs(kHelloFileName);
  kHelloContents.assign((std::istreambuf_iterator<char>(hello_ifs)),
                        (std::istreambuf_iterator<char>()));
  ifstream long_ifs(kLongFileName);
  kLongContents.assign((std::istreambuf_iterator<char>(long_ifs)),
                       (std::istreambuf_iterator<char>()));
  BufferedFileReader bf(kHelloFileName);
  BufferChecker bc(bf);

  while (bf >> token) {
    REQUIRE_FALSE(bc.check_token_errors(token, offset, true));
    REQUIRE(verify_token(token, kHelloContents, delims, &offset, true));
    REQUIRE(offset == static_cast<off_t>(bf.Tell()));
  }

  REQUIRE(static_cast<off_t>(kHelloContents.length()) == offset);
  REQUIRE(kHelloContents.length() == bf.Tell());

  offset = 0;
  bf.OpenFile(kLongFileName);
  while (bf >> token) {
    REQUIRE_FALSE(bc.check_token_errors(token, offset, true));
    REQUIRE(verify_token(token, kLongContents, delims, &offset, true));
    REQUIRE(offset == static_cast<off_t>(bf.Tell()));
  }

  REQUIRE(static_cast<off_t>(kLongContents.length()) - 2 == offset);
  REQUIRE(isspace(kLongContents.at(kLongContents.size() - 1)));
  REQUIRE(isspace(kLongContents.at(kLongContents.size() - 2)));
  REQUIRE(kLongContents.length() == bf.Tell());
  res = static_cast<bool>(bf >> token);
  REQUIRE_FALSE(res);
}

TEST_CASE("op_int", "[Test_BufferedFileReader]") {
  string token{};
  string line{};
  int value{12};
  bool res{false};
  string str_delims{" \r\v\n\t\f"};
  string int_delims{" \r\v\n\t\fabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"};
  off_t offset{0};
  // file contents
  string kNumbersContents{};

  // flil the above strings with the actual contents of the file
  ifstream numbers_ifs(kNumbersFileName);
  kNumbersContents.assign((std::istreambuf_iterator<char>(numbers_ifs)),
                        (std::istreambuf_iterator<char>()));
  
  BufferedFileReader bf(kNumbersFileName);
  BufferChecker bc(bf);

  REQUIRE(static_cast<bool>(bf >> value));
  REQUIRE(value == 0);
  REQUIRE_FALSE(bc.check_token_errors("0", offset, true));
  REQUIRE(verify_token("0", kNumbersContents, int_delims, &offset, true));
  REQUIRE(offset == static_cast<off_t>(bf.Tell()));

  REQUIRE(bf >> token);
  REQUIRE(token == "xDEADBEEF");
  REQUIRE_FALSE(bc.check_token_errors(token, offset, true));
  REQUIRE(verify_token(token, kNumbersContents, str_delims, &offset, true));
  REQUIRE(offset == static_cast<off_t>(bf.Tell()));

  REQUIRE(bf >> token);
  REQUIRE(token == "is");
  REQUIRE_FALSE(bc.check_token_errors(token, offset, true));
  REQUIRE(verify_token(token, kNumbersContents, str_delims, &offset, true));
  REQUIRE(offset == static_cast<off_t>(bf.Tell()));

  REQUIRE(bf >> token);
  REQUIRE(token == "a");
  REQUIRE_FALSE(bc.check_token_errors(token, offset, true));
  REQUIRE(verify_token(token, kNumbersContents, str_delims, &offset, true));
  REQUIRE(offset == static_cast<off_t>(bf.Tell()));

  REQUIRE(bf >> token);
  REQUIRE(token == "cool");
  REQUIRE_FALSE(bc.check_token_errors(token, offset, true));
  REQUIRE(verify_token(token, kNumbersContents, str_delims, &offset, true));
  REQUIRE(offset == static_cast<off_t>(bf.Tell()));

  REQUIRE(static_cast<bool>(bf >> value));
  REQUIRE(value == 16);
  REQUIRE_FALSE(bc.check_token_errors("16", offset, true));
  REQUIRE(verify_token("16", kNumbersContents, int_delims, &offset, true));
  REQUIRE(offset == static_cast<off_t>(bf.Tell()));

  bf.GetLine(line);
  REQUIRE(line == " bit num");
  REQUIRE_FALSE(bc.check_token_errors(line, offset, false));
  REQUIRE(verify_token(line, kNumbersContents, "\n", &offset, false));
  offset += 1;
  REQUIRE(offset == static_cast<off_t>(bf.Tell()));

  REQUIRE(bf >> token);
  REQUIRE(token == "the");
  REQUIRE_FALSE(bc.check_token_errors(token, offset, true));
  REQUIRE(verify_token(token, kNumbersContents, str_delims, &offset, true));
  REQUIRE(offset == static_cast<off_t>(bf.Tell()));

  REQUIRE(bf >> token);
  REQUIRE(token == "biggest");
  REQUIRE_FALSE(bc.check_token_errors(token, offset, true));
  REQUIRE(verify_token(token, kNumbersContents, str_delims, &offset, true));
  REQUIRE(offset == static_cast<off_t>(bf.Tell()));

  REQUIRE(static_cast<bool>(bf >> value));
  REQUIRE(value == 16);
  REQUIRE_FALSE(bc.check_token_errors("16", offset, true));
  REQUIRE(verify_token("16", kNumbersContents, int_delims, &offset, true));
  REQUIRE(offset == static_cast<off_t>(bf.Tell()));

  REQUIRE(bf >> token);
  REQUIRE(token == "bit");
  REQUIRE_FALSE(bc.check_token_errors(token, offset, true));
  REQUIRE(verify_token(token, kNumbersContents, str_delims, &offset, true));
  REQUIRE(offset == static_cast<off_t>(bf.Tell()));

  bf.GetLine(line, '6');
  REQUIRE(line == " numb is ");
  REQUIRE_FALSE(bc.check_token_errors(line, offset, false));
  REQUIRE(verify_token(line, kNumbersContents, "6", &offset, false));
  offset += 1;
  REQUIRE(offset == static_cast<off_t>(bf.Tell()));

  REQUIRE(static_cast<bool>(bf >> value));
  REQUIRE(value == 5536);
  REQUIRE_FALSE(bc.check_token_errors("5536", offset, true));
  REQUIRE(verify_token("5536", kNumbersContents, int_delims, &offset, true));
  REQUIRE(offset == static_cast<off_t>(bf.Tell()));

  bf.GetLine(line);
  REQUIRE(line == "");
  REQUIRE_FALSE(bc.check_token_errors(line, offset, false));
  REQUIRE(verify_token(line, kNumbersContents, "\n", &offset, false));
  offset += 1;
  REQUIRE(offset == static_cast<off_t>(bf.Tell()));

  bf.GetLine(line);
  REQUIRE(line == "one of my favourite bands lately has been");
  REQUIRE_FALSE(bc.check_token_errors(line, offset, false));
  REQUIRE(verify_token(line, kNumbersContents, "\n", &offset, false));
  offset += 1;
  REQUIRE(offset == static_cast<off_t>(bf.Tell()));

  REQUIRE(bf >> token);
  REQUIRE(token == "condor");
  REQUIRE_FALSE(bc.check_token_errors(token, offset, true));
  REQUIRE(verify_token(token, kNumbersContents, str_delims, &offset, true));
  REQUIRE(offset == static_cast<off_t>(bf.Tell()));

  REQUIRE(static_cast<bool>(bf >> value));
  REQUIRE(value == 44);
  REQUIRE_FALSE(bc.check_token_errors("44", offset, true));
  REQUIRE(verify_token("44", kNumbersContents, int_delims, &offset, true));
  REQUIRE(offset == static_cast<off_t>(bf.Tell()));

  REQUIRE(kNumbersContents.length() == bf.Tell());
  res = static_cast<bool>(bf >> value);
  REQUIRE_FALSE(res);
}

TEST_CASE("GetLine", "[Test_BufferedFileReader]") {
  string kByeContents{};
  string kGreatContents{};

  ifstream long_ifs(kGreatFileName);
  kGreatContents.assign((std::istreambuf_iterator<char>(long_ifs)),
                       (std::istreambuf_iterator<char>()));

  ifstream bye_ifs(kByeFileName);
  kByeContents.assign((std::istreambuf_iterator<char>(bye_ifs)),
                      (std::istreambuf_iterator<char>()));

  string line;
  string delims = "\n";
  off_t offset = 0;

  BufferedFileReader bf(kByeFileName);
  BufferChecker bc(bf);

  while (bf.GetLine(line)) {
    REQUIRE_FALSE(bc.check_token_errors(line, offset, false));
    REQUIRE(verify_token(line, kByeContents, delims, &offset, false));
    offset += 1;
    REQUIRE(offset == static_cast<off_t>(bf.Tell()));
  }

  REQUIRE(static_cast<off_t>(kByeContents.length()) == offset);

  offset = 0;
  bf.OpenFile(kGreatFileName);

  while (bf.GetLine(line)) {
    REQUIRE_FALSE(bc.check_token_errors(line, offset, false));
    REQUIRE(verify_token(line, kGreatContents, delims, &offset, false));
    offset += 1;
    REQUIRE(offset == static_cast<off_t>(bf.Tell()));
  }

  REQUIRE(static_cast<off_t>(kGreatContents.length()) == offset);
}

TEST_CASE("Complex", "[Test_BufferedFileReader]") {

  optional<string> opt{};
  string token;
  string delims = " \n\f\r\t\v";
  off_t offset = 0;
  bool res{false};
  bool read_line{false};
  string kLongContents{};
  ifstream long_ifs(kLongFileName);
  kLongContents.assign((std::istreambuf_iterator<char>(long_ifs)),
                       (std::istreambuf_iterator<char>()));

  BufferedFileReader bf(kLongFileName);
  BufferChecker bc(bf);

  for (int i = 0; i < 3; i++) {
    while (bf.Good()) {
      if (read_line) {
        bf.GetLine(token);
        REQUIRE_FALSE(bc.check_token_errors(token, offset, false));
        REQUIRE(verify_token(token, kLongContents, delims, &offset, false));
        offset += 1;
        REQUIRE(offset == static_cast<off_t>(bf.Tell()));
      } else {
        bf >> token;
        REQUIRE_FALSE(bc.check_token_errors(token, offset, true));
        REQUIRE(verify_token(token, kLongContents, delims, &offset, true));
        REQUIRE(offset == static_cast<off_t>(bf.Tell()));
      }
      read_line = !read_line;
    }

    REQUIRE(static_cast<off_t>(kLongContents.length()) == offset);
    res = static_cast<bool>(bf >> token);
    REQUIRE_FALSE(res);
    offset = 0;
    bf.Rewind();
  }
}

TEST_CASE("Move Ctor", "[Test_BufferedFileReader]") {
  string kGreatContents{};
  ifstream long_ifs(kGreatFileName);
  kGreatContents.assign((std::istreambuf_iterator<char>(long_ifs)),
                       (std::istreambuf_iterator<char>()));

  // use new so we can control when it is destructed
  BufferedFileReader* bf = new BufferedFileReader(kGreatFileName);

  BufferedFileReader& bf0 = *bf;
  BufferChecker bc0(bf0);

  int fd = bc0.fd();
  string token{};
  bool bfbool = static_cast<bool>(bf0 >> token);
  REQUIRE(bfbool);
  REQUIRE(token == "Project");

  auto arr = bc0.buffer();

  // move construct a new bf
  BufferedFileReader bf1(std::move(bf0));
  BufferChecker bc1(bf1);

  REQUIRE(fd == bc1.fd());

  // fd of the original needs to change so that if
  // it closes, then it doesn't close the file in the move destination.
  REQUIRE(fd != bc0.fd());

  // check that the file is not open already
  int old_fd = bc0.fd();
  int res = fcntl(old_fd, F_GETFL);
  int err = errno;
  REQUIRE(res < 0);
  REQUIRE(err == EBADF);

  // open the file descriptor
  // but only if a valid num was used.
  res = dup2(STDOUT_FILENO, old_fd);

  // make sure that the destructor for the moved from bf does not close it
  delete bf;

  // make sure that if we could re-open it
  // that it was not closed by the destructor
  if (res != -1) {
    res = fcntl(old_fd, F_GETFL);
    err = errno;
    REQUIRE(res >= 0);
    REQUIRE(err != EBADF);

    close(old_fd);
  }


  for (size_t i = bc1.curr_index(); i < bc1.curr_length(); i++) {
    REQUIRE(arr[i] == bc1.buffer()[i]);
  }

  off_t offset = 0;
  REQUIRE(verify_token(token, kGreatContents, " \t\n\r\v\f", &offset, true));

  // make sure reading the file is ok after moving
  while (bf1 >> token) {
    REQUIRE_FALSE(bc1.check_token_errors(token, offset, true));
    REQUIRE(verify_token(token, kGreatContents, " \t\n\r\v\f", &offset, true));
    REQUIRE(offset == static_cast<off_t>(bf1.Tell()));
  }

  REQUIRE(static_cast<off_t>(kGreatContents.length() - 1) == offset);
  REQUIRE(isspace(kGreatContents.at(kGreatContents.length() - 1)));
}

TEST_CASE("Move op=", "[Test_BufferedFileReader]") {
  string kGreatContents{};
  ifstream long_ifs(kGreatFileName);
  kGreatContents.assign((std::istreambuf_iterator<char>(long_ifs)),
                       (std::istreambuf_iterator<char>()));

  // use new so we can control when it is destructed
  BufferedFileReader* bf = new BufferedFileReader(kGreatFileName);

  BufferedFileReader& bf0 = *bf;
  BufferChecker bc0(bf0);

  int fd = bc0.fd();
  //auto opt = bf0.get_token();
  //REQUIRE(opt.has_value());
  string token{};
  bool bfbool = static_cast<bool>(bf0 >> token);
  REQUIRE(bfbool);
  REQUIRE(token == "Project");

  auto arr = bc0.buffer();

  // move assign a new bf
  BufferedFileReader bf1(kHelloFileName);
  bf1 = std::move(bf0);
  BufferChecker bc1(bf1);

  REQUIRE(fd == bc1.fd());

  // fd of the original needs to change so that if
  // it closes, then it doesn't close the file in the move destination.
  REQUIRE(fd != bc0.fd());

  // check that the file is not open already
  int old_fd = bc0.fd();
  int res = fcntl(old_fd, F_GETFL);
  int err = errno;
  REQUIRE(res < 0);
  REQUIRE(err == EBADF);

  // open the file descriptor
  // but only if a valid num was used.
  res = dup2(STDOUT_FILENO, old_fd);

  // make sure that the destructor for the moved from bf does not close it
  delete bf;

  // make sure that if we could re-open it
  // that it was not closed by the destructor
  if (res != -1) {
    res = fcntl(old_fd, F_GETFL);
    err = errno;
    REQUIRE(res >= 0);
    REQUIRE(err != EBADF);

    close(old_fd);
  }


  for (size_t i = bc1.curr_index(); i < bc1.curr_length(); i++) {
    REQUIRE(arr[i] == bc1.buffer()[i]);
  }

  off_t offset = 0;
  REQUIRE(verify_token(token, kGreatContents, " \t\n\r\v\f", &offset, true));

  // make sure reading the file is ok after moving
  while (bf1 >> token) {
    REQUIRE_FALSE(bc1.check_token_errors(token, offset, true));
    REQUIRE(verify_token(token, kGreatContents, " \t\n\r\v\f", &offset, true));
    REQUIRE(offset == static_cast<off_t>(bf1.Tell()));
  }

  REQUIRE(static_cast<off_t>(kGreatContents.length() - 1) == offset);
}
