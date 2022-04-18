#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <err.h>
#include <string.h>

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} __attribute__((packed)) pixel_t;

long parse_arg(char const *const p);

long parse_arg(char const *const p) {
	char *endptr;
	long val = strtol(p, &endptr, 10);
	if (*p != '\0' && *endptr == '\0') {
		return val;
	} else {
		errx(1, "Invalid number");
	}
}

void dump_frame(pixel_t const *const canvas, int canvas_len);

void dump_frame(pixel_t const *const canvas, int canvas_len)
{
	// Just dump it, it's rawvideo and byte order match
	fwrite(canvas, 1, canvas_len, stdout);
}

int main(int argc, char **argv)
{
	if (argc != 4) {
		errx(1, "Invalid arguments");
	}

	// Start with white canvas
	long width = parse_arg(argv[1]);
	long height = parse_arg(argv[2]);
	long spf = parse_arg(argv[3]); // seconds per frame
	
	int canvas_len = sizeof(pixel_t)*height*width;
	pixel_t *canvas = malloc(canvas_len);
	if (canvas == NULL) {
		err(1, "Unable to allocate memory");
	}
	memset(canvas, 0xff, canvas_len);

	long tick = 0;
	long line = 0;
	
	while (++line) {
		long ts;
		int x, y, n;
		n = scanf("%ld\t%d\t%d\t", &ts, &x, &y);
		if (n == EOF) {
			// Finished
			break;
		} else if (n != 3) {
			errx(2, "Line %ld: Format error (start)", line);
		}

		// Validate input
		if (x < 0 || x >= width || y < 0 || y >= height) {
			errx(2, "Line %ld: Invalid coordinates: %d,%d", line, x, y);
		}
		pixel_t *pix = canvas + width * y + x;

		// Generate frame if wanted.
		if (tick < ts) {
			tick += spf;
			if (tick < ts) {
				// Frameskip if too much idle
				tick = ts;
			}
			dump_frame(canvas, canvas_len);
		}
		
		// Read remaining line
		n = scanf("%" SCNu8 "\t%" SCNu8 "\t%" SCNu8, &(pix->r), &(pix->g), &(pix->b));
		if (n != 3) {
			errx(2, "Line %ld: Format error (end)", line);
		}
		char c = getc(stdin);
		if (c != '\n') {
			errx(2, "Line %ld: Not enough data", line);
		}
	}
	dump_frame(canvas, canvas_len);
}
