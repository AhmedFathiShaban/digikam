/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2006-01-20
 * Description : main image editor GUI implementation
 *
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

#include <qlabel.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qtimer.h>
#include <qfileinfo.h>

// KDE includes.

#include <kprinter.h>
#include <kkeydialog.h>
#include <kdeversion.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <kedittoolbar.h>
#include <kaboutdata.h>
#include <kcursor.h>
#include <kstdaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmenubar.h>
#include <kimageio.h>
#include <kaccel.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <kprotocolinfo.h>
#include <kglobalsettings.h>
#include <ktoolbar.h>
#include <kstatusbar.h>
#include <kprogress.h>

// Local includes.

#include "canvas.h"
#include "dimginterface.h"
#include "imageplugin.h"
#include "imagepluginloader.h"
#include "imageresizedlg.h"
#include "imageprint.h"
#include "iofileprogressbar.h"
#include "iccsettingscontainer.h"
#include "iofilesettingscontainer.h"
#include "savingcontextcontainer.h"
#include "editorwindowprivate.h"
#include "editorwindow.h"

namespace Digikam
{

EditorWindow::EditorWindow(const char *name)
            : KMainWindow(0, name, WType_TopLevel|WDestructiveClose)
{
    d = new EditorWindowPriv;

    m_canvas           = 0;
    m_undoAction       = 0;
    m_redoAction       = 0;
    m_zoomPlusAction   = 0;
    m_zoomMinusAction  = 0;
    m_zoomFitAction    = 0;
    m_fullScreenAction = 0;
    m_saveAction       = 0;
    m_saveAsAction     = 0;
    m_revertAction     = 0;
    m_filePrintAction  = 0;    
    m_fileDeleteAction = 0;
    m_forwardAction    = 0;
    m_backwardAction   = 0;
    m_firstAction      = 0;
    m_lastAction       = 0;
    m_fullScreen       = false;
    m_isReadOnly       = false;

    // Settings containers instance.

    m_ICCSettings     = new ICCSettingsContainer();
    m_IOFileSettings  = new IOFileSettingsContainer();
    m_savingContext   = new SavingContextContainer();
}

EditorWindow::~EditorWindow()
{
    delete m_canvas;
    delete m_ICCSettings;
    delete m_IOFileSettings;
    delete m_savingContext;
    delete d;
}

void EditorWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
        return;

    if (!promptUserSave())
        return;

    saveSettings();
    e->accept();
}

void EditorWindow::setupStandardActions()
{
    // Standard 'File' menu actions.

    m_backwardAction = KStdAction::back(this, SLOT(slotBackward()),
                                    actionCollection(), "editorwindow_backward");

    m_forwardAction = KStdAction::forward(this, SLOT(slotForward()),
                                          actionCollection(), "editorwindow_forward");

    m_firstAction = new KAction(i18n("&First"), "start",
                                KStdAccel::shortcut( KStdAccel::Home),
                                this, SLOT(slotFirst()),
                                actionCollection(), "editorwindow_first");

    m_lastAction = new KAction(i18n("&Last"), "finish",
                               KStdAccel::shortcut( KStdAccel::End),
                               this, SLOT(slotLast()),
                               actionCollection(), "editorwindow_last");

    m_saveAction   = KStdAction::save(this, SLOT(slotSave()),
                                      actionCollection(), "editorwindow_save");

    m_saveAsAction = KStdAction::saveAs(this, SLOT(slotSaveAs()),
                                        actionCollection(), "editorwindow_saveas");

    m_revertAction = KStdAction::revert(m_canvas, SLOT(slotRestore()),
                                        actionCollection(), "editorwindow_revert");

    m_saveAction->setEnabled(false);
    m_saveAsAction->setEnabled(false);
    m_revertAction->setEnabled(false);

    m_filePrintAction = new KAction(i18n("Print Image..."), "fileprint",
                                    CTRL+Key_P,
                                    this, SLOT(slotFilePrint()),
                                    actionCollection(), "editorwindow_print");

    m_fileDeleteAction = new KAction(i18n("Delete File"), "editdelete",
                                     SHIFT+Key_Delete,
                                     this, SLOT(slotDeleteCurrentItem()),
                                     actionCollection(), "editorwindow_delete");

    KStdAction::quit(this, SLOT(close()), actionCollection(), "editorwindow_exit");
}

void EditorWindow::setupStatusBar()
{
    m_nameLabel = new IOFileProgressBar(statusBar());
    m_nameLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(m_nameLabel,1);
    m_zoomLabel = new QLabel(statusBar());
    m_zoomLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(m_zoomLabel,1);
    m_resLabel  = new QLabel(statusBar());
    m_resLabel->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(m_resLabel,1);
}

void EditorWindow::printImage(KURL url)
{
    uchar* ptr      = DImgInterface::instance()->getImage();
    int w           = DImgInterface::instance()->origWidth();
    int h           = DImgInterface::instance()->origHeight();
    bool hasAlpha   = DImgInterface::instance()->hasAlpha();
    bool sixteenBit = DImgInterface::instance()->sixteenBit();

    if (!ptr || !w || !h)
        return;

    DImg image(w, h, sixteenBit, hasAlpha, ptr);

    KPrinter printer;
    QString appName = KApplication::kApplication()->aboutData()->appName();
    printer.setDocName( url.filename() );
    printer.setCreator( appName );
#if KDE_IS_VERSION(3,2,0)
    printer.setUsePrinterResolution(true);
#endif

    KPrinter::addDialogPage( new ImageEditorPrintDialogPage( this, (appName.append(" page")).ascii() ));

    if ( printer.setup( this, i18n("Print %1").arg(printer.docName().section('/', -1)) ) )
    {
        ImagePrint printOperations(image, printer, url.filename());
        if (!printOperations.printImageWithQt())
        {
            KMessageBox::error(this, i18n("Failed to print file: '%1'")
                               .arg(url.filename()));
        }
    }
}

void EditorWindow::slotImagePluginsHelp()
{
    KApplication::kApplication()->invokeHelp( QString::null, "digikamimageplugins" );
}

void EditorWindow::slotEditKeys()
{
    KKeyDialog dialog(true, this);
    dialog.insert( actionCollection(), i18n( "General" ) );
    ImagePluginLoader* loader = ImagePluginLoader::instance();
    
    for (ImagePlugin* plugin = loader->pluginList().first();
         plugin; plugin = loader->pluginList().next())
    {
        if (plugin)
        {
            dialog.insert( plugin->actionCollection(), plugin->name() );
        }
    }
    
    dialog.configure();
}

void EditorWindow::slotResize()
{
    int width  = m_canvas->imageWidth();
    int height = m_canvas->imageHeight();

    Digikam::ImageResizeDlg dlg(this, &width, &height);
    
    if (dlg.exec() == QDialog::Accepted &&
        (width != m_canvas->imageWidth() ||
         height != m_canvas->imageHeight()))
        m_canvas->resizeImage(width, height);
}

void EditorWindow::slotAboutToShowUndoMenu()
{
    m_undoAction->popupMenu()->clear();
    QStringList titles;
    m_canvas->getUndoHistory(titles);
    
    if(!titles.isEmpty())
    {
        int id = 1;
        QStringList::Iterator iter = titles.begin();        
        for(; iter != titles.end(); ++iter,++id)
        {
            m_undoAction->popupMenu()->insertItem(*iter, id);
        }        
    }
}

void EditorWindow::slotAboutToShowRedoMenu()
{
    m_redoAction->popupMenu()->clear();
    QStringList titles;
    m_canvas->getRedoHistory(titles);
    
    if(!titles.isEmpty())
    {
        int id = 1;
        QStringList::Iterator iter = titles.begin();        
        for(; iter != titles.end(); ++iter,++id)
        {
            m_redoAction->popupMenu()->insertItem(*iter, id);
        }        
    }
}

void EditorWindow::slotConfToolbars()
{
    saveMainWindowSettings(KGlobal::config(), "ImageViewer Settings");
    KEditToolbar dlg(factory(), this);
    connect(&dlg, SIGNAL(newToolbarConfig()),
            SLOT(slotNewToolbarConfig()));
    dlg.exec();
}

void EditorWindow::slotNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config(), "ImageViewer Settings");
}

void EditorWindow::slotToggleAutoZoom()
{
    bool checked = m_zoomFitAction->isChecked();

    m_zoomPlusAction->setEnabled(!checked);
    m_zoomMinusAction->setEnabled(!checked);

    m_canvas->slotToggleAutoZoom();
}

void EditorWindow::slotZoomChanged(float zoom)
{
    m_zoomLabel->setText(i18n("Zoom: ") +
                         QString::number(zoom*100, 'f', 2) +
                         QString("%"));

    m_zoomPlusAction->setEnabled(!m_canvas->maxZoom() &&
                                 !m_zoomFitAction->isChecked());
    m_zoomMinusAction->setEnabled(!m_canvas->minZoom() &&
                                  !m_zoomFitAction->isChecked());
}

void EditorWindow::slotEscapePressed()
{
    if (m_fullScreen)
        m_fullScreenAction->activate();
}

}  // namespace Digikam

#include "editorwindow.moc"
