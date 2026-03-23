#include <cctype>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "BufferedFileReader.hpp"

// Provided: synonym for Good()
BufferedFileReader::operator bool() const {
  return this->Good();
}

BufferedFileReader::BufferedFileReader(const std::string& fname)
    : m_curr_length(0), m_curr_index(0), m_buffer{}, m_fd(-1), m_good(false) {
  OpenFile(fname);
}

BufferedFileReader::~BufferedFileReader() {
  CloseFile();
}

BufferedFileReader::BufferedFileReader(BufferedFileReader&& other) noexcept
    : m_curr_length(other.m_curr_length),
      m_curr_index(other.m_curr_index),
      m_buffer(other.m_buffer),
      m_fd(other.m_fd),
      m_good(other.m_good) {
  other.m_fd = -1;
  other.m_good = false;
  other.m_curr_length = 0;
  other.m_curr_index = 0;
}

BufferedFileReader& BufferedFileReader::operator=(
    BufferedFileReader&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  CloseFile();
  m_fd = other.m_fd;
  m_good = other.m_good;
  m_curr_length = other.m_curr_length;
  m_curr_index = other.m_curr_index;
  m_buffer = other.m_buffer;
  other.m_fd = -1;
  other.m_good = false;
  other.m_curr_length = 0;
  other.m_curr_index = 0;
  return *this;
}

void BufferedFileReader::FillBuffer() {
  const ssize_t bytes = read(m_fd, m_buffer.data(), k_buf_size);
  if (bytes <= 0) {
    m_curr_length = 0;
  } else {
    m_curr_length = static_cast<size_t>(bytes);
  }
  m_curr_index = 0;
}

void BufferedFileReader::OpenFile(const std::string& fname) {
  if (m_fd >= 0) {
    close(m_fd);
  }
  m_fd = open(fname.c_str(), O_RDONLY);
  m_good = (m_fd >= 0);
  m_curr_length = 0;
  m_curr_index = 0;
  if (m_good) {
    FillBuffer();
  }
}

void BufferedFileReader::CloseFile() {
  if (m_fd >= 0) {
    close(m_fd);
    m_fd = -1;
  }
  m_good = false;
  m_curr_length = 0;
  m_curr_index = 0;
}

char BufferedFileReader::GetChar() {
  if (m_fd < 0) {
    return static_cast<char>(EOF);
  }
  if (m_curr_index >= m_curr_length) {
    FillBuffer();
    if (m_curr_length == 0) {
      m_good = false;
      return static_cast<char>(EOF);
    }
  }
  return m_buffer[m_curr_index++];
}

BufferedFileReader& BufferedFileReader::operator>>(std::string& str) {
  str.erase();
  if (m_fd < 0 || !m_good) {
    return *this;
  }

  // Skip leading whitespace
  char c = GetChar();
  while (c != static_cast<char>(EOF) &&
         (std::isspace(static_cast<unsigned char>(c)) != 0)) {
    c = GetChar();
  }
  if (c == static_cast<char>(EOF)) {
    return *this;
  }

  // Read non-whitespace characters
  while (c != static_cast<char>(EOF) &&
         !(std::isspace(static_cast<unsigned char>(c)) != 0)) {
    str += c;
    // Peek at next character without consuming if it's whitespace
    if (m_curr_index >= m_curr_length) {
      FillBuffer();
      if (m_curr_length == 0) {
        break;
      }
    }
    c = m_buffer[m_curr_index];
    if ((std::isspace(static_cast<unsigned char>(c)) != 0)) {
      break;
    }
    m_curr_index++;
  }
  return *this;
}

BufferedFileReader& BufferedFileReader::operator>>(int& value) {
  value = 0;
  if (m_fd < 0 || !m_good) {
    return *this;
  }

  // Skip leading whitespace
  char c = GetChar();
  while (c != static_cast<char>(EOF) &&
         (std::isspace(static_cast<unsigned char>(c)) != 0)) {
    c = GetChar();
  }
  if (c == static_cast<char>(EOF)) {
    return *this;
  }

  // Non-whitespace non-digit: treat as EOF
  if (!(std::isdigit(static_cast<unsigned char>(c)) != 0)) {
    m_good = false;
    return *this;
  }

  // Read digit characters
  while (c != static_cast<char>(EOF) &&
         (std::isdigit(static_cast<unsigned char>(c)) != 0)) {
    value = value * 10 + (c - '0');
    // Peek at next character without consuming if it's not a digit
    if (m_curr_index >= m_curr_length) {
      FillBuffer();
      if (m_curr_length == 0) {
        break;
      }
    }
    c = m_buffer[m_curr_index];
    if (!(std::isdigit(static_cast<unsigned char>(c)) != 0)) {
      break;
    }
    m_curr_index++;
  }
  return *this;
}

BufferedFileReader& BufferedFileReader::GetLine(std::string& line, char delim) {
  line.erase();
  if (m_fd < 0) {
    return *this;
  }

  char c = GetChar();
  if (c == static_cast<char>(EOF)) {
    return *this;
  }

  while (c != static_cast<char>(EOF) && c != delim) {
    line += c;
    c = GetChar();
  }
  return *this;
}

int BufferedFileReader::Tell() const {
  if (m_fd < 0) {
    return -1;
  }
  const off_t file_pos = lseek(m_fd, 0, SEEK_CUR);
  return static_cast<int>(file_pos -
                          static_cast<off_t>(m_curr_length - m_curr_index));
}

void BufferedFileReader::Rewind() {
  if (m_fd < 0) {
    return;
  }
  lseek(m_fd, 0, SEEK_SET);
  m_good = true;
  m_curr_length = 0;
  m_curr_index = 0;
  FillBuffer();
}

bool BufferedFileReader::Good() const {
  return m_good;
}
