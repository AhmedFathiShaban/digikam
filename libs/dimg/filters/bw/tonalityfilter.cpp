/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : Change tonality image filter
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "tonalityfilter.h"

// C++ includes

#include <cstdio>
#include <cmath>

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "dcolor.h"

namespace Digikam
{

TonalityFilter::TonalityFilter(DImg* orgImage, QObject* parent, const TonalityContainer& settings)
              : DImgThreadedFilter(orgImage, parent, "TonalityFilter")
{
    m_settings = settings;
    initFilter();
}

TonalityFilter::TonalityFilter(uchar* bits, uint width, uint height, bool sixteenBits,
                               const TonalityContainer& settings)
              : DImgThreadedFilter()
{
    m_settings = settings;
    m_orgImage = DImg(width, height, sixteenBits, true, bits, true);
    initFilter();
    startFilterDirectly();
    memcpy(bits, m_orgImage.bits(), m_orgImage.numBytes());
}

TonalityFilter::~TonalityFilter()
{
}

void TonalityFilter::filterImage()
{
    changeTonality(m_orgImage);
    m_destImage.putImageData(m_orgImage.bits());
}

void TonalityFilter::changeTonality(DImg& image)
{
    if (image.isNull()) return;

    changeTonality(image.bits(), image.width(), image.height(), image.sixteenBit());
}

/** Change color tonality of an image for applying a RGB color mask.*/
void TonalityFilter::changeTonality(uchar* bits, uint width, uint height, bool sixteenBit)
{
    if (!bits) return;

    uint size = width*height;
    int  progress;

    int hue, sat, lig;

    DColor mask(m_settings.redMask, m_settings.greenMask, m_settings.blueMask, 0, sixteenBit);
    mask.getHSL(&hue, &sat, &lig);

    if (!sixteenBit)        // 8 bits image.
    {
        uchar* ptr = bits;

        for (uint i = 0 ; i < size ; ++i)
        {
            // Convert to grayscale using tonal mask

            lig = lround(0.3 * ptr[2] + 0.59 * ptr[1] + 0.11 * ptr[0]);

            mask.setRGB(hue, sat, lig, sixteenBit);

            ptr[0] = (uchar)mask.blue();
            ptr[1] = (uchar)mask.green();
            ptr[2] = (uchar)mask.red();
            ptr += 4;

            progress = (int)(((double)i * 100.0) / size);
            if ( progress%5 == 0 )
                postProgress( progress );
        }
    }
    else               // 16 bits image.
    {
        unsigned short* ptr = (unsigned short *)bits;

        for (uint i = 0 ; i < size ; ++i)
        {
            // Convert to grayscale using tonal mask

            lig = lround(0.3 * ptr[2] + 0.59 * ptr[1] + 0.11 * ptr[0]);

            mask.setRGB(hue, sat, lig, sixteenBit);

            ptr[0] = (unsigned short)mask.blue();
            ptr[1] = (unsigned short)mask.green();
            ptr[2] = (unsigned short)mask.red();
            ptr += 4;

            progress = (int)(((double)i * 100.0) / size);
            if ( progress%5 == 0 )
                postProgress( progress );
        }
    }
}

}  // namespace Digikam
