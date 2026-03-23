#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "./SimpleFileReader.hpp"

SimpleFileReader::SimpleFileReader(const std::string& fname)
    : m_fd(-1), m_good(false) {
  OpenFile(fname);
}

SimpleFileReader::~SimpleFileReader() {
  CloseFile();
}

void SimpleFileReader::OpenFile(const std::string& fname) {
  if (m_fd >= 0) {
    close(m_fd);
  }
  m_fd = open(fname.c_str(), O_RDONLY);
  m_good = (m_fd >= 0);
}

void SimpleFileReader::CloseFile() {
  if (m_fd >= 0) {
    close(m_fd);
    m_fd = -1;
  }
  m_good = false;
}

char SimpleFileReader::GetChar() {
  if (!m_good || m_fd < 0) {
    return static_cast<char>(EOF);
  }
  char c = 0;
  ssize_t bytes_read = read(m_fd, &c, 1);
  if (bytes_read <= 0) {
    m_good = false;
    return static_cast<char>(EOF);
  }
  return c;
}

std::optional<std::string> SimpleFileReader::GetChars(size_t n) {
  if (!m_good || m_fd < 0) {
    return std::nullopt;
  }
  std::string result;
  result.reserve(n);
  for (size_t i = 0; i < n; i++) {
    char c = 0;
    ssize_t bytes_read = read(m_fd, &c, 1);
    if (bytes_read <= 0) {
      m_good = false;
      break;
    }
    result += c;
  }
  if (result.empty()) {
    return std::nullopt;
  }
  return result;
}

int SimpleFileReader::Tell() const {
  if (m_fd < 0) {
    return -1;
  }
  return static_cast<int>(lseek(m_fd, 0, SEEK_CUR));
}

void SimpleFileReader::Rewind() {
  if (m_fd >= 0) {
    lseek(m_fd, 0, SEEK_SET);
    m_good = true;
  }
}

bool SimpleFileReader::Good() const {
  return m_good;
}
