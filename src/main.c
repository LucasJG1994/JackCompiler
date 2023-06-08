#include <stdio.h>

#include "scanner.h"

int main(int argc, char** argv) {
	if(argc == 2) yy_init(argv[1]);
	else printf("Usage: jack <path>");
	return 0;
}