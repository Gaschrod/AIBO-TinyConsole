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

main(int argc, char **argv)
{
    if (argc != 7) {
        fprintf(stderr,
                "Usage: cdt2bmp path y_segment cr_max cr_min cb_max cb_min\n");
        exit(1);
    }

    BMP bmp;
    bmp.SaveCDT(argv[1], atoi(argv[2]),
                atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
}
