/*******************************************************************************
 * Author: Mingxu Hu
 * Dependency:
 * Test:
 * Execution:
 * Description:
 *
 * Manual:
 * ****************************************************************************/

#include "ImageFunctions.h"

void translate(Image& dst,
               const Image& src,
               const double nTransCol,
               const double nTransRow)
{
    double rCol = nTransCol / src.nColRL();
    double rRow = nTransRow / src.nRowRL();

    IMAGE_FOR_EACH_PIXEL_FT(src)
    {
        double phase = 2 * M_PI * (i * rCol + j * rRow);
        dst.setFT(src.getFT(i, j) * COMPLEX_POLAR(-phase), i, j);
    }
}

void meshReverse(Image& img)
{
    IMAGE_FOR_EACH_PIXEL_FT(img)
        if ((i + j) % 2 == 1)
            img.setFT(-img.getFT(i, j), i, j);
}

void meshReverse(Volume& vol)
{
    VOLUME_FOR_EACH_PIXEL_FT(vol)
        if ((i + j + k) % 2 == 1)
            vol.setFT(-vol.getFT(i, j, k), i, j, k);
}

/***
void meshReverse(Image& img)
{
    for (int j = 0; j < img.nRowFT(); j++)
        for (int i = 0; i < img.nColFT(); i++)
            if ((i + j) % 2 == 1)
                img[j * img.nColFT() + i] *= -1;
}

void meshReverse(Volume& vol)
{
    for (int k = 0; k < vol.nSlcFT(); k++)
        for (int j = 0; j < vol.nRowFT(); j++)
            for (int i = 0; i < vol.nColFT(); i++)
                if ((i + j + k) % 2 == 1)
                    vol[k * vol.nColFT() * vol.nRowFT()
                      + j * vol.nColFT()
                      + i] *= -1;
}
***/
