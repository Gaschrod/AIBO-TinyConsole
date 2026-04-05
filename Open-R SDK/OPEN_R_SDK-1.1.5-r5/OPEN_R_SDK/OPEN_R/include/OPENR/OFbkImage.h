//
// OFbkImage.h
//
// Copyright 1997,1998,1999,2000,2001,2002,2004 Sony Corporation
//

#ifndef _OFbkImage_h_DEFINED
#define _OFbkImage_h_DEFINED

#include <OPENR/ODataFormats.h>

typedef longword OFbkImageBand;
const OFbkImageBand ofbkimageBAND_Y    = 0; // Y_LL
const OFbkImageBand ofbkimageBAND_Cr   = 1;
const OFbkImageBand ofbkimageBAND_Cb   = 2;
const OFbkImageBand ofbkimageBAND_CDT  = 3;
const OFbkImageBand ofbkimageBAND_Y_LH = 4;
const OFbkImageBand ofbkimageBAND_Y_HL = 5;
const OFbkImageBand ofbkimageBAND_Y_HH = 6;

class OFbkImage {
public:
    OFbkImage(OFbkImageInfo* info, byte* data, OFbkImageBand band);
    ~OFbkImage();

    OFbkImage& operator =(const OFbkImage& fbk_image);

    bool   IsValid() const { return image_ != 0 ? true : false; }
    byte*  Pointer() const { return image_;    }
    int    Width()   const { return width_;    }
    int    Height()  const { return height_;   }
    int    Skip()    const { return skip_;     }

    inline byte Pixel(int x, int y) const;

    // Tag Information
    inline word FieldCounter() const;
    inline byte ColorFrequency(OCdtChannel chan) const;

    // OBSOLETE
    longword GetTagInfo();

private:
    byte* image_;
    int   width_;
    int   height_;
    int   skip_;
};

byte
OFbkImage::Pixel(int x, int y) const
{
    return *(image_ + (width_ + skip_) * y + x);
}

word
OFbkImage::FieldCounter() const
{
    byte* tag = image_ + (width_ + skip_) * (height_ - 1) + 0;
    return ((word)tag[1] << 8) | (word)tag[0];
}

byte
OFbkImage::ColorFrequency(OCdtChannel chan) const
{
    byte freq;
    byte* tag = image_ + (width_ + skip_) * (height_ - 1) + 0;

    if (chan == ocdtCHANNEL0) {
        freq = tag[8];
    } else if (chan == ocdtCHANNEL1) {
        freq = tag[9];
    } else if (chan == ocdtCHANNEL2) {
        freq = tag[10];
    } else if (chan == ocdtCHANNEL3) {
        freq = tag[11];
    } else if (chan == ocdtCHANNEL4) {
        freq = tag[4];
    } else if (chan == ocdtCHANNEL5) {
        freq = tag[5];
    } else if (chan == ocdtCHANNEL6) {
        freq = tag[6];
    } else if (chan == ocdtCHANNEL7) {
        freq = tag[7];
    } else {
        freq = 0;
    }

    return freq;
}

#endif /* _OFbkImage_h_DEFINED */
