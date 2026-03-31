#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using std::cerr;
using std::cin;
using std::cout;
using std::string;
using std::vector;

// split a line into tokens by whitespace
static vector<string> Tokenize(const string& line) {
  vector<string> tokens;
  std::istringstream stream(line);
  string token;
  while (stream >> token) {
    tokens.push_back(token);
  }
  return tokens;
}

// build null-terminated argv array for execvp
static vector<char*> BuildArgv(vector<string>& tokens) {
  vector<char*> argv;
  argv.reserve(tokens.size() + 1);
  for (string& token : tokens) {
    argv.push_back(token.data());
  }
  argv.push_back(nullptr);
  return argv;
}

// fork and exec a command; returns true if child exited normally
static bool RunCommand(vector<string>& tokens) {
  const pid_t pid = fork();
  if (pid < 0) {
    cerr << "fork() failed: " << strerror(errno) << "\n";
    return false;
  }

  if (pid == 0) {
    // child: exec the command
    vector<char*> argv = BuildArgv(tokens);
    execvp(argv[0], argv.data());
    // execvp only returns on error
    cerr << strerror(errno) << "\n";
    exit(EXIT_FAILURE);
  }

  // parent: wait for child
  int status = 0;
  waitpid(pid, &status, 0);
  return (WIFEXITED(status) != 0) && (WEXITSTATUS(status) == EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
  // parse optional retry count argument
  int retry_count = 0;
  if (argc == 2) {
    try {
      retry_count = std::stoi(argv[1]);
      if (retry_count <= 0) {
        cerr << "Error: retry count must be a positive integer\n";
        return EXIT_FAILURE;
      }
    } catch (const std::invalid_argument&) {
      cerr << "Error: invalid argument: " << argv[1] << "\n";
      return EXIT_FAILURE;
    } catch (const std::out_of_range&) {
      cerr << "Error: argument out of range: " << argv[1] << "\n";
      return EXIT_FAILURE;
    }
  } else if (argc > 2) {
    cerr << "Usage: " << argv[0] << " [retry_count]\n";
    return EXIT_FAILURE;
  }

  string line;
  while (true) {
    cout << "$ " << std::flush;

    if (!std::getline(cin, line)) {
      // EOF
      break;
    }

    // tokenize the input
    vector<string> tokens = Tokenize(line);

    // skip empty input
    if (tokens.empty()) {
      continue;
    }

    // exit command
    if (tokens[0] == "exit") {
      break;
    }

    // run the command
    const bool success = RunCommand(tokens);

    // retry logic
    if (!success && (retry_count > 0)) {
      bool recovered = false;
      for (int attempt = 0; attempt < retry_count; attempt++) {
        cerr << "retrying...\n";
        if (RunCommand(tokens)) {
          recovered = true;
          break;
        }
      }
      if (!recovered) {
        cerr << "Failed to run program after retrying\n";
      }
    }
  }

  return EXIT_SUCCESS;
}
