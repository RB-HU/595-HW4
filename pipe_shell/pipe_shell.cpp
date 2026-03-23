#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

// split str by delim, skipping empty tokens
static std::vector<std::string> Split(const std::string& str, char delim) {
  std::vector<std::string> tokens;
  std::string token;
  for (const char c : str) {
    if (c == delim) {
      if (!token.empty()) {
        tokens.push_back(token);
        token.clear();
      }
    } else {
      token += c;
    }
  }
  if (!token.empty()) {
    tokens.push_back(token);
  }
  return tokens;
}

// build null-terminated argv array for execvp, pointers into args
static std::vector<char*> BuildArgv(std::vector<std::string>& args) {
  std::vector<char*> argv;
  argv.reserve(args.size() + 1);
  for (std::string& s : args) {
    argv.push_back(s.data());
  }
  argv.push_back(nullptr);
  return argv;
}

// run a pipeline: fork one child per command, wire up pipes between them,
// then wait for all to finish
static void ExecutePipeline(std::vector<std::vector<std::string>>& commands) {
  const size_t num_cmds = commands.size();

  // Create num_cmds - 1 pipes
  std::vector<std::array<int, 2>> pipes(num_cmds - 1);
  for (size_t i = 0; i < num_cmds - 1; i++) {
    if (pipe(pipes[i].data()) < 0) {
      std::cerr << "pipe() failed\n";
      return;
    }
  }

  std::vector<pid_t> pids;

  for (size_t i = 0; i < num_cmds; i++) {
    const pid_t pid = fork();
    if (pid < 0) {
      std::cerr << "fork() failed\n";
      return;
    }

    if (pid == 0) {
      // Child process

      // If not the first command, read from previous pipe
      if (i > 0) {
        if (dup2(pipes[i - 1][0], STDIN_FILENO) < 0) {
          std::cerr << "dup2() failed\n";
          exit(EXIT_FAILURE);
        }
      }

      // If not the last command, write to next pipe
      if (i < num_cmds - 1) {
        if (dup2(pipes[i][1], STDOUT_FILENO) < 0) {
          std::cerr << "dup2() failed\n";
          exit(EXIT_FAILURE);
        }
      }

      // Close all pipe fds in child
      for (size_t j = 0; j < num_cmds - 1; j++) {
        close(pipes[j][0]);
        close(pipes[j][1]);
      }

      std::vector<char*> argv = BuildArgv(commands[i]);
      execvp(argv[0], argv.data());

      // execvp only returns on error
      std::cerr << strerror(errno) << "\n";
      exit(EXIT_FAILURE);
    }

    pids.push_back(pid);
  }

  // Parent: close all pipe fds
  for (size_t i = 0; i < num_cmds - 1; i++) {
    close(pipes[i][0]);
    close(pipes[i][1]);
  }

  // Wait for all children
  for (const pid_t pid : pids) {
    waitpid(pid, nullptr, 0);
  }
}

int main() {
  std::string line;

  while (true) {
    std::cout << "$ " << std::flush;

    if (!std::getline(std::cin, line)) {
      // EOF
      break;
    }

    if (line == "exit") {
      break;
    }

    if (line.empty()) {
      continue;
    }

    // Split line by '|' to get individual commands
    const std::vector<std::string> cmd_strings = Split(line, '|');

    if (cmd_strings.empty()) {
      continue;
    }

    // Split each command string into tokens (by space)
    std::vector<std::vector<std::string>> commands;
    bool valid = true;
    for (const std::string& cmd_str : cmd_strings) {
      const std::vector<std::string> tokens = Split(cmd_str, ' ');
      if (tokens.empty()) {
        std::cerr << "Error: empty command\n";
        valid = false;
        break;
      }
      commands.push_back(tokens);
    }

    if (!valid) {
      continue;
    }

    ExecutePipeline(commands);
  }

  return EXIT_SUCCESS;
}
