#ifndef MESSAGE_QUEUE_HPP_
#define MESSAGE_QUEUE_HPP_

#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>

template <typename T>
class MessageQueue {
 public:
  MessageQueue();
  ~MessageQueue() = default;

  bool Add(const T& val);
  void Close();
  std::optional<T> Remove();
  std::optional<T> WaitRemove();
  int Size();

  MessageQueue(const MessageQueue& other) = delete;
  MessageQueue& operator=(const MessageQueue& other) = delete;
  MessageQueue(MessageQueue&& other) = delete;
  MessageQueue& operator=(MessageQueue&& other) = delete;

 private:
  std::deque<T> m_que;
  std::mutex m_mutex;
  std::condition_variable m_cv;
  bool m_closed = false;
};

#include "./MessageQueue.ipp"

#endif  // MESSAGE_QUEUE_HPP_
