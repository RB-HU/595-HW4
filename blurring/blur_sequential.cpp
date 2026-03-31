#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "./qdbmp.hpp"

using std::cerr;
using std::string;
using std::vector;

// compute box blur for a single pixel (x, y) using original pixel data
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

int main(int argc, char* argv[]) {
  if (argc != 4) {
    cerr << "Usage: " << argv[0] << " <input file> <output file> <box size>\n";
    return EXIT_FAILURE;
  }

  const string input_fname{argv[1]};
  const string output_fname{argv[2]};
  const int box_size = std::stoi(argv[3]);

  if (box_size <= 0) {
    cerr << "Error: box size must be positive\n";
    return EXIT_FAILURE;
  }

  BitMap image(input_fname);
  if (image.check_error() != BMP_OK) {
    perror("ERROR: Failed to open BMP file.");
    return EXIT_FAILURE;
  }

  const UINT width = image.width();
  const UINT height = image.height();

  // read all original pixels into a 2D vector to avoid using modified values
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

  // apply box blur to each pixel
  for (UINT y = 0; y < height; y++) {
    for (UINT x = 0; x < width; x++) {
      const RGB blurred = BlurPixel(original, x, y, box_size, width, height);
      output.set_pixel(x, y, blurred);
    }
  }

  output.write_file(output_fname);
  if (output.check_error() != BMP_OK) {
    perror("ERROR: Failed to write output BMP.");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
