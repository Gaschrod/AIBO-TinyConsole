//
// Copyright 2002,2004 Sony Corporation 
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
#include <string.h>
#include "BMP.h"

bool
BMP::SaveCDT(char* path, byte y_segment,
             byte cr_max, byte cr_min, byte cb_max, byte cb_min)
{
    byte pixels[3];
    BMPHeader     header;
    BMPInfoHeader infoheader;
    bool noCDT;

    infoheader.width     = 256;
    infoheader.height    = 256;
    infoheader.imagesize = 256 * 256 * 3;

    header.size = infoheader.imagesize + 54;
    
    FILE* fp = fopen(path, "wb");
    if (fp == 0) {
        fprintf(stderr, "BMP::SaveCDT() : can't open %s\n", path);
        return false;
    }

    SaveBMPHeader(fp, header);
    SaveBMPInfoHeader(fp, infoheader);

    if (cr_max == 128 && cr_min == 128 && cb_max == 128 && cb_min == 128) {
        noCDT = true;
    } else {
        noCDT = false;
    }


    for (int y = 0 ; y < infoheader.height; y++) {
        for (int x = 0; x < infoheader.width; x++) {

            YCrCb2RGB(y_segment * 8 + 4, y, x,
                      &pixels[R_PIXEL], &pixels[G_PIXEL], &pixels[B_PIXEL]);
            
            if (noCDT != true) {
                if ((y == cr_min && cb_min <= x && x <= cb_max) ||
                    (y == cr_max && cb_min <= x && x <= cb_max) ||
                    (x == cb_min && cr_min <= y && y <= cr_max) ||
                    (x == cb_max && cr_min <= y && y <= cr_max)) {
                    pixels[R_PIXEL] = pixels[G_PIXEL] = pixels[B_PIXEL] = 0;
                }
            }

            fwrite(pixels, 1, 3, fp);
        }
    }

    fclose(fp);
    return true;
}

bool
BMP::SaveRawDataAsCDT(char* basepath, byte* data, size_t w, size_t h)
{
    byte pixels[3];
    byte* buf;
    char path[128];

    size_t offY  = 0;
    size_t offCr = w;
    size_t offCb = 2 * w;
    size_t skip  = 2 * w;

    BMPHeader     header;
    BMPInfoHeader infoheader;

    infoheader.width     = 256;
    infoheader.height    = 256;
    infoheader.imagesize = 256 * 256 * 3;

    header.size = infoheader.imagesize + 54;

    buf = (byte*)malloc(256*256);
    if (buf == 0) return false;

    for (int yseg = 0; yseg < 32; yseg++) {

        sprintf(path, "%s%02d.bmp", basepath, yseg);
        byte ypix_min = 8 * yseg;
        byte ypix_max = 8 * yseg + 7;
        memset(buf, 0, 256*256);
        
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                byte ypix = *(data + offY + (w + skip) * y + x);
                if (ypix_min <= ypix && ypix <= ypix_max) {
                    byte cr = *(data + offCr + (w + skip) * y + x);
                    byte cb = *(data + offCb + (w + skip) * y + x);
                    *(buf + 256 * cr + cb) = 1;
                }
            }
        }

        FILE* fp = fopen(path, "w");
        if (fp == 0) {
            fprintf(stderr,
                    "BMP::SaveRawDataAsCDT() : can't open %s\n", path);
            return false;
        }

        SaveBMPHeader(fp, header);
        SaveBMPInfoHeader(fp, infoheader);

        for (int y = 0; y < 256; y++) { // Cr
            for (int x = 0; x < 256; x++) { // Cb
                if (*(buf + 256 * y + x) != 0) {
                    YCrCb2RGB(ypix_min + 4, y, x,
                              &pixels[R_PIXEL],
                              &pixels[G_PIXEL],
                              &pixels[B_PIXEL]);
                } else {
                    pixels[R_PIXEL] = pixels[G_PIXEL] = pixels[B_PIXEL] = 0;
                }
                fwrite(pixels, 1, 3, fp);
            }
        }

        fclose(fp);
    }

    free(buf);

    return true;
}

void
BMP::SaveBMPHeader(FILE* fp, const BMPHeader& header)
{
    fwrite(header.magic, 1, 2, fp);
    write_longword(fp, header.size);
    write_word(fp, header.reserved1);
    write_word(fp, header.reserved2);
    write_longword(fp, header.offset);
}

void
BMP::SaveBMPInfoHeader(FILE* fp, const BMPInfoHeader& infoheader)
{
    fwrite(&infoheader, sizeof(infoheader), 1, fp);
}

void
BMP::YCrCb2RGB(byte y, byte cr, byte cb, byte* r, byte* g, byte* b)
{
    double Y  = (double)y / 255.0;                  //  0.0 <= Y  <= 1.0
    double Cr = ((double)cr * 2.0 - 255.0) / 255.0; // -1.0 <= Cr <= 1.0
    double Cb = ((double)cb * 2.0 - 255.0) / 255.0; // -1.0 <= Cb <= 1.0

    double R = 255.0 * (Y + Cr);
    double G = 255.0 * (Y - 0.51*Cr - 0.19*Cb);
    double B = 255.0 * (Y + Cb);

    if (R > 255.0) {
        *r = 255;
    } else if (R < 0.0) {
        *r = 0;
    } else {
        *r = (byte)R;
    }

    if (G > 255.0) {
        *g = 255;
    } else if (G < 0.0) {
        *g = 0;
    } else {
        *g = (byte)G;
    }

    if (B > 255.0) {
        *b = 255;
    } else if (B < 0.0) {
        *b = 0;
    } else {
        *b = (byte)B;
    }
}

void
BMP::write_word(FILE* fp, word w)
{
    fputc(w & 0xff, fp);
    fputc((w >> 8) & 0xff, fp);
}

void
BMP::write_longword(FILE* fp, longword l)
{
    fputc(l & 0xff, fp);
    fputc((l >> 8) & 0xff, fp);
    fputc((l >> 16) & 0xff, fp);
    fputc((l >> 24) & 0xff, fp);
}

void
BMP::write_slongword(FILE* fp, slongword sl)
{
    fputc(sl & 0xff, fp);
    fputc((sl >> 8) & 0xff, fp);
    fputc((sl >> 16) & 0xff, fp);
    fputc((sl >> 24) & 0xff, fp);
}
