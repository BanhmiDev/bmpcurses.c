#include "bmpcurses.h"
#include <stdlib.h>
#include <math.h>
#include <ncurses.h>

static int const BLOCK_WIDTH = 8;
static int const BLOCK_HEIGHT = 8;

int main(int argc, char *argv[])
{
    WINDOW *window = initscr();
    start_color();

    if (argc <= 1) {
        return pixedError("No image file specified!");
    }

    if (!has_colors()) {
        return pixedError("Colors not supported!");
    }

    Pixel *dataArray; /* Array of original image */
    Pixel *averagedArray; /* Array after averaging */

    FILE *file;
    file = fopen(argv[1], "rb");
    if (file == NULL) {
        return pixedError("File not found!");
    }

    unsigned int dataOffset = getDataOffset(file);
    unsigned int width = getWidth(file);
    unsigned int height = getHeight(file);
    unsigned int size = width * height;

    dataArray = malloc(sizeof(Pixel) * size);
    dataArray = getDataArray(file, dataOffset, size);
    averagedArray = getAveragedArray(dataArray, size, width, height);  
 
    displayImage(averagedArray, ceil(ceil(size/BLOCK_WIDTH)/BLOCK_HEIGHT), ceil(width/BLOCK_WIDTH), ceil(height/BLOCK_HEIGHT));
    
    free(dataArray);
    free(averagedArray);

    refresh();
    getch(); /* Preserve ncurses window */

    return EXIT_SUCCESS;
}

unsigned int getDataOffset(FILE *file)
{
    unsigned int offset;

    /* Offset, i.e. starting address where the bitmap
       image data can be found has the size of 4 bytes */
    fseek(file, 0x0A, SEEK_SET);
    fread(&offset, 4, 1, file);

    return offset;
}

unsigned int getWidth(FILE *file)
{
    unsigned int width;

    /* Offset: width at 0x12 (hexadecimal offset) */
    fseek(file, 0x12, SEEK_SET);
    fread(&width, 2, 1, file);

    return width;
}

unsigned getHeight(FILE *file)
{
    unsigned int height;

    /* Offset: height at 0x16 */
    fseek(file, 0x16, SEEK_SET);
    fread(&height, 2, 1, file);

    return height;
}

Pixel* getDataArray(FILE *file, unsigned int index, unsigned int size)
{
    Pixel *dataArray = malloc(sizeof(Pixel) * size);

    /* Order: RGBA */
    for (int i = 0; i < size; i++) {
        fseek(file, index + (i * 4), SEEK_SET);
        fread(&dataArray[i].a, 1, 1, file);
        
        fseek(file, index + (i * 4) + 1, SEEK_SET);
        fread(&dataArray[i].b, 1, 1, file);
        
        fseek(file, index + (i * 4) + 2, SEEK_SET);
        fread(&dataArray[i].g, 1, 1, file);
        
        fseek(file, index + (i * 4) + 3, SEEK_SET);
        fread(&dataArray[i].r, 1, 1, file);
    }

    return dataArray;
}

Pixel* getAveragedArray(Pixel *pixel, unsigned int size, unsigned int width, unsigned int height)
{
    /* Block constraints are defined as constants */
    unsigned int averagedSize = ceil(ceil(size/BLOCK_WIDTH)/BLOCK_HEIGHT);
    Pixel *averagedArray = malloc(sizeof(Pixel) * averagedSize);
  
    /* Divide by BLOCK_WIDTH*BLOCK_HEIGHT to average pixel values */
    int divisor = BLOCK_WIDTH*BLOCK_HEIGHT;

    for (int i = 0; i < ceil(height/BLOCK_HEIGHT); i++) {
        for (int j = 0; j < ceil(width/BLOCK_WIDTH); j++) {
            /* Iterate over real data array */
            for (int y = 0; y < BLOCK_HEIGHT; y++) {
                for (int x = 0; x < BLOCK_WIDTH; x++) {
                    /* Assign pixels */
                    averagedArray[(int)(i * (width/BLOCK_WIDTH) + j)] = addPixels(averagedArray[(int)(i * (width/BLOCK_WIDTH) + j)], pixel[(int)(((i * BLOCK_HEIGHT + y) * width) + (j * BLOCK_WIDTH + x))]);     
                }
            }

            /* Average pixel values */
            averagedArray[(int)(i * (width/BLOCK_WIDTH) + j)].r /= divisor;
            averagedArray[(int)(i * (width/BLOCK_WIDTH) + j)].g /= divisor;
            averagedArray[(int)(i * (width/BLOCK_WIDTH) + j)].b /= divisor;
            averagedArray[(int)(i * (width/BLOCK_WIDTH) + j)].a /= divisor;
        }
    }

    return averagedArray;
}

short int getGreyscale(Pixel *pixel, unsigned int index)
{
    /* Get greyscale of a color by averaging r + g + b data and translating to ncurses' RGB.
       ncurses RGB color value ranges from 0-1000.
       Translating from ncurses RGB: 255_rgb = value * 0.255 
       Translating in reverse: ncurses_rgb = value / 0.255 */
    short int greyscale = ((pixel[index].r + pixel[index].g + pixel[index].b) / 3) / 0.255;
    return (int)(greyscale / 62); 
}

Pixel addPixels(Pixel p1, Pixel p2)
{  
    Pixel result;
    result.r = p1.r + p2.r;
    result.g = p1.g + p2.g;
    result.b = p1.b + p2.b;
    result.a = p1.a + p2.a;
    return result;
}

void displayImage(Pixel *pixel, unsigned int size, unsigned int width, unsigned int height)
{
    /* Init first 16 greyscale colors. */
    for (int i = 0; i < 16; i++) {
        init_color(i, i * 62, i * 62, i * 62);
        init_pair(i, i, i);
    }
  
    /* BMP arranges data from bottom left to top right */
    for (int i = height-1; i > 0; i--) {
        for (int j = 0; j < width; j++) {
            attron(COLOR_PAIR(getGreyscale(pixel, (i * width) + j)));
            mvprintw(abs(i-height), j, " ");
        }
    }
}

int pixedError(char message[])
{
    printw(message);
    printw("\nPress any key to exit...\n");
    refresh();
    getch();
    endwin();
    return EXIT_FAILURE;
}
