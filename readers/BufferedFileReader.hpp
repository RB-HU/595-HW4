#ifndef BUFFERED_FILE_READER_HPP_
#define BUFFERED_FILE_READER_HPP_

#include <array>
#include <cstddef>
#include <optional>
#include <string>

///////////////////////////////////////////////////////////////////////////////
// A BufferedFileReader is a class for reading files.
//
// This class is a moderately complex wrapper around POSIX file I/O calls
// with more functionality than SimpleFileReader. Reading from the file
// is buffered to increase performance.
///////////////////////////////////////////////////////////////////////////////
class BufferedFileReader {
 public:
  explicit BufferedFileReader(const std::string& fname);
  ~BufferedFileReader();

  BufferedFileReader(BufferedFileReader&& other) noexcept;
  BufferedFileReader& operator=(BufferedFileReader&& other) noexcept;

  void OpenFile(const std::string& fname);
  void CloseFile();

  [[nodiscard]] char GetChar();

  BufferedFileReader& operator>>(std::string& str);
  BufferedFileReader& operator>>(int& value);

  BufferedFileReader& GetLine(std::string& line, char delim = '\n');

  [[nodiscard]] int Tell() const;
  void Rewind();

  [[nodiscard]] bool Good() const;
  explicit operator bool() const;

  BufferedFileReader(const BufferedFileReader& other) = delete;
  BufferedFileReader& operator=(const BufferedFileReader& other) = delete;

  friend class BufferChecker;

 private:
  static constexpr size_t k_buf_size = 1024;

  size_t m_curr_length;
  size_t m_curr_index;
  std::array<char, k_buf_size> m_buffer;
  int m_fd;
  bool m_good;

  // Refills m_buffer from the file; resets m_curr_index to 0.
  void FillBuffer();
};

#endif  // BUFFERED_FILE_READER_HPP_
