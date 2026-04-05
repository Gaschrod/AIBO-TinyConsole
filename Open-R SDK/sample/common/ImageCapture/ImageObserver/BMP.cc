//
// Copyright 2002,2003,2004 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#include <OPENR/OFbkImage.h>
#include <OPENR/OSyslog.h>
#include "BMP.h"

bool 
BMP::SaveYCrCb2RGB(char* path,
                   OFbkImageVectorData* imageVec, OFbkImageLayer layer)
{
    if (layer == ofbkimageLAYER_C) return false;

    byte pixels[3];
    OFbkImageInfo* info = imageVec->GetInfo(layer);
    byte*          data = imageVec->GetData(layer);
    
    OFbkImage yImage(info, data, ofbkimageBAND_Y);
    OFbkImage crImage(info, data, ofbkimageBAND_Cr);
    OFbkImage cbImage(info, data, ofbkimageBAND_Cb);

    BMPHeader     header;
    BMPInfoHeader infoheader;

    infoheader.width     = yImage.Width();
    infoheader.height    = yImage.Height();
    infoheader.imagesize = yImage.Width() * yImage.Height() * 3;

    header.size = infoheader.imagesize + 54;

    FILE* fp = fopen(path, "w");
    if (fp == 0) {
        OSYSLOG1((osyslogERROR, "can't open %s", path));
        return false;
    }

    SaveBMPHeader(fp, header);
    SaveBMPInfoHeader(fp, infoheader);

    for (int y = infoheader.height - 1 ; y >= 0; y--) {
        for (int x = 0; x < infoheader.width; x++) {
            YCrCb2RGB(yImage.Pixel(x, y),
                      crImage.Pixel(x, y),
                      cbImage.Pixel(x, y),
                      &pixels[R_PIXEL], &pixels[G_PIXEL], &pixels[B_PIXEL]);
            fwrite(pixels, 1, 3, fp);
        }
    }

    fclose(fp);
    return true;
}

bool 
BMP::SaveLayerC(char* basepath, OFbkImageVectorData* imageVec)
{
    byte pixels[3];
    char path[128];
    OFbkImageInfo* info = imageVec->GetInfo(ofbkimageLAYER_C);
    byte*          data = imageVec->GetData(ofbkimageLAYER_C);
    
    OFbkImage cdtImage(info, data, ofbkimageBAND_CDT);

    BMPHeader     header;
    BMPInfoHeader infoheader;

    infoheader.width     = cdtImage.Width();
    infoheader.height    = cdtImage.Height();
    infoheader.imagesize = cdtImage.Width() * cdtImage.Height() * 3;

    header.size = infoheader.imagesize + 54;

    for (int i = 0; i < ocdtNUM_CHANNELS; i++) {

        byte plane = 0x01 << i;
        sprintf(path, "%s%d.BMP", basepath, i);
        
        FILE* fp = fopen(path, "w");
        if (fp == 0) {
            OSYSLOG1((osyslogERROR, "can't open %s", path));
            return false;
        }

        SaveBMPHeader(fp, header);
        SaveBMPInfoHeader(fp, infoheader);

        for (int y = infoheader.height - 1 ; y >= 0; y--) {
            for (int x = 0; x < infoheader.width; x++) {
                if (cdtImage.Pixel(x, y) & plane) {
                    pixels[R_PIXEL] = pixels[G_PIXEL] = pixels[B_PIXEL] = 255;
                } else {
                    pixels[R_PIXEL] = pixels[G_PIXEL] = pixels[B_PIXEL] = 0;
                }
                fwrite(pixels, 1, 3, fp);
            }
        }

        fclose(fp);
    }

    return true;
}

bool 
BMP::SaveRaw2Gray(char* path,
                  byte* image, int width, int height, int skip)
{
    byte pixels[3];
    BMPHeader     header;
    BMPInfoHeader infoheader;

    infoheader.width     = width;
    infoheader.height    = height;
    infoheader.imagesize = width * height * 3;

    header.size = infoheader.imagesize + 54;

    FILE* fp = fopen(path, "w");
    if (fp == 0) {
        OSYSLOG1((osyslogERROR, "can't open %s", path));
        return false;
    }

    SaveBMPHeader(fp, header);
    SaveBMPInfoHeader(fp, infoheader);

    for (int y = infoheader.height - 1 ; y >= 0; y--) {
        for (int x = 0; x < infoheader.width; x++) {
            byte pixel = *(image + (width + skip) * y + x);
            pixels[R_PIXEL] = pixels[G_PIXEL] = pixels[B_PIXEL] = pixel;
            fwrite(pixels, 1, 3, fp);
        }
    }

    fclose(fp);
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
    // offset binary [0, 255] -> signed char [-128, 127]
    sbyte scr = (sbyte)(cr ^ 0x80);
    sbyte scb = (sbyte)(cb ^ 0x80);

    double Y  = (double)y / 255.0;   //  0.0 <= Y  <= 1.0
    double Cr = (double)scr / 128.0; // -1.0 <= Cr <  1.0
    double Cb = (double)scb / 128.0; // -1.0 <= Cb <  1.0

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
