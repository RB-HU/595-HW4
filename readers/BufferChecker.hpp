#ifndef BUFFER_CHECKER_HPP_
#define BUFFER_CHECKER_HPP_

#include <cctype>
#include <string>

#include "BufferedFileReader.hpp"

class BufferChecker {
 public:
  BufferChecker(const BufferedFileReader& bfr) : m_bf(bfr) {}

  // Returns true if there is a detectable error
  // False if an error was not detected
  bool check_char_errors(char to_check, off_t file_offset) {
    size_t index = file_offset % BufferedFileReader::k_buf_size;
    if (index == BufferedFileReader::k_buf_size - 1) {
      // give some flexibility on how the last character is handled
      return false;
    }

    return m_bf.m_buffer.at(index) != to_check;
  }

  // Returns true if there is a detectable error
  // False if an error was not detected
  bool check_token_errors(const std::string& token,
                          off_t file_offset,
                          bool lstrip) {
    size_t end_index =
        (file_offset + token.length()) % BufferedFileReader::k_buf_size;
    size_t start_index = file_offset % BufferedFileReader::k_buf_size;

    // Check if we can even check if the token is in the buffer.
    // Can't be checked if token is clipped by buffer length, or token length is
    // greater than buffer length, and other edge cases
    if (start_index > end_index || end_index - start_index != token.length()) {
      return false;
    }

    while (lstrip && isspace(buffer().at(start_index)) &&
           start_index < BufferedFileReader::k_buf_size &&
           start_index < curr_length()) {
      start_index += 1;
      end_index += 1;
    }

    // buffer refilled
    if (end_index >= curr_length() ||
        (start_index > 1000 && curr_index() <= 100)) {
      return false;
    }

    for (off_t i = 0; start_index + i < end_index; i++) {
      if (token.at(i) != m_bf.m_buffer.at(i + start_index)) {
        return true;
      }
    }
    return false;
  }

  int fd() const { return m_bf.m_fd; }

  const std::array<char, BufferedFileReader::k_buf_size>& buffer() const {
    return m_bf.m_buffer;
  }

  size_t curr_length() const { return m_bf.m_curr_length; }

  size_t curr_index() const { return m_bf.m_curr_index; }

  bool good() const { return m_bf.m_good; }

 private:
  const BufferedFileReader& m_bf;
};

#endif  // BUFFER_CHECKER_HPP_
