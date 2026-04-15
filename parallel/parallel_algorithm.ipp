#include <algorithm>
#include <thread>
#include <vector>

namespace parallel_algorithm {

template <typename T, typename F>
std::vector<T> Transform(const std::vector<T>& input, F func) {
  std::vector<T> result(input.size());

  size_t num_threads = std::thread::hardware_concurrency();
  num_threads = (num_threads == 0) ? 4 : num_threads;

  // clamp to input size (no point having more threads than elements)
  if (num_threads > input.size()) {
    num_threads = input.size();
  }

  if (num_threads == 0) {
    return result;
  }

  // the calling thread counts as one, so spawn num_threads - 1 extra threads
  const size_t extra_threads = num_threads - 1;

  size_t chunk = input.size() / num_threads;
  size_t leftover = input.size() % num_threads;

  // worker lambda: process [start, end)
  auto worker = [&](size_t start, size_t end) {
    for (size_t i = start; i < end; i++) {
      result[i] = func(input[i]);
    }
  };

  std::vector<std::jthread> threads;
  threads.reserve(extra_threads);

  size_t current = 0;
  for (size_t t = 0; t < extra_threads; t++) {
    size_t end = current + chunk + (t < leftover ? 1 : 0);
    threads.emplace_back(worker, current, end);
    current = end;
  }

  // calling thread does its share
  worker(current, input.size());

  // jthreads join automatically on destruction
  return result;
}

template <typename T, typename F>
T Reduce(const std::vector<T>& input, const T& init, F func) {
  if (input.empty()) {
    return init;
  }

  size_t num_threads = std::thread::hardware_concurrency();
  num_threads = (num_threads == 0) ? 4 : num_threads;

  // need at least 2 elements per thread for reduce to make sense
  // clamp threads so each thread gets at least 2 elements
  size_t max_useful = input.size() / 2;
  num_threads = std::min(num_threads, max_useful);
  if (num_threads == 0) {
    num_threads = 1;
  }

  const size_t extra_threads = num_threads - 1;
  size_t chunk = input.size() / num_threads;
  size_t leftover = input.size() % num_threads;

  std::vector<T> partial(num_threads, init);

  auto worker = [&](size_t tid, size_t start, size_t end) {
    T acc = input[start];
    for (size_t i = start + 1; i < end; i++) {
      acc = func(acc, input[i]);
    }
    partial[tid] = acc;
  };

  std::vector<std::jthread> threads;
  threads.reserve(extra_threads);

  size_t current = 0;
  for (size_t t = 0; t < extra_threads; t++) {
    size_t end = current + chunk + (t < leftover ? 1 : 0);
    threads.emplace_back(worker, t, current, end);
    current = end;
  }

  // calling thread handles the last chunk
  worker(num_threads - 1, current, input.size());

  // jthreads join on destruction (threads vector goes out of scope)
  threads.clear();

  // combine partial results with init
  T result = partial[0];
  for (size_t t = 1; t < num_threads; t++) {
    result = func(result, partial[t]);
  }
  result = func(result, init);

  return result;
}

};  // namespace parallel_algorithm
