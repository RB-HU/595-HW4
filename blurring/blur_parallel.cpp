#include <pthread.h>

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "./qdbmp.hpp"

using std::cerr;
using std::string;
using std::vector;

// arguments passed to each thread
struct ThreadArgs {
  const vector<vector<RGB>>* original;
  BitMap* output;
  UINT width;
  UINT height;
  int box_size;
  UINT start_row;  // inclusive
  UINT end_row;    // exclusive
};

// compute box blur for a single pixel using original pixel data
static RGB BlurPixel(const vector<vector<RGB>>& original,
                     UINT x,
                     UINT y,
                     int box_size,
                     UINT width,
                     UINT height) {
  long sum_r = 0;
  long sum_g = 0;
  long sum_b = 0;
  int count = 0;

  const int x_min = static_cast<int>(x) - box_size;
  const int x_max = static_cast<int>(x) + box_size;
  const int y_min = static_cast<int>(y) - box_size;
  const int y_max = static_cast<int>(y) + box_size;

  for (int ny = y_min; ny <= y_max; ny++) {
    if (ny < 0 || ny >= static_cast<int>(height)) {
      continue;
    }
    for (int nx = x_min; nx <= x_max; nx++) {
      if (nx < 0 || nx >= static_cast<int>(width)) {
        continue;
      }
      const RGB& px = original[ny][nx];
      sum_r += px.red;
      sum_g += px.green;
      sum_b += px.blue;
      count++;
    }
  }

  return RGB{static_cast<UCHAR>(sum_r / count),
             static_cast<UCHAR>(sum_g / count),
             static_cast<UCHAR>(sum_b / count)};
}

// thread function: blurs rows [start_row, end_row)
static void* BlurRows(void* arg) {
  const ThreadArgs* args = static_cast<ThreadArgs*>(arg);
  for (UINT y = args->start_row; y < args->end_row; y++) {
    for (UINT x = 0; x < args->width; x++) {
      const RGB blurred = BlurPixel(*args->original, x, y, args->box_size,
                                    args->width, args->height);
      args->output->set_pixel(x, y, blurred);
    }
  }
  return nullptr;
}

int main(int argc, char* argv[]) {
  if (argc != 5) {
    cerr << "Usage: " << argv[0]
         << " <input file> <output file> <box size> <thread count>\n";
    return EXIT_FAILURE;
  }

  const string input_fname{argv[1]};
  const string output_fname{argv[2]};
  const int box_size = std::stoi(argv[3]);
  const int num_threads = std::stoi(argv[4]);

  if (box_size <= 0) {
    cerr << "Error: box size must be positive\n";
    return EXIT_FAILURE;
  }

  if (num_threads <= 0) {
    cerr << "Error: thread count must be positive\n";
    return EXIT_FAILURE;
  }

  BitMap image(input_fname);
  if (image.check_error() != BMP_OK) {
    perror("ERROR: Failed to open BMP file.");
    return EXIT_FAILURE;
  }

  const UINT width = image.width();
  const UINT height = image.height();

  // read all original pixels into a 2D vector
  vector<vector<RGB>> original(height, vector<RGB>(width, RGB{0, 0, 0}));
  for (UINT y = 0; y < height; y++) {
    for (UINT x = 0; x < width; x++) {
      original[y][x] = image.get_pixel(x, y);
    }
  }

  BitMap output(width, height);
  if (output.check_error() != BMP_OK) {
    perror("ERROR: Failed to create output BMP.");
    return EXIT_FAILURE;
  }

  // divide rows evenly among threads
  const UINT n = static_cast<UINT>(num_threads);
  vector<pthread_t> threads(n);
  vector<ThreadArgs> args(n);

  const UINT rows_per_thread = height / n;
  const UINT leftover = height % n;

  UINT current_row = 0;
  for (UINT i = 0; i < n; i++) {
    // distribute leftover rows to the first few threads
    const UINT rows = rows_per_thread + (i < leftover ? 1 : 0);
    args[i] = ThreadArgs{&original, &output, width,      height,
                         box_size,  current_row, current_row + rows};
    current_row += rows;
    pthread_create(&threads[i], nullptr, BlurRows, &args[i]);
  }

  // wait for all threads to finish before writing output
  for (UINT i = 0; i < n; i++) {
    pthread_join(threads[i], nullptr);
  }

  output.write_file(output_fname);
  if (output.check_error() != BMP_OK) {
    perror("ERROR: Failed to write output BMP.");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
