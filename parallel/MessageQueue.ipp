template <typename T>
MessageQueue<T>::MessageQueue() : m_closed(false) {}

template <typename T>
bool MessageQueue<T>::Add(const T& val) {
  std::scoped_lock lock(m_mutex);
  if (m_closed) {
    return false;
  }
  m_que.push_back(val);
  m_cv.notify_one();
  return true;
}

template <typename T>
void MessageQueue<T>::Close() {
  std::scoped_lock lock(m_mutex);
  m_closed = true;
  m_cv.notify_all();
}

template <typename T>
std::optional<T> MessageQueue<T>::Remove() {
  std::scoped_lock lock(m_mutex);
  if (m_que.empty()) {
    return std::nullopt;
  }
  T val = m_que.front();
  m_que.pop_front();
  return val;
}

template <typename T>
std::optional<T> MessageQueue<T>::WaitRemove() {
  std::unique_lock<std::mutex> lock(m_mutex);
  m_cv.wait(lock, [this] { return !m_que.empty() || m_closed; });
  if (m_que.empty()) {
    return std::nullopt;
  }
  T val = m_que.front();
  m_que.pop_front();
  return val;
}

template <typename T>
int MessageQueue<T>::Size() {
  std::scoped_lock lock(m_mutex);
  return static_cast<int>(m_que.size());
}
