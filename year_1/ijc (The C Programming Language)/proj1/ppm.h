#ifndef _PPM_H_
#define _PPM_H_

//Structure to hold .ppm image data and size
typedef struct
{
    unsigned xsize;
    unsigned ysize;
    char data[];
} ppm;

//Read .ppm image data from file
ppm* ppm_read(const char * filename);

void ppm_free(ppm *p);

#endif //_PPM_H_