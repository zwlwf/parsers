#define  STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(int argc, char** argv) {
	int x,y,n;
	unsigned char* data = stbi_load(argv[1], &x, &y, &n, 0);
	if(!data) {
		printf("failed to load image\n");
		return -1;
	}
	printf("image size is %d X %d with %d channels\n", x, y, n);
	return 0;
}
