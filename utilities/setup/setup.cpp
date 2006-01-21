/* ============================================================
 * Authors: Renchi Raju <renchi at pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at free.fr>
 * Date   : 2003-02-03
 * Description : digiKam setup dialog.
 * 
 * Copyright 2003-2005 by Renchi Raju and Gilles Caulier
 * Copyright 2006 by Gilles Caulier
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
 
// Qt includes.

#include <qtabwidget.h>
#include <qapplication.h>
#include <qframe.h>

// KDE includes.

#include <klocale.h>
#include <kiconloader.h>

// Local includes.

#include "setupgeneral.h"
#include "setupexif.h"
#include "setupcollections.h"
#include "setupmime.h"
#include "setupeditor.h"
#include "setupimgplugins.h"
#include "setupicc.h"
#include "setupplugins.h"
#include "setupcamera.h"
#include "setupmisc.h"
#include "setup.h"

namespace Digikam
{

class SetupPrivate
{
public:

    SetupPrivate()
    {
        page_general     = 0;
        page_exif        = 0;
        page_collections = 0;
        page_mime        = 0;
        page_editor      = 0;
        page_imgPlugins  = 0;
        page_icc         = 0;
        page_plugins     = 0;
        page_camera      = 0;
        page_misc        = 0;
        generalPage      = 0;
        exifPage         = 0;
        collectionsPage  = 0;
        mimePage         = 0;
        editorPage       = 0;
        iccPage          = 0;
        cameraPage       = 0;
        miscPage         = 0;
    }

    QFrame           *page_general;
    QFrame           *page_exif;
    QFrame           *page_collections;
    QFrame           *page_mime;
    QFrame           *page_editor;
    QFrame           *page_imgPlugins;
    QFrame           *page_icc;
    QFrame           *page_plugins;
    QFrame           *page_camera;
    QFrame           *page_misc;

    SetupGeneral     *generalPage;
    SetupExif        *exifPage;
    SetupCollections *collectionsPage;
    SetupMime        *mimePage;
    SetupEditor      *editorPage;
    SetupICC         *iccPage;
    SetupCamera      *cameraPage;
    SetupMisc        *miscPage;
};

Setup::Setup(QWidget* parent, const char* name, Setup::Page page)
     : KDialogBase(IconList, i18n("Configure"), Help|Ok|Cancel, Ok, parent,
                   name, true, true )
{
    d = new SetupPrivate;
    setHelp("setupdialog.anchor", "digikam");

    d->page_general = addPage(i18n("Albums"), i18n("Albums"),
                           BarIcon("folder_image", KIcon::SizeMedium));
    d->generalPage = new SetupGeneral(d->page_general, this);

    d->page_exif = addPage(i18n("Embedded Info"), i18n("Embedded Image Information"),
                        BarIcon("exifinfo", KIcon::SizeMedium));
    d->exifPage = new SetupExif(d->page_exif);

    d->page_collections = addPage(i18n("Collections"), i18n("Album Collections"),
                               BarIcon("fileopen", KIcon::SizeMedium));
    d->collectionsPage = new SetupCollections(d->page_collections);

    d->page_mime = addPage(i18n("Mime Types"), i18n("File (MIME) Types"),
                        BarIcon("filetypes", KIcon::SizeMedium));
    d->mimePage = new SetupMime(d->page_mime);

    d->page_editor = addPage(i18n("Image Editor"), i18n("Image Editor Settings"),
                          BarIcon("image", KIcon::SizeMedium));
    d->editorPage = new SetupEditor(d->page_editor);

    d->page_imgPlugins = addPage(i18n("Image Plugins"), i18n("Image Editor Plugins"),
                          BarIcon("digikamimageplugins", KIcon::SizeMedium));
    m_imgPluginsPage = new SetupImgPlugins(d->page_imgPlugins);

    d->page_icc = addPage(i18n("ICC Profiles"), i18n("ICC Profiles"),
                       BarIcon("colorize", KIcon::SizeMedium));
    d->iccPage = new SetupICC(d->page_icc);

    d->page_plugins = addPage(i18n("Kipi Plugins"), i18n("Main Interface Plugins"),
                           BarIcon("kipi", KIcon::SizeMedium));
    m_pluginsPage = new SetupPlugins(d->page_plugins);

    d->page_camera = addPage(i18n("Cameras"), i18n("Cameras"),
                          BarIcon("digitalcam", KIcon::SizeMedium));
    d->cameraPage = new SetupCamera(d->page_camera);

    d->page_misc   = addPage(i18n("Miscellaneous"), i18n("Miscellaneous"),
                          BarIcon("misc", KIcon::SizeMedium));
    d->miscPage = new SetupMisc(d->page_misc);

    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOkClicked()) );

    showPage((int) page);
    show();
}

Setup::~Setup()
{
    delete d;
}

void Setup::slotOkClicked()
{
    d->generalPage->applySettings();
    d->collectionsPage->applySettings();
    d->mimePage->applySettings();
    d->cameraPage->applySettings();
    d->exifPage->applySettings();
    d->editorPage->applySettings();
    m_imgPluginsPage->applySettings();
    d->iccPage->applySettings();
    d->miscPage->applySettings();
    close();
}

}  // namespace Digikam

#include "setup.moc"
