/* ============================================================
 * 
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-18
 * Description : class for determining new file name in terms of version management
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

// Qt includes

#include <QFileInfo>
#include <QDir>
#include <QFile>

// KDE includes

#include <KDiskFreeSpaceInfo>
#include <KDebug>

// Local includes

#include "versionmanager.h"

namespace Digikam
{

QString VersionManager::getVersionedFilename(const QString& originalPath, const QString& originalName, 
                                             qint64 fileSize, const QString& formatForRAW,
                                             QString& formatForSubversions,
                                             bool editingOriginal, bool fork, bool editingRAW)
{
    kDebug() << "Original path: " << originalPath << " | Original name: " << originalName
             << " | Editing original: " << editingOriginal << " | Subversion: " << fork;

    //check if we have enough free space for another file
    KDiskFreeSpaceInfo diskInfo = KDiskFreeSpaceInfo::freeSpaceInfo(originalPath);

    if(diskInfo.isValid() && diskInfo.available() > (uint)fileSize * 2)
    {
        //DMetadata metadata(originalPath + "/" + originalName);
        QString newFileName;

        if(!editingOriginal && !fork)
        {
            kDebug() << "Subversion image, will use this version";
            kDebug() << originalName;
            return originalName;
        }
        else
        {
            //the user is editing the original image itself, determine last saved version number and create a new one
            kDebug() << "Original image, will create a new version";
            QFileInfo fileInfo(originalPath + QString("/") + originalName);
            QDir dirInfo(originalPath);

            formatForSubversions = formatForSubversions.toLower();

            // To find the right number for the new version, go through all the items in the given dir, 
            // the version number won't be bigger than count()
            for(uint i = 1; i < dirInfo.count(); i++)
            {
                newFileName.clear();
                newFileName.append(fileInfo.completeBaseName());
                if(fork && !editingOriginal)
                    newFileName.append("v");
                else newFileName.append("_v");
                newFileName.append(QString::number(i));
                newFileName.append(".");
                if(editingRAW)
                {
                    newFileName.append(formatForRAW);
                }
                else
                {
                    newFileName.append(formatForSubversions);
                }
                kDebug() << newFileName;
                QFile newFile(originalPath + QString("/") + newFileName);
                if(!newFile.exists()) 
                {
                    kDebug() << newFileName;
                    return newFileName;
                }
            }
        }
    }

    // FIXME: if space is not enough, do not return a null string there.
    return QString();
}

VersionManager* VersionManager::m_instance = 0;

VersionManager* VersionManager::instance()
{
    if (!m_instance)
        m_instance = new VersionManager;

    return m_instance;
}

} // namespace Digikam
