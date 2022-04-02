// steg-decode.c
// Řešení IJC-DU1, příklad b), 20.3.2022
// Autor: Vadim Goncearenco, FIT
// Přeloženo: gcc 7.5.0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "ppm.h"
#include "bitset.h"
#include "eratosthenes.h"

#define START_BIT 29 //Bit index where the secret message starts

int main(int argc, char* argv[])
{
	if (argc != 2)
		error_exit("Invalid number of arguments!\n");

	char* filename = argv[1];
	//Get all the image data
	ppm* img = ppm_read(filename);
	if (!img)
		goto mem_error_img;

	ul img_size = img->xsize * img->ysize * 3;
	//Create bit array according to image size
	bitset_alloc(bs, img_size);
	//Set all prime number bits to 0
	Eratosthenes(bs);
	//Allocate buffer for secret message
	ul cap  = 16;
	ul len = 0;
	char* msg = calloc(1, cap);
	if (!msg)
		goto mem_error_msg;

	ul p_cnt = 0; //Prime numbers count
	//Loop through whole data starting from 'START_BIT'
	for (ul i = START_BIT; i < img_size; i++)
	{
		if (bitset_getbit(bs, i) != 0)
			continue;

		char byte = img->data[i]; //Current byte in image
		char lsb = byte & 1; //LSB in current byte

		if ( !(p_cnt % 8) ) //Every 8th prime number do this:
		{			
			if (len == cap) //Increase message buffer
			{
				cap *= 2;
				char* tmp = realloc(msg, cap);
				if (!tmp)
					goto mem_error_msg;
				msg = tmp;
				memset(msg+len, '\0', cap-len);
			}
			len++;
		}

		ul ind = p_cnt / 8;
		u offset = p_cnt % 8;
		//Add LSB to current byte in buffer
		msg[ind] |= (char)(lsb << offset);

		p_cnt++;
		//If a "zero byte" is found, end the algorithm
		if ((p_cnt % 8 == 0) && (msg[(p_cnt-1)/8] == '\0'))
			break;
	}
	//Print out the secret message
	printf("%s\n", msg);

	free(img); img = NULL;
	free(msg); msg = NULL;
	bitset_free(bs);
	return 0;

mem_error_msg:
	free(msg); msg = NULL;
mem_error_img:
	free(img); img = NULL;
	error_exit("Memory allocation failed!\n");
	return MEM_ERR;
}