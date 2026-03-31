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
    vector<char*> argv = BuildArgv(tokens);
    execvp(argv[0], argv.data());
    cerr << strerror(errno) << "\n";
    exit(EXIT_FAILURE);
  }

  int status = 0;
  waitpid(pid, &status, 0);
  return (WIFEXITED(status) != 0) && (WEXITSTATUS(status) == EXIT_SUCCESS);
}

// retry a command up to retry_count times; prints messages on each retry
static void RetryCommand(vector<string>& tokens, int retry_count) {
  for (int attempt = 0; attempt < retry_count; attempt++) {
    cout << "retrying...\n";
    if (RunCommand(tokens)) {
      return;
    }
  }
  cout << "Failed to run program after retrying\n";
}

// parse the retry count from argv[1]; returns -1 on error
static int ParseRetryCount(const string& arg) {
  try {
    const int count = std::stoi(arg);
    if (count <= 0) {
      cerr << "Error: retry count must be a positive integer\n";
      return -1;
    }
    return count;
  } catch (const std::invalid_argument&) {
    cerr << "Error: invalid argument: " << arg << "\n";
    return -1;
  } catch (const std::out_of_range&) {
    cerr << "Error: argument out of range: " << arg << "\n";
    return -1;
  }
}

// run the main shell loop with the given retry count (0 = no retry)
static void RunShell(int retry_count) {
  string line;
  while (true) {
    cout << "$ " << std::flush;

    if (!std::getline(cin, line)) {
      break;
    }

    vector<string> tokens = Tokenize(line);

    if (tokens.empty()) {
      continue;
    }

    if (tokens[0] == "exit") {
      break;
    }

    const bool success = RunCommand(tokens);
    if (!success && (retry_count > 0)) {
      RetryCommand(tokens, retry_count);
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc > 2) {
    cerr << "Usage: " << argv[0] << " [retry_count]\n";
    return EXIT_FAILURE;
  }

  int retry_count = 0;
  if (argc == 2) {
    retry_count = ParseRetryCount(argv[1]);
    if (retry_count < 0) {
      return EXIT_FAILURE;
    }
  }

  RunShell(retry_count);
  return EXIT_SUCCESS;
}
