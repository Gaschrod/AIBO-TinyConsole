//
// Copyright 2002 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#include <stdio.h>
#include <stdlib.h>
#include "BMP.h"

const size_t LAYER_H_WIDTH   = 176;
const size_t LAYER_H_HEIGHT  = 144;
const size_t LAYER_H_RAWSIZE = 3 * LAYER_H_WIDTH * LAYER_H_HEIGHT;

main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr,
                "Usage: layerh2bmp layerH.raw basepath \n");
        exit(1);
    }

    unsigned char* raw = (unsigned char*)malloc(LAYER_H_RAWSIZE);
    if (raw == 0) {
        fprintf(stderr, "layer2bmp : can't allocate memory.\n");
        exit(1);
    }

    FILE* fp = fopen(argv[1], "r");
    if (fp == 0) {
        fprintf(stderr, "layer2bmp : can't open %s\n", argv[1]);
        exit(1);
    }
    fread(raw, 1, LAYER_H_RAWSIZE, fp);
    fclose(fp);

    BMP bmp;
    bmp.SaveRawDataAsCDT(argv[2], raw, 176, 144);
}
