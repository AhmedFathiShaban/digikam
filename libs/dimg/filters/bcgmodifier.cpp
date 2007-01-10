/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date   : 2005-03-06
 * Description : a Brightness/Contrast/Gamma image filter.
 * 
 * Copyright 2005 by Renchi Raju, Gilles Caulier
 * Copyright 2006-2007 by Gilles Caulier
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

#define CLAMP_0_255(x)   QMAX(QMIN(x, 255), 0)
#define CLAMP_0_65535(x) QMAX(QMIN(x, 65535), 0)

// C++ includes.

#include <cstdio>
#include <cmath>

// Local includes.

#include "dimg.h"
#include "bcgmodifier.h"

namespace Digikam
{

class BCGModifierPriv
{
public:

    BCGModifierPriv()
    {
        modified       = false;
        overIndicator  = false;
        underIndicator = false;
    }

    bool overIndicator;
    bool underIndicator;
    bool modified;
    
    int  map16[65536];
    int  map[256];
};

BCGModifier::BCGModifier()
{
    d = new BCGModifierPriv;    
    reset();
}

BCGModifier::~BCGModifier()
{
    delete d;
}

bool BCGModifier::modified() const
{
    return d->modified;
}

void BCGModifier::setOverIndicator(bool overIndicator)
{
    d->overIndicator = overIndicator;
}

void BCGModifier::setUnderIndicator(bool underIndicator)
{
    d->underIndicator = underIndicator;
}
    
void BCGModifier::reset()
{
    // initialize to linear mapping

    for (int i=0; i<65536; i++)
        d->map16[i] = i;

    for (int i=0; i<256; i++)
        d->map[i] = i;

    d->modified       = false;
    d->overIndicator  = false;
    d->underIndicator = false;
}

void BCGModifier::applyBCG(DImg& image)
{
    /*
       What is the idea with setting the values when overIndicator is true?
       When the correct value is beyond the upper limit,
       we set the value to be negative.
       When the next function is called (setGamma, setBrightness, setContrast),
       it has the opportunity to correct the excess, or the value
       will again be set to its negative.
       When the correction arrays are applied, all colors with 
       negative values in the arrays will be to black.
     */

    if (!d->modified || image.isNull())
        return;

    uint size = image.width()*image.height();

    if (!image.sixteenBit())                    // 8 bits image.
    {
        uchar* data = (uchar*) image.bits();

        for (uint i=0; i<size; i++)
        {
            if (d->overIndicator && (d->map[data[0]] > 255 || 
                d->map[data[1]] > 255 || d->map[data[2]] > 255))
            {
                data[0] = 0;
                data[1] = 0;
                data[2] = 0;
            }
            else if (d->underIndicator && (d->map[data[0]] < 0 || 
                d->map[data[1]] < 0 || d->map[data[2]] < 0))
            {
                data[0] = 255;
                data[1] = 255;
                data[2] = 255;
            }
            else
            {
                data[0] = CLAMP_0_255(d->map[data[0]]);
                data[1] = CLAMP_0_255(d->map[data[1]]);
                data[2] = CLAMP_0_255(d->map[data[2]]);
            }

            data += 4;
        }
    }
    else                                        // 16 bits image.
    {
        ushort* data = (ushort*) image.bits();

        for (uint i=0; i<size; i++)
        {
            if (d->overIndicator && (d->map16[data[0]] > 65535 || 
                d->map16[data[1]] > 65535 || d->map16[data[2]] > 65535))
            {
                data[0] = 0;
                data[1] = 0;
                data[2] = 0;
            }
            else if (d->underIndicator && (d->map16[data[0]] < 0 || 
                d->map16[data[1]] < 0 || d->map16[data[2]] < 0))
            {
                data[0] = 65535;
                data[1] = 65535;
                data[2] = 65535;
            }
            else
            {            
                data[0] = CLAMP_0_65535(d->map16[data[0]]);
                data[1] = CLAMP_0_65535(d->map16[data[1]]);
                data[2] = CLAMP_0_65535(d->map16[data[2]]);
            }

            data += 4;
        }
    }
}

void BCGModifier::setGamma(double val)
{
    val = (val < 0.01) ? 0.01 : val;

    for (int i=0; i<65536; i++)
        d->map16[i] = lround(pow(((double)d->map16[i] / 65535.0), (1.0 / val)) * 65535.0);

    for (int i=0; i<256; i++)
        d->map[i] = lround(pow(((double)d->map[i] / 255.0), (1.0 / val)) * 255.0);
    
    d->modified = true;
}

void BCGModifier::setBrightness(double val)
{
    int val1 = lround(val * 65535);

    for (int i = 0; i < 65536; i++)
        d->map16[i] = d->map16[i] + val1;

    val1 = lround(val * 255);
    
    for (int i = 0; i < 256; i++)
        d->map[i] = d->map[i] + val1;
    
    d->modified = true;
}

void BCGModifier::setContrast(double val)
{
    for (int i = 0; i < 65536; i++)
        d->map16[i] = lround((d->map16[i] - 32767) * val) + 32767;

    for (int i = 0; i < 256; i++)
        d->map[i] = lround((d->map[i] - 127) * val) + 127;
    
    d->modified = true;
}

}  // NameSpace Digikam
