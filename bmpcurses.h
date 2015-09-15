#ifndef BMPCURSES_H
#define BMPCURSES_H

#include <stdio.h>

typedef struct {
    unsigned int r;
    unsigned int g;
    unsigned int b;
    unsigned int a;
} Pixel;

unsigned int getDataOffset(FILE *file);
unsigned int getWidth(FILE *file);
unsigned int getHeight(FILE *file);

Pixel* getDataArray(FILE *file, unsigned int index, unsigned int size); /* Get the pixel data */
Pixel* getAveragedArray(Pixel *pixel, unsigned int size, unsigned int width, unsigned int height); /* Average the pixel data */

short int getGreyscale(Pixel *pixel, unsigned int index); /* Get greyscale value */
Pixel addPixels(Pixel p1, Pixel p2); /* Add two pixels */

void displayImage(Pixel *pixel, unsigned int size, unsigned int width, unsigned int height); /* Display greyscale image in ncurses */

int pixedError(char message[]); /* Error message via ncurses */

#endif
