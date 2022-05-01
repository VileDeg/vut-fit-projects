#include <stdlib.h>

#include "ppm.h"
#include "error.h"

#define U_SIZE sizeof(unsigned)
#define MAX_XY 8000 //Maximum possible resolution for both X and Y
#define COLOR_MAXVAL 255u //Maximum color value to be used in image

ppm* ppm_read(const char* filename)
{
    //Try open file
	FILE* fp = fopen(filename, "rb");
	if (!fp)
		error_exit("Unable to open file: %s\n", filename);

	char buff[16]; //Buffer for reading
    //Try read format
	if (!fgets(buff, sizeof(buff), fp))
		error_exit("Unable to read image format: %s\n", filename);
    //Check if format is 'P6'
    if (buff[0] != 'P' || buff[1] != '6')
    {
        warning_msg("Invalid format (must be \"P6\"): %s\n", filename);
        return NULL;
    }
    //Temp variables for image size
    unsigned xsize = 0;
    unsigned ysize = 0;
    //Try read ppm image size
    if (fscanf(fp, "%u %u", &xsize, &ysize) != 2)
	    error_exit("Unable to read image size: %s\n", filename);
    //Check size
	if (xsize > MAX_XY || ysize > MAX_XY)
		error_exit("Image size is too big: %s\n", filename);		
    //Try read color value
    unsigned maxval = 0;
    if (fscanf(fp, "%u", &maxval) != 1)
        error_exit("Unable to read color value: %s\n", filename);
    //Check if color value is 255
    if (maxval != COLOR_MAXVAL)
        error_exit("Invalid maximum color value (not 255): %s\n", filename);
    //Skip to the end of line
    while (fgetc(fp) != '\n');
    //Allocate memory for struct
    ppm* img = calloc(1, U_SIZE*2 + xsize * ysize * 3);
    if (!img)
        error_exit("Memory allocation failed!\n");

    img->xsize = xsize;
    img->ysize = ysize;
    //Read the image data
    if (fread(img->data, 3 * img->xsize, img->ysize, fp) != img->ysize)
        error_exit("Unable to read image: %s\n", filename);

    fclose(fp);
    return img;
}

void ppm_free(ppm *p)
{
	free(p->data);
	free(p);
	p = NULL;
}
