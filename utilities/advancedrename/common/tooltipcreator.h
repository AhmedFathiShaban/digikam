/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a class to build the tooltip for a renameparser and its options
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef TOOLTIPCREATOR_H
#define TOOLTIPCREATOR_H

// Qt includes

#include <QString>

namespace Digikam
{

class Parser;

class TooltipCreator
{
public:

    TooltipCreator(Parser* parser);
    virtual ~TooltipCreator();

    QString tooltip();

private:

    // common methods
    QString markOption(const QString& str);
    QString tableStart();
    QString tableEnd();


    // parse object related methods
    template <class T> QString createEntries(const QList<T*> &data);
    template <class T> QString createSection(const QString& sectionName, const QList<T*> &data);
                       QString createHeader(const QString& str);

private:

    Parser* const parser;
};

} // namespace Digikam

#endif /* TOOLTIPCREATOR_H */
