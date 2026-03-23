#ifndef SIMPLE_FILE_READER_HPP_
#define SIMPLE_FILE_READER_HPP_

#include <optional>
#include <string>

///////////////////////////////////////////////////////////////////////////////
// A SimpleFileReader is a class for reading files.
//
// This class is a simple wrapper around POSIX file I/O calls
// and thus has limited functionality.
// Also note that the reads are UNBUFFERED.
///////////////////////////////////////////////////////////////////////////////
class SimpleFileReader {
 public:
  // Constructor for a SimpleFileReader. Should open the
  // file and do whatever is necesary to "set-up" the object.
  // After construction, reading from the file should start
  // at the front of the file.
  // Undefined behaviour if the file name is invalid.
  //
  // Arguments:
  // - fname: The name of the file to be read
  explicit SimpleFileReader(const std::string& fname);

  // Destructor for a SimpleFileReader. Should clean up
  // any allocated resources such as memory or open files.
  //
  // Arguments: None
  ~SimpleFileReader();

  // Sets up the SimpleFileReader to start reading from the
  // front of the specified file. Note that there could or could not
  // already be an open file managed by the SimpleFileReader.
  // This function handles both cases and if the object is already
  // managing a file, that file is closed.
  // Undefined behaviour if the file name is invalid.
  //
  // Arguments:
  // - fname: The name of the file to be opened
  void OpenFile(const std::string& fname);

  // Closes the file currently managed by the SimpleFileReader.
  // If there is not a file currently open, then nothing should happen.
  //
  // Arguments: None
  void CloseFile();

  // Gets the next singular character from the file.
  //
  // Arguments: None
  //
  // Returns:
  // - the next char in the file. If at the end of the file,
  //   or if there is no file open currently, then EOF is returned.
  [[nodiscard]] char GetChar();

  // Gets the next n characters from the file and returns it as a string.
  //
  // Arguments:
  // - n: non-negative number of characters to read from the file.
  //
  // Returns:
  // - a string representing the next n character of the file.
  //   If the end of file is reached, while trying to read
  //   that many chars, simply return a string with
  //   all of the characters read before reaching the end.
  // - nullopt if the file is not open currently or already
  //   at the end of the file.
  [[nodiscard]] std::optional<std::string> GetChars(size_t n);

  // Returns the current position the user is in to the file.
  //
  // Arguments: None
  //
  // Returns:
  // - The current position we are in the file, which is the
  //   Offset from the start of the file. (e.g, if the user is at
  //   the start of the file, returns 0. If the user has read 2
  //   characters, return 2. etc.).
  // - -1 if there is no open file
  [[nodiscard]] int Tell() const;

  // Resets the file to start reading from the beginning
  // of the file that is currently open.
  // Undefined behaviour if there is no file open currently.
  //
  // Arguments: None
  void Rewind();

  // Returns whether or not the file is available for reading
  // (e.g. if the file is open and not at the end of file)
  // Note: The reader is only considered to be at the end of file
  // if it had previously tried to read, and then hit the end
  // of the file, causing it to have an "incomplete" read).
  //
  // Arguements: None
  //
  // Returns:
  // - false if the file reader is at the end of the file
  //   or if there is no file open
  // - true otherwise
  [[nodiscard]] bool Good() const;

  // Ignore These
  // If you want to know more, this is disabling the
  // copy constructor and the assignment operator.
  SimpleFileReader(const SimpleFileReader& other) = delete;
  SimpleFileReader& operator=(const SimpleFileReader& other) = delete;
  SimpleFileReader(const SimpleFileReader&& other) = delete;
  SimpleFileReader& operator=(const SimpleFileReader&& other) = delete;

 private:
  // fields
  int m_fd;     // The File Descriptor that we use to manage our file.
  bool m_good;  // Whether or not the reader is good to read
};

#endif  // SIMPLE_FILE_READER_HPP_
