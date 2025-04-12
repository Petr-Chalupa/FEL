#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define OUT_PPM "output.ppm"
#define OUT_HISTOGRAM "output.txt"

#define COLOR_MIN 0
#define COLOR_MAX 255

enum {
  ERR_TOO_FEW_ARGS = 100,
  ERR_FILE_NOT_FOUND = 101,
  ERR_FILE_READ_FAILURE = 102,
  ERR_FILE_WRITE_FAILURE = 103,
  ERR_MALLOC_FAILURE = 104,
};

int read_image(char const *filename, unsigned char **img, size_t *width, size_t *height);
int apply_mask(int mask[3][3], unsigned char *img, size_t width, size_t height, unsigned char **new_img);
int write_image(unsigned char *img, size_t width, size_t height);
int write_histogram(unsigned char *img, size_t width, size_t height);

int main(int argc, char const *argv[]) {
  // Read input filename
  if (argc < 2) return ERR_TOO_FEW_ARGS;
  char const *in_filename = argv[1];

  // Read RGB values from file
  unsigned char *img = NULL;
  size_t width = 0;
  size_t height = 0;
  int read_r = read_image(in_filename, &img, &width, &height);
  if (read_r != 0) return read_r;

  // Apply convolution mask to image
  int mask[3][3] = {{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}};
  unsigned char *new_img = NULL;
  int apply_r = apply_mask(mask, img, width, height, &new_img);
  if (apply_r != 0) return apply_r;

  // Write new image to file
  int write_r = write_image(new_img, width, height);
  if (write_r != 0) return write_r;

  // Write histogram to file
  int write_histogram_r = write_histogram(new_img, width, height);
  if (write_histogram_r != 0) return write_histogram_r;

  free(img);
  free(new_img);
  return 0;
}

int read_image(char const *filename, unsigned char **img, size_t *width, size_t *height) {
  FILE *file = fopen(filename, "rb");
  if (file == NULL) return ERR_FILE_NOT_FOUND;

  if (fscanf(file, "P6\n%zu\n%zu\n255\n", width, height) < 2) {
    fclose(file);
    return ERR_FILE_READ_FAILURE;
  }

  *img = malloc(3 * (*width) * (*height) * sizeof(char));
  if (img == NULL) {
    fclose(file);
    return ERR_MALLOC_FAILURE;
  }

  if (fread(*img, 3, (*width) * (*height), file) != (*width) * (*height)) {
    free(*img);
    fclose(file);
    return ERR_FILE_READ_FAILURE;
  }

  fclose(file);
  return 0;
}

int apply_mask(int mask[3][3], unsigned char *img, size_t width, size_t height, unsigned char **new_img) {
  *new_img = malloc(3 * width * height * sizeof(char));
  if (*new_img == NULL) {
    return ERR_MALLOC_FAILURE;
  }

  for (size_t i = 0; i < height; i++) {
    for (size_t j = 0; j < width; j++) {
      for (size_t k = 0; k < 3; k++) {
        // k: R=0, G=1, B=2
        int new_value = 0;

        if (i == 0 || j == 0 || i == height - 1 || j == width - 1) {
          // Mask not fully overlapping the image, copy pixels
          new_value = img[3 * (i * width + j) + k];
        } else {
          // Apply mask
          for (int mi = -1; mi <= 1; mi++) {
            for (int mj = -1; mj <= 1; mj++) {
              new_value += img[3 * ((i + mi) * width + (j + mj)) + k] * mask[mi + 1][mj + 1];
            }
          }
        }

        if (new_value < COLOR_MIN) new_value = COLOR_MIN;
        if (new_value > COLOR_MAX) new_value = COLOR_MAX;

        (*new_img)[3 * (i * width + j) + k] = (unsigned char)new_value;
      }
    }
  }

  return 0;
}

int write_image(unsigned char *img, size_t width, size_t height) {
  FILE *file = fopen(OUT_PPM, "wb");
  if (file == NULL) return ERR_FILE_WRITE_FAILURE;

  fprintf(file, "P6\n%zu\n%zu\n255\n", width, height);
  fwrite(img, 3, width * height, file);

  fclose(file);
  return 0;
}

int write_histogram(unsigned char *img, size_t width, size_t height) {
  FILE *file = fopen(OUT_HISTOGRAM, "w");
  if (file == NULL) return ERR_FILE_WRITE_FAILURE;

  size_t intervals[5] = {0, 0, 0, 0, 0};

  for (size_t i = 0; i < 3 * width * height; i += 3) {
    int grayscale = round(0.2126 * img[i] + 0.7152 * img[i + 1] + 0.0722 * img[i + 2]);
    if (grayscale < 51)
      intervals[0]++;
    else if (grayscale < 102)
      intervals[1]++;
    else if (grayscale < 153)
      intervals[2]++;
    else if (grayscale < 204)
      intervals[3]++;
    else
      intervals[4]++;
  }

  fprintf(file, "%zu %zu %zu %zu %zu", intervals[0], intervals[1], intervals[2], intervals[3], intervals[4]);

  fclose(file);
  return 0;
}

/*
For potentially faster execution, consider the following optimizations:
1.  Inlining all code into main() to minimize function call overhead.
2.  Computing color channels (R, G, B) separately instead of in a loop to enable better compiler optimizations like loop
unrolling or SIMD instructions.
3.  Calculating the histogram during the convolution pass to avoid an extra iteration over the image data.

These were enough for a decent result, but they f.e. avoid dealing with cache optimizations.

Further potential optimizations include:
1.  Optimizing memory access patterns for better cache utilization.
2.  Different loop unrolling strategies.
*/
