/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-12-09
 * Description : a tool to print images
 *
 * Copyright (C) 2002-2003 by Todd Shoemaker <todd at theshoemakers dot net>
 * Copyright (C) 2007-2012 by Angelo Naselli <anaselli at linux dot it>
 * Copyright (C) 2006-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef ADV_PRINT_PHOTO_H
#define ADV_PRINT_PHOTO_H

// Qt includes

#include <QRect>
#include <QFont>
#include <QColor>
#include <QUrl>
#include <QPointer>

// Local includes

#include "dmetadata.h"
#include "dinfointerface.h"

namespace Digikam
{

class AdvPrintAdditionalInfo
{
public:

    int    mUnit;
    int    mPrintPosition;
    int    mScaleMode;
    bool   mKeepRatio;
    bool   mAutoRotate;
    double mPrintWidth, mPrintHeight;
    bool   mEnlargeSmallerImages;

public:

    AdvPrintAdditionalInfo()
        : mUnit(0),
          mPrintPosition(0),
          mScaleMode(0),
          mKeepRatio(true),
          mAutoRotate(true),
          mPrintWidth(0.0),
          mPrintHeight(0.0),
          mEnlargeSmallerImages(false)
    {
    }

    AdvPrintAdditionalInfo(const AdvPrintAdditionalInfo& ai)
    {
        mUnit                 = ai.mUnit;
        mPrintPosition        = ai.mPrintPosition;
        mScaleMode            = ai.mScaleMode;
        mKeepRatio            = ai.mKeepRatio;
        mAutoRotate           = ai.mAutoRotate;
        mPrintWidth           = ai.mPrintWidth;
        mPrintHeight          = ai.mPrintHeight;
        mEnlargeSmallerImages = ai.mEnlargeSmallerImages;
    }
};

// -----------------------------------------------------------

class AdvPrintCaptionInfo
{
public:

    enum AvailableCaptions
    {
        NoCaptions = 0,
        FileNames,
        ExifDateTime,
        Comment,
        Free
    };

public:

    AvailableCaptions m_caption_type;
    QFont             m_caption_font;
    QColor            m_caption_color;
    int               m_caption_size;
    QString           m_caption_text;

public:

    AdvPrintCaptionInfo()
        : m_caption_type(NoCaptions),
          m_caption_font(QLatin1String("Sans Serif")),
          m_caption_color(Qt::yellow),
          m_caption_size(2),
          m_caption_text(QLatin1String(""))
    {
    }

    AdvPrintCaptionInfo(const AdvPrintCaptionInfo& ci)
    {
        m_caption_type  = ci.m_caption_type;
        m_caption_font  = ci.m_caption_font;
        m_caption_color = ci.m_caption_color;
        m_caption_size  = ci.m_caption_size;
        m_caption_text  = ci.m_caption_text;
    }

    virtual ~AdvPrintCaptionInfo()
    {
    }
};

// -----------------------------------------------------------

class AdvPrintPhoto
{

public:

    explicit AdvPrintPhoto(int thumbnailSize, DInfoInterface* const iface);
    AdvPrintPhoto(const AdvPrintPhoto&);
    ~AdvPrintPhoto();

    QPixmap&   thumbnail();
    QImage     loadPhoto();
    int        width();
    int        height();
    QSize&     size();
    DMetadata& metaIface();

    double scaleWidth(double unitToInches);
    double scaleHeight(double unitToInches);

public:

    // full path
    QUrl                    m_filename;

    int                     m_thumbnailSize;

    QRect                   m_cropRegion;

    // to get first copy quickly
    bool                    m_first;

    // number of copies
    int                     m_copies;

    int                     m_rotation;
    AdvPrintAdditionalInfo* m_pAddInfo;
    AdvPrintCaptionInfo*    m_pAdvPrintCaptionInfo;
    DInfoInterface*         m_iface;

private:

    void loadCache();

private:

    QPixmap*  m_thumbnail;
    QSize*    m_size;
    DMetadata m_meta;
};

} // Namespace Digikam

#endif // ADV_PRINT_PHOTO_H
