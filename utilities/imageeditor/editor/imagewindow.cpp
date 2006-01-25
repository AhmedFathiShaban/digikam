/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-02-12
 * Description : digiKam image editor GUI
 *
 * Copyright 2004-2005 by Renchi Raju, Gilles Caulier
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

// C++ Includes.

#include <cstdio>

// Qt includes.

#include <qpopupmenu.h>
#include <qcursor.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qimage.h>
#include <qsplitter.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kimageio.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kmenubar.h>
#include <ktoolbar.h>
#include <kaccel.h>
#include <kaction.h>
#include <kstdaccel.h>
#include <kstdaction.h>
#include <kstdguiitem.h>
#include <kstatusbar.h>
#include <kpopupmenu.h>
#include <kprogress.h>

// LibKexif includes.

#include <libkexif/kexifdata.h>
#include <libkexif/kexifutils.h>

// Local includes.

#include "canvas.h"
#include "dimginterface.h"
#include "imageplugin.h"
#include "imagepluginloader.h"
#include "imageresizedlg.h"
#include "imageprint.h"
#include "albummanager.h"
#include "album.h"
#include "albumdb.h"
#include "albumsettings.h"
#include "syncjob.h"
#include "albumiconview.h"
#include "albumiconitem.h"
#include "imageinfo.h"
#include "imagepropertiessidebardb.h"
#include "tagspopupmenu.h"
#include "iofileprogressbar.h"
#include "iccsettingscontainer.h"
#include "iofilesettingscontainer.h"
#include "savingcontextcontainer.h"
#include "imagewindow.h"

namespace Digikam
{

ImageWindow::ImageWindow()
           : EditorWindow( "Image Editor" )
{
    m_instance              = this;
    m_rotatedOrFlipped      = false;
    m_allowSaving           = true;
    m_fullScreenHideToolBar = false;
    m_view                  = 0L;

    // Construct the view

    setupUserArea();
    setupStatusBar();

    // Build the gui

    setupActions();
        
    QPtrList<ImagePlugin> pluginList = ImagePluginLoader::instance()->pluginList();

    for (ImagePlugin* plugin = pluginList.first();
         plugin; plugin = pluginList.next())
    {
        if (plugin) 
        {
            guiFactory()->addClient(plugin);
            plugin->setParentWidget(this);
            plugin->setEnabledSelectionActions(false);
        }
    }

    m_contextMenu = dynamic_cast<QPopupMenu*>(factory()->container("RMBMenu", this));
    
    // -- Some Accels not available from actions -------------

    m_accel = new KAccel(this);
    m_accel->insert("Exit fullscreen", i18n("Exit Fullscreen"),
                    i18n("Exit out of the fullscreen mode"),
                    Key_Escape, this, SLOT(slotEscapePressed()),
                    false, true);

    // -- setup connections ---------------------------
           
    connect(m_canvas, SIGNAL(signalRightButtonClicked()),
            this, SLOT(slotContextMenu()));
            
    connect(m_canvas, SIGNAL(signalZoomChanged(float)),
            this, SLOT(slotZoomChanged(float)));
            
    connect(m_canvas, SIGNAL(signalSelected(bool)),
            this, SLOT(slotSelected(bool)));
            
    connect(m_canvas, SIGNAL(signalChanged(bool, bool)),
            this, SLOT(slotChanged(bool, bool)));
            
    connect(m_canvas, SIGNAL(signalShowNextImage()),
            this, SLOT(slotForward()));
            
    connect(m_canvas, SIGNAL(signalShowPrevImage()),
            this, SLOT(slotBackward()));

    connect(m_canvas, SIGNAL(signalLoadingStarted(const QString &)),
            this, SLOT(slotLoadingStarted(const QString &)));

    connect(m_canvas, SIGNAL(signalLoadingFinished(const QString &, bool, bool)),
            this, SLOT(slotLoadingFinished(const QString &, bool, bool)));

    connect(m_canvas, SIGNAL(signalLoadingProgress(const QString &, float)),
            this, SLOT(slotLoadingProgress(const QString &, float)));

    connect(m_canvas, SIGNAL(signalSavingStarted(const QString &)),
            this, SLOT(slotSavingStarted(const QString &)));

    connect(m_canvas, SIGNAL(signalSavingFinished(const QString &, bool)),
            this, SLOT(slotSavingFinished(const QString &, bool)));

    connect(m_canvas, SIGNAL(signalSavingProgress(const QString&, float)),
            this, SLOT(slotSavingProgress(const QString&, float)));

    connect(m_rightSidebar, SIGNAL(signalNextItem()),
            this, SLOT(slotForward()));
                
    connect(m_rightSidebar, SIGNAL(signalPrevItem()),
            this, SLOT(slotBackward()));
    
    // -- read settings --------------------------------
    
    readSettings();
    applySettings();

    // This is just a bloody workaround until we have found the problem
    // which leads the imagewindow to open in a wrong size
    resize(640, 480);
    
    setAutoSaveSettings("ImageViewer Settings");    
    m_rightSidebar->populateTags();
}

ImageWindow::~ImageWindow()
{
    m_instance = 0;

    QPtrList<ImagePlugin> pluginList
        = ImagePluginLoader::instance()->pluginList();
    for (ImagePlugin* plugin = pluginList.first();
         plugin; plugin = pluginList.next())
    {
        if (plugin) 
        {
            guiFactory()->removeClient(plugin);
            plugin->setParentWidget(0);
            plugin->setEnabledSelectionActions(false);
        }
    }
    
    delete m_rightSidebar;
}

void ImageWindow::setupUserArea()
{
    QWidget* widget  = new QWidget(this);
    QHBoxLayout *lay = new QHBoxLayout(widget);
    
    m_splitter       = new QSplitter(widget);
    m_canvas         = new Canvas(m_splitter);
    m_rightSidebar   = new ImagePropertiesSideBarDB(widget, m_splitter,
                                                    Sidebar::Right, true, false);
    
    lay->addWidget(m_splitter);
    lay->addWidget(m_rightSidebar);
    
    m_splitter->setOpaqueResize(false);
    setCentralWidget(widget);
}

void ImageWindow::setupActions()
{
    setupStandardActions();

    // -- Edit actions ----------------------------------------------------------------                     

    m_copyAction = KStdAction::copy(m_canvas, SLOT(slotCopy()),
                                    actionCollection(), "imageview_copy");
    m_copyAction->setEnabled(false);

    m_undoAction = new KToolBarPopupAction(i18n("Undo"), "undo", 
                                           KStdAccel::shortcut(KStdAccel::Undo),
                                           m_canvas, SLOT(slotUndo()),
                                           actionCollection(), "imageview_undo");
    connect(m_undoAction->popupMenu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowUndoMenu()));
    connect(m_undoAction->popupMenu(), SIGNAL(activated(int)),
            m_canvas, SLOT(slotUndo(int)));
    m_undoAction->setEnabled(false);

    m_redoAction = new KToolBarPopupAction(i18n("Redo"), "redo", 
                                           KStdAccel::shortcut(KStdAccel::Redo),
                                           m_canvas, SLOT(slotRedo()),
                                           actionCollection(), "imageview_redo");
    connect(m_redoAction->popupMenu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowRedoMenu()));
    connect(m_redoAction->popupMenu(), SIGNAL(activated(int)),
            m_canvas, SLOT(slotRedo(int)));
    m_redoAction->setEnabled(false);
           
    // -- View actions ----------------------------------------------------------------
    
    m_zoomPlusAction = new KAction(i18n("Zoom &In"), "viewmag+",
                                   CTRL+Key_Plus,
                                   m_canvas, SLOT(slotIncreaseZoom()),
                                   actionCollection(), "imageview_zoom_plus");

    m_zoomMinusAction = new KAction(i18n("Zoom &Out"), "viewmag-",
                                    CTRL+Key_Minus,
                                    m_canvas, SLOT(slotDecreaseZoom()),
                                    actionCollection(), "imageview_zoom_minus");

    m_zoomFitAction = new KToggleAction(i18n("Zoom &AutoFit"), "viewmagfit",
                                        Key_A,
                                        this, SLOT(slotToggleAutoZoom()),
                                        actionCollection(), "imageview_zoom_fit");

    m_fullScreenAction = new KToggleAction(i18n("Toggle Full Screen"),
                                           "window_fullscreen",
                                           CTRL+SHIFT+Key_F,
                                           this, SLOT(slotToggleFullScreen()),
                                           actionCollection(), "toggle_fullScreen");

    m_viewHistogramAction = new KSelectAction(i18n("&Histogram"), "histogram",
                                              Key_H,
                                              this, SLOT(slotViewHistogram()),
                                              actionCollection(), "imageview_histogram");
    m_viewHistogramAction->setEditable(false);

    QStringList selectItems;
    selectItems << i18n("Hide");
    selectItems << i18n("Luminosity");
    selectItems << i18n("Red");
    selectItems << i18n("Green");
    selectItems << i18n("Blue");
    selectItems << i18n("Alpha");
    m_viewHistogramAction->setItems(selectItems);

    // -- Transform actions ----------------------------------------------------------
    
    m_resizeAction = new KAction(i18n("&Resize..."), "resize_image", 0,
                                 this, SLOT(slotResize()),
                                 actionCollection(), "imageview_resize");

    m_cropAction = new KAction(i18n("&Crop"), "crop",
                               CTRL+Key_X,
                               m_canvas, SLOT(slotCrop()),
                               actionCollection(), "imageview_crop");
    m_cropAction->setEnabled(false);
    m_cropAction->setWhatsThis( i18n("This option can be used to crop the image. "
                                     "Select the image region to enable this action.") );

    // -- rotate actions -------------------------------------------------------------
    
    m_rotateAction = new KActionMenu(i18n("&Rotate"), "rotate_cw",
                                     actionCollection(),
                                     "imageview_rotate");
    m_rotateAction->setDelayed(false);

    m_rotate90Action  = new KAction(i18n("90 Degrees"),
                                    0, Key_1, m_canvas, SLOT(slotRotate90()),
                                    actionCollection(),
                                    "rotate_90");
    m_rotate180Action = new KAction(i18n("180 Degrees"),
                                    0, Key_2, m_canvas, SLOT(slotRotate180()),
                                    actionCollection(),
                                    "rotate_180");
    m_rotate270Action = new KAction(i18n("270 Degrees"),
                                    0, Key_3, m_canvas, SLOT(slotRotate270()),
                                    actionCollection(),
                                    "rotate_270");
    m_rotateAction->insert(m_rotate90Action);
    m_rotateAction->insert(m_rotate180Action);
    m_rotateAction->insert(m_rotate270Action);

    // -- flip actions ---------------------------------------------------------------
    
    m_flipAction = new KActionMenu(i18n("Flip"),
                                   "flip",
                                   actionCollection(),
                                   "imageview_flip");
    m_flipAction->setDelayed(false);

    m_flipHorzAction = new KAction(i18n("Horizontally"), 0,
                                   m_canvas, SLOT(slotFlipHoriz()),
                                   actionCollection(),
                                   "flip_horizontal"); 

    m_flipVertAction = new KAction(i18n("Vertically"), 0,
                                   m_canvas, SLOT(slotFlipVert()),
                                   actionCollection(),
                                   "flip_vertical");
    m_flipAction->insert(m_flipHorzAction);
    m_flipAction->insert(m_flipVertAction);

    // -- help actions ---------------------------------------------------------------
    
    m_imagePluginsHelp = new KAction(i18n("Image Plugins Handbooks"), 
                                     "digikamimageplugins", 0, 
                                     this, SLOT(slotImagePluginsHelp()),
                                     actionCollection(), "imageview_imagepluginshelp");

    // -- Configure toolbar and shortcuts ---------------------------------------------
    
    KStdAction::keyBindings(this, SLOT(slotEditKeys()),
                            actionCollection());
    KStdAction::configureToolbars(this, SLOT(slotConfToolbars()),
                                  actionCollection());

    // --- Create the gui --------------------------------------------------------------
    
    createGUI("digikamimagewindowui.rc", false);

    // -- if rotating/flipping set the rotatedflipped flag to true ---------------------

    connect(m_rotate90Action, SIGNAL(activated()),
            this, SLOT(slotRotatedOrFlipped()));
            
    connect(m_rotate180Action, SIGNAL(activated()),
            this, SLOT(slotRotatedOrFlipped()));
            
    connect(m_rotate270Action, SIGNAL(activated()),
            this, SLOT(slotRotatedOrFlipped()));
            
    connect(m_flipHorzAction, SIGNAL(activated()),
            this, SLOT(slotRotatedOrFlipped()));
            
    connect(m_flipVertAction, SIGNAL(activated()),
            this, SLOT(slotRotatedOrFlipped()));
}

void ImageWindow::loadURL(const KURL::List& urlList,
                          const KURL& urlCurrent,
                          const QString& caption,
                          bool  allowSaving,
                          AlbumIconView* view)
{
    if (!promptUserSave())
        return;
    
    setCaption(i18n("digiKam Image Editor - Album \"%1\"").arg(caption));

    m_view        = view;
    m_urlList     = urlList;
    m_urlCurrent  = urlCurrent;
    m_allowSaving = allowSaving;
    
    m_saveAction->setEnabled(false);
    m_revertAction->setEnabled(false);
    m_undoAction->setEnabled(false);
    m_redoAction->setEnabled(false);

    QTimer::singleShot(0, this, SLOT(slotLoadCurrent()));
}

void ImageWindow::applySettings()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");

    // Background color.
    QColor bgColor(Qt::black);
    m_canvas->setBackgroundColor(config->readColorEntry("BackgroundColor", &bgColor));
    m_canvas->update();

    // JPEG quality slider settings : 0 - 100 ==> libjpeg settings : 25 - 100.
    m_IOFileSettings->JPEGCompression  = (int)((75.0/99.0)*(float)config->readNumEntry("JPEGCompression", 75)
                                               + 25.0 - (75.0/99.0));

    // PNG compression slider settings : 1 - 9 ==> libpng settings : 100 - 1.
    m_IOFileSettings->PNGCompression   = (int)(((1.0-100.0)/8.0)*(float)config->readNumEntry("PNGCompression", 1)
                                                 + 100.0 - ((1.0-100.0)/8.0));

    m_IOFileSettings->TIFFCompression  = config->readBoolEntry("TIFFCompression", false);

    m_IOFileSettings->rawDecodingSettings.automaticColorBalance = config->readBoolEntry("AutomaticColorBalance", true);
    m_IOFileSettings->rawDecodingSettings.cameraColorBalance    = config->readBoolEntry("CameraColorBalance", true);
    m_IOFileSettings->rawDecodingSettings.RGBInterpolate4Colors = config->readBoolEntry("RGBInterpolate4Colors", false);
    m_IOFileSettings->rawDecodingSettings.enableRAWQuality      = config->readBoolEntry("EnableRAWQuality", false);
    m_IOFileSettings->rawDecodingSettings.RAWQuality            = config->readNumEntry("RAWQuality", 0);

    AlbumSettings *settings = AlbumSettings::instance();
    if (settings->getUseTrash())
    {
        m_fileDeleteAction->setIcon("edittrash");
        m_fileDeleteAction->setText(i18n("Move to Trash"));
    }
    else
    {
        m_fileDeleteAction->setIcon("editdelete");
        m_fileDeleteAction->setText(i18n("Delete File"));
    }

    m_canvas->setExifOrient(settings->getExifRotate());
    
    QSizePolicy rightSzPolicy(QSizePolicy::Preferred, QSizePolicy::Expanding, 2, 1);
    if(config->hasKey("Splitter Sizes"))
        m_splitter->setSizes(config->readIntListEntry("Splitter Sizes"));
    else 
        m_canvas->setSizePolicy(rightSzPolicy);
}

void ImageWindow::readSettings()
{
    bool autoZoom = false;

    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");    

    // GUI options.
    autoZoom = config->readBoolEntry("AutoZoom", true);
    m_fullScreen = config->readBoolEntry("FullScreen", false);
    m_fullScreenHideToolBar = config->readBoolEntry("FullScreen Hide ToolBar", false);

    if (autoZoom)
    {
        m_zoomFitAction->activate();
        m_zoomPlusAction->setEnabled(false);
        m_zoomMinusAction->setEnabled(false);
    }

    if (m_fullScreen)
    {
        m_fullScreen = false;
        m_fullScreenAction->activate();
    }
    
    QRect histogramRect = config->readRectEntry("Histogram Rectangle");
    if (!histogramRect.isNull())
        m_canvas->setHistogramPosition(histogramRect.topLeft());
    
    int histogramType = config->readNumEntry("HistogramType", 0);
    histogramType = (histogramType < 0 || histogramType > 5) ? 0 : histogramType;
    m_viewHistogramAction->setCurrentItem(histogramType);
    slotViewHistogram(); // update

    // Settings for Color Management stuff
    config->setGroup("Color Management");

    m_ICCSettings->enableCMSetting = config->readBoolEntry("EnableCM");
    m_ICCSettings->askOrApplySetting = config->readBoolEntry("BehaviourICC");
    m_ICCSettings->BPCSetting = config->readBoolEntry("BPCAlgorithm");
    m_ICCSettings->renderingSetting = config->readNumEntry("RenderingIntent");
    m_ICCSettings->inputSetting = config->readPathEntry("InProfileFile");
    m_ICCSettings->workspaceSetting = config->readPathEntry("WorkProfileFile");
    m_ICCSettings->monitorSetting = config->readPathEntry("MonitorProfileFile");
    m_ICCSettings->proofSetting = config->readPathEntry("ProofProfileFile");
}

void ImageWindow::saveSettings()
{
    KConfig* config = kapp->config();
    
    config->setGroup("ImageViewer Settings");
    config->writeEntry("AutoZoom", m_zoomFitAction->isChecked());
    config->writeEntry("Splitter Sizes", m_splitter->sizes());

    int histogramType = m_viewHistogramAction->currentItem();
    histogramType = (histogramType < 0 || histogramType > 5) ? 0 : histogramType;
    config->writeEntry("HistogramType", histogramType);

    config->writeEntry("FullScreen", m_fullScreen);
    
    QPoint pt;
    QRect rc(0, 0, 0, 0);
    if (m_canvas->getHistogramPosition(pt)) 
        rc = QRect(pt.x(), pt.y(), 1, 1);
    config->writeEntry("Histogram Rectangle", rc);
    config->sync();
}

void ImageWindow::slotLoadCurrent()
{
    KURL::List::iterator it = m_urlList.find(m_urlCurrent);
    
    if (m_view)
    {
        IconItem* item = m_view->findItem((*it).url());
        if (item)
           m_view->setCurrentItem(item);
    }

    if (it != m_urlList.end()) 
    {
        //QApplication::setOverrideCursor(Qt::WaitCursor);

        if (m_ICCSettings->enableCMSetting)
        {
            kdDebug() << "enableCMSetting=true" << endl;
            m_canvas->load(m_urlCurrent.path(), m_ICCSettings, m_IOFileSettings);
        }
        else
        {
            kdDebug() << "enableCMSetting=false" << endl;
            m_canvas->load(m_urlCurrent.path(), 0, m_IOFileSettings);
        }
        
        ++it;
        if (it != m_urlList.end())
            m_canvas->preload((*it).path());

        //QApplication::restoreOverrideCursor();
    }

}

void ImageWindow::slotLoadingStarted(const QString &filename)
{
    //TODO: Disable actions as appropriate

    m_nameLabel->progressBarVisible(true);
    QApplication::setOverrideCursor(Qt::WaitCursor);
}

void ImageWindow::slotLoadingFinished(const QString &filename, bool success, bool isReadOnly)
{
    //TODO: enable actions as appropriate
    //TODO: handle success == false

    m_nameLabel->progressBarVisible(false);
    QApplication::restoreOverrideCursor();
    m_isReadOnly = isReadOnly;

    uint index = m_urlList.findIndex(m_urlCurrent);

    m_rotatedOrFlipped = false;

    QString text = m_urlCurrent.filename() +
            i18n(" (%2 of %3)")
            .arg(QString::number(index+1))
            .arg(QString::number(m_urlList.count()));
    m_nameLabel->setText(text);

    if (m_urlList.count() == 1) 
    {
        m_backwardAction->setEnabled(false);
        m_forwardAction->setEnabled(false);
        m_firstAction->setEnabled(false);
        m_lastAction->setEnabled(false);
    }
    else 
    {
        m_backwardAction->setEnabled(true);
        m_forwardAction->setEnabled(true);
        m_firstAction->setEnabled(true);
        m_lastAction->setEnabled(true);
    }

    if (index == 0) 
    {
        m_backwardAction->setEnabled(false);
        m_firstAction->setEnabled(false);
    }

    if (index == m_urlList.count()-1) 
    {
        m_forwardAction->setEnabled(false);
        m_lastAction->setEnabled(false);
    }

    // Disable some menu actions if the current root image URL
    // isn't include in the digiKam Albums library database.
    // This is necessary when ImageEditor is opened from cameraclient.

    KURL u(m_urlCurrent.directory());
    PAlbum *palbum = AlbumManager::instance()->findPAlbum(u);

    if (!palbum)
    {
        m_fileDeleteAction->setEnabled(false);
    }
    else
    {
        m_fileDeleteAction->setEnabled(true);
    }
}

void ImageWindow::slotLoadingProgress(const QString& filePath, float progress)
{
    m_nameLabel->setProgressValue((int)(progress*100.0));
}

void ImageWindow::slotForward()
{
    if(!promptUserSave())
        return;

    KURL::List::iterator it = m_urlList.find(m_urlCurrent);

    if (it != m_urlList.end()) 
    {
        if (m_urlCurrent != m_urlList.last())
        {
           KURL urlNext = *(++it);
           m_urlCurrent = urlNext;
           slotLoadCurrent();
        }
    }
}

void ImageWindow::slotBackward()
{
    if(!promptUserSave())
        return;

    KURL::List::iterator it = m_urlList.find(m_urlCurrent);

    if (it != m_urlList.begin()) 
    {
        if (m_urlCurrent != m_urlList.first())
        {
            KURL urlPrev = *(--it);
            m_urlCurrent = urlPrev;
            slotLoadCurrent();
        }
    }
}

void ImageWindow::slotFirst()
{
    if(!promptUserSave())
        return;
    
    m_urlCurrent = m_urlList.first();
    slotLoadCurrent();
}

void ImageWindow::slotLast()
{
    if(!promptUserSave())
        return;
    
    m_urlCurrent = m_urlList.last();
    slotLoadCurrent();
}

void ImageWindow::slotViewHistogram()
{
    int curItem = m_viewHistogramAction->currentItem();
    m_canvas->setHistogramType(curItem);
}

void ImageWindow::slotContextMenu()
{
    if (m_contextMenu)
    {
        TagsPopupMenu* assignTagsMenu = 0;
        TagsPopupMenu* removeTagsMenu = 0;
        int separatorID = -1;

        if (m_view)
        {
            IconItem* item = m_view->findItem(m_urlCurrent.url());
            if (item)
            {
                Q_LLONG id = ((AlbumIconItem*)item)->imageInfo()->id();
                QValueList<Q_LLONG> idList;
                idList.append(id);

                assignTagsMenu = new TagsPopupMenu(idList, 1000,
                                                   TagsPopupMenu::ASSIGN);
                removeTagsMenu = new TagsPopupMenu(idList, 2000,
                                                   TagsPopupMenu::REMOVE);

                separatorID = m_contextMenu->insertSeparator();

                m_contextMenu->insertItem(i18n("Assign Tag"), assignTagsMenu);
                int i = m_contextMenu->insertItem(i18n("Remove Tag"), removeTagsMenu);

                connect(assignTagsMenu, SIGNAL(signalTagActivated(int)),
                        SLOT(slotAssignTag(int)));
                connect(removeTagsMenu, SIGNAL(signalTagActivated(int)),
                        SLOT(slotRemoveTag(int)));

                AlbumDB* db = AlbumManager::instance()->albumDB();
                if (!db->hasTags( idList ))
                    m_contextMenu->setItemEnabled(i,false);
            }
        }
        
        m_contextMenu->exec(QCursor::pos());

        if (separatorID != -1)
        {
            m_contextMenu->removeItem(separatorID);
        }
        
        delete assignTagsMenu;
        delete removeTagsMenu;
    }
}

void ImageWindow::slotChanged(bool moreUndo, bool moreRedo)
{
    m_resLabel->setText(QString::number(m_canvas->imageWidth())  +
                        QString("x") +
                        QString::number(m_canvas->imageHeight()) +
                        QString(" ") +
                        i18n("pixels"));

    m_revertAction->setEnabled(moreUndo);
    m_undoAction->setEnabled(moreUndo);
    m_redoAction->setEnabled(moreRedo);

    if (m_allowSaving)
    {
        m_saveAction->setEnabled(moreUndo);
    }

    if (!moreUndo)
        m_rotatedOrFlipped = false;
        
    if (m_urlCurrent.isValid())
    {
        KURL u(m_urlCurrent.directory());
        PAlbum *palbum = AlbumManager::instance()->findPAlbum(u);
        
        QRect sel           = m_canvas->getSelectedArea();
        DImg* img           = DImgInterface::instance()->getImg();
        AlbumIconItem* item = 0;
        
        if (palbum)
           item = m_view->findItem(m_urlCurrent.url());
            
        m_rightSidebar->itemChanged(m_urlCurrent.url(), sel.isNull() ? 0 : &sel, img, m_view, item);
    }
}

void ImageWindow::slotSelected(bool val)
{
    // Update menu actions.
    m_cropAction->setEnabled(val);
    m_copyAction->setEnabled(val);

    ImagePluginLoader* loader = ImagePluginLoader::instance();
    for (ImagePlugin* plugin = loader->pluginList().first();
         plugin; plugin = loader->pluginList().next()) 
    {
        if (plugin) 
        {
            plugin->setEnabledSelectionActions(val);
        }
    }
    
    // Update histogram.
    
    QRect sel = m_canvas->getSelectedArea();
    m_rightSidebar->imageSelectionChanged( sel.isNull() ? 0 : &sel);
}

void ImageWindow::slotRotatedOrFlipped()
{
    m_rotatedOrFlipped = true;
}

void ImageWindow::slotDeleteCurrentItem()
{
    KURL u(m_urlCurrent.directory());
    PAlbum *palbum = AlbumManager::instance()->findPAlbum(u);

    if (!palbum)
        return;

    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings->getUseTrash())
    {
        QString warnMsg(i18n("About to Delete File \"%1\"\nAre you sure?")
                        .arg(m_urlCurrent.filename()));
        if (KMessageBox::warningContinueCancel(this,
                                               warnMsg,
                                               i18n("Warning"),
                                               i18n("Delete"))
            !=  KMessageBox::Continue)
        {
            return;
        }
    }

    if (!SyncJob::userDelete(m_urlCurrent))
    {
        QString errMsg(SyncJob::lastErrorMsg());
        KMessageBox::error(this, errMsg, errMsg);
        return;
    }

    emit signalFileDeleted(m_urlCurrent);

    KURL CurrentToRemove = m_urlCurrent;
    KURL::List::iterator it = m_urlList.find(m_urlCurrent);

    if (it != m_urlList.end())
    {
        if (m_urlCurrent != m_urlList.last())
        {
            // Try to get the next image in the current Album...

            KURL urlNext = *(++it);
            m_urlCurrent = urlNext;
            m_urlList.remove(CurrentToRemove);
            slotLoadCurrent();
            return;
        }
        else if (m_urlCurrent != m_urlList.first())
        {
            // Try to get the previous image in the current Album...

            KURL urlPrev = *(--it);
            m_urlCurrent = urlPrev;
            m_urlList.remove(CurrentToRemove);
            slotLoadCurrent();
            return;
        }
    }

    // No image in the current Album -> Quit ImageEditor...

    KMessageBox::information(this,
                             i18n("There is no image to show in the current album.\n"
                                  "The image editor will be closed."),
                             i18n("No Image in Current Album"));

    close();
}

bool ImageWindow::save()
{
    m_savingContext->saveTempFile = new KTempFile(m_urlCurrent.directory(false), QString::null);
    m_savingContext->saveTempFile->setAutoDelete(true);
    m_savingContext->saveURL = KURL();
    m_savingContext->fromSave = true;

    m_canvas->saveAsTmpFile(m_savingContext->saveTempFile->name(), m_IOFileSettings);
    return true;
}

bool ImageWindow::saveAs()
{
    // FIXME : Add 16 bits file formats and others files format like TIFF not supported by kimgio.

    QString mimetypes = KImageIO::mimeTypes(KImageIO::Writing).join(" ");
    mimetypes.append(" image/tiff");
    kdDebug () << "mimetypes=" << mimetypes << endl;    

    KFileDialog imageFileSaveDialog(m_urlCurrent.directory(),
                                    QString::null,
                                    this,
                                    "imageFileSaveDialog",
                                    false);

    imageFileSaveDialog.setOperationMode( KFileDialog::Saving );
    imageFileSaveDialog.setMode( KFile::File );
    imageFileSaveDialog.setSelection(m_urlCurrent.fileName());
    imageFileSaveDialog.setCaption( i18n("New Image File Name") );
    imageFileSaveDialog.setFilter(mimetypes);

    // Check for cancel.
    if ( imageFileSaveDialog.exec() != KFileDialog::Accepted )
    {
       return false;
    }

    KURL newURL = imageFileSaveDialog.selectedURL();

    // Check if target image format have been selected from Combo List of SaveAs dialog.
    m_savingContext->format = KImageIO::typeForMime(imageFileSaveDialog.currentMimeFilter());

    if ( m_savingContext->format.isEmpty() )
    {
        // Else, check if target image format have been add to target image file name using extension.

        QFileInfo fi(newURL.path());
        m_savingContext->format = fi.extension(false);
        
        if ( m_savingContext->format.isEmpty() )
        {
            // If format is empty then file format is same as that of the original file.
            m_savingContext->format = QImageIO::imageFormat(m_urlCurrent.path());
        }
        else
        {
            // Else, check if format from file name extension is include on file mime type list.

            QString imgExtPattern;
            QStringList imgExtList = QStringList::split(" ", mimetypes);
            for (QStringList::ConstIterator it = imgExtList.begin() ; it != imgExtList.end() ; it++)
            {    
                imgExtPattern.append (KImageIO::typeForMime(*it).upper());
                imgExtPattern.append (" ");
            }    
            imgExtPattern.append (" TIF TIFF");
            if ( imgExtPattern.contains("JPEG") ) imgExtPattern.append (" JPG");
    
            if ( !imgExtPattern.contains( m_savingContext->format.upper() ) )
            {
                KMessageBox::error(this, i18n("Target image file format \"%1\" unsupported.")
                        .arg(m_savingContext->format));
                kdWarning() << k_funcinfo << "target image file format " << m_savingContext->format << " unsupported!" << endl;
                return false;
            }
        }
    }
    
    if (!newURL.isValid())
    {
        KMessageBox::error(this, i18n("Failed to save file\n\"%1\" to Album\n\"%2\".")
                           .arg(newURL.filename())
                           .arg(newURL.path().section('/', -2, -2)));
        kdWarning() << k_funcinfo << "target URL isn't valid !" << endl;
        return false;
    }

    // if new and original url are save use slotSave() ------------------------------
    
    KURL currURL(m_urlCurrent);
    currURL.cleanPath();
    newURL.cleanPath();

    if (currURL.equals(newURL))
    {
        slotSave();
        return false;
    }

    // Check for overwrite ----------------------------------------------------------
    
    QFileInfo fi(newURL.path());
    m_savingContext->fileExists = false;
    if ( fi.exists() )
    {
        int result =

            KMessageBox::warningYesNo( this, 
                                       i18n("A file named \"%1\" already "
                                            "exists. Are you sure you want "
                                            "to overwrite it?")
                                       .arg(newURL.filename()),
                                       i18n("Overwrite File?"),
                                       i18n("Overwrite"),
                                       KStdGuiItem::cancel() );

        if (result != KMessageBox::Yes)
            return false;

        m_savingContext->fileExists = true;
    }

    // Now do the actual saving -----------------------------------------------------

    m_savingContext->saveTempFile = new KTempFile(newURL.directory(false), QString::null);
    m_savingContext->saveTempFile->setAutoDelete(true);
    m_savingContext->saveURL = newURL;
    m_savingContext->fromSave = false;

    m_canvas->saveAsTmpFile(m_savingContext->saveTempFile->name(), m_IOFileSettings, m_savingContext->format.lower());

    return true;
}

void ImageWindow::slotSavingStarted(const QString &filename)
{
    //TODO: disable actions as appropriate
    kapp->setOverrideCursor( KCursor::waitCursor() );
}

void ImageWindow::slotSavingFinished(const QString &filename, bool success)
{
    //TODO: enable actions as appropriate
    if (m_savingContext->fromSave)
    {
        // from save()
        if (!success)
        {
            kapp->restoreOverrideCursor();
            KMessageBox::error(this, i18n("Failed to save file\n\"%1\" to album\n\"%2\".")
                            .arg(m_urlCurrent.filename())
                            .arg(m_urlCurrent.path().section('/', -2, -2)));
            finishSaving(false);
            return;
        }

        if( m_rotatedOrFlipped || m_canvas->exifRotated() )
            KExifUtils::writeOrientation(m_savingContext->saveTempFile->name(), KExifData::NORMAL);

        if (::rename(QFile::encodeName(m_savingContext->saveTempFile->name()),
              QFile::encodeName(m_urlCurrent.path())) != 0)
        {
            kapp->restoreOverrideCursor();
            KMessageBox::error(this, i18n("Failed to overwrite original file"),
                               i18n("Error Saving File"));
            finishSaving(false);
            return;
        }

        delete m_savingContext->saveTempFile;
        m_savingContext->saveTempFile = 0;

        m_canvas->setModified( false );
        emit signalFileModified(m_urlCurrent);
        slotLoadCurrent();

        kapp->restoreOverrideCursor();
    }
    else
    {
        // from saveAs()
        if (success == false)
        {
            kapp->restoreOverrideCursor();
            KMessageBox::error(this, i18n("Failed to save file\n\"%1\" to album\n\"%2\".")
                            .arg(m_savingContext->saveURL.filename())
                            .arg(m_savingContext->saveURL.path().section('/', -2, -2)));

            finishSaving(false);
            return;
        }

        // only try to write exif if both src and destination are jpeg files
        if (QString(QImageIO::imageFormat(m_urlCurrent.path())).upper() == "JPEG" &&
            m_savingContext->format.upper() == "JPEG")
        {
            if( m_rotatedOrFlipped )
                KExifUtils::writeOrientation(m_savingContext->saveTempFile->name(), KExifData::NORMAL);
        }

        if (::rename(QFile::encodeName(m_savingContext->saveTempFile->name()),
              QFile::encodeName(m_savingContext->saveURL.path())) != 0)
        {
            kapp->restoreOverrideCursor();
            KMessageBox::error(this, i18n("Failed to save to new file"),
                               i18n("Error Saving File"));
            finishSaving(false);
            return;
        }

        delete m_savingContext->saveTempFile;
        m_savingContext->saveTempFile = 0;

        // Find the src and dest albums ------------------------------------------

        KURL srcDirURL(QDir::cleanDirPath(m_urlCurrent.directory()));
        PAlbum* srcAlbum = AlbumManager::instance()->findPAlbum(srcDirURL);
        if (!srcAlbum)
        {
            kapp->restoreOverrideCursor();
            kdWarning() << k_funcinfo << "Cannot find the source album" << endl;
            finishSaving(false);
            return;
        }

        KURL dstDirURL(QDir::cleanDirPath(m_savingContext->saveURL.directory()));
        PAlbum* dstAlbum = AlbumManager::instance()->findPAlbum(dstDirURL);
        if (!dstAlbum)
        {
            kapp->restoreOverrideCursor();
            kdWarning() << k_funcinfo << "Cannot find the destination album" << endl;
            finishSaving(false);
            return;
        }

        // Now copy the metadata of the original file to the new file ------------

        AlbumDB* db = AlbumManager::instance()->albumDB();
        db->copyItem(srcAlbum->id(), m_urlCurrent.fileName(),
                     dstAlbum->id(), m_savingContext->saveURL.fileName());

        // Add new file URL into list if the new file has been added in the current Album

        if ( srcAlbum == dstAlbum &&                        // Target Album = current Album ?
             m_urlList.find(m_savingContext->saveURL) == m_urlList.end() )    // The image file not already exist
        {                                                   // in the list.
            KURL::List::iterator it = m_urlList.find(m_urlCurrent);
            m_urlList.insert(it, m_savingContext->saveURL);
            m_urlCurrent = m_savingContext->saveURL;
        }

        if(m_savingContext->fileExists)
            emit signalFileModified(m_savingContext->saveURL);
        else
            emit signalFileAdded(m_savingContext->saveURL);

        m_canvas->setModified( false );
        kapp->restoreOverrideCursor();
        slotLoadCurrent();
    }

    finishSaving(true);
}

void ImageWindow::finishSaving(bool success)
{
    if (m_savingContext->progressDialog)
    {
        m_savingContext->synchronousSavingResult = success;
        m_savingContext->progressDialog->close();
    }
}

void ImageWindow::slotSavingProgress(const QString& filePath, float progress)
{
    //TODO: progress display in status bar
    if (m_savingContext->progressDialog)
    {
        // in synchronous saving mode
        m_savingContext->progressDialog->progressBar()->setProgress((int)(100 * progress));
    }
}

void ImageWindow::slotToggleFullScreen()
{
    if (m_fullScreen)
    {

#if QT_VERSION >= 0x030300
        setWindowState( windowState() & ~WindowFullScreen );
#else
        showNormal();
#endif
        menuBar()->show();
        statusBar()->show();

        QObject* obj = child("ToolBar","KToolBar");
        if (obj)
        {
            KToolBar* toolBar = static_cast<KToolBar*>(obj);
            if (m_fullScreenAction->isPlugged(toolBar) && m_removeFullScreenButton)
                m_fullScreenAction->unplug(toolBar);
            if (toolBar->isHidden())
                toolBar->show();
        }

        // -- remove the imageguiclient action accels ----

        unplugActionAccel(m_forwardAction);
        unplugActionAccel(m_backwardAction);
        unplugActionAccel(m_firstAction);
        unplugActionAccel(m_lastAction);
        unplugActionAccel(m_saveAction);
        unplugActionAccel(m_saveAsAction);
        unplugActionAccel(m_zoomPlusAction);
        unplugActionAccel(m_zoomMinusAction);
        unplugActionAccel(m_zoomFitAction);
        unplugActionAccel(m_cropAction);
        unplugActionAccel(m_filePrintAction);
        unplugActionAccel(m_fileDeleteAction);

        m_fullScreen = false;
    }
    else
    {
        // hide the menubar and the statusbar
        menuBar()->hide();
        statusBar()->hide();

        QObject* obj = child("ToolBar","KToolBar");
        if (obj)
        {
            KToolBar* toolBar = static_cast<KToolBar*>(obj);
            if (m_fullScreenHideToolBar)
                toolBar->hide();
            else
            {    
                if ( !m_fullScreenAction->isPlugged(toolBar) )
                {
                    m_fullScreenAction->plug(toolBar);
                    m_removeFullScreenButton=true;
                }
                else    
                {
                    // If FullScreen button is enable in toolbar settings
                    // We don't remove it at full screen out.
                    m_removeFullScreenButton=false;
                }
            }

        }

        // -- Insert all the imageguiclient actions into the accel --

        plugActionAccel(m_forwardAction);
        plugActionAccel(m_backwardAction);
        plugActionAccel(m_firstAction);
        plugActionAccel(m_lastAction);
        plugActionAccel(m_saveAction);
        plugActionAccel(m_saveAsAction);
        plugActionAccel(m_zoomPlusAction);
        plugActionAccel(m_zoomMinusAction);
        plugActionAccel(m_zoomFitAction);
        plugActionAccel(m_cropAction);
        plugActionAccel(m_filePrintAction);
        plugActionAccel(m_fileDeleteAction);

        showFullScreen();
        m_fullScreen = true;
    }
}

bool ImageWindow::promptUserSave()
{
    if (m_saveAction->isEnabled())
    {
        int result = KMessageBox::warningYesNoCancel(this,
                                  i18n("The image \"%1\" has been modified.\n"
                                       "Do you want to save it?")
                                       .arg(m_urlCurrent.filename()),
                                  QString::null,
                                  KStdGuiItem::save(),
                                  KStdGuiItem::discard());

        if (result == KMessageBox::Yes)
        {
            m_savingContext->progressDialog = new KProgressDialog(this, 0, QString::null,
                        i18n("Saving \"%1\"").arg(m_urlCurrent.filename()), true );
            m_savingContext->progressDialog->setAllowCancel(false);

            bool saving;
            if (m_isReadOnly)
                saving = saveAs();
            else
                saving = save();

            // we must return a value here, and we must wait on asynchronous saving
            //TODO: test this, think about this
            // What is enter_loop() good for?
            if (saving)
            {
                // if saving has started, enter modal dialog to synchronize
                // on the saving operation. When saving is finished, the dialog is closed.
                m_savingContext->progressDialog->exec();
                delete m_savingContext->progressDialog;
                m_savingContext->progressDialog = 0;
                return m_savingContext->synchronousSavingResult;
            }
            else
                return false;
        }
        else if (result == KMessageBox::No)
        {
            m_saveAction->setEnabled(false);
            return true;
        }
        else
            return false;
    }
    return true;
}

void ImageWindow::plugActionAccel(KAction* action)
{
    if (!action)
        return;

    m_accel->insert(action->text(),
                    action->text(),
                    action->whatsThis(),
                    action->shortcut(),
                    action,
                    SLOT(activate()));
}

void ImageWindow::unplugActionAccel(KAction* action)
{
    m_accel->remove(action->text());
}

void ImageWindow::slotAssignTag(int tagID)
{
    IconItem* item = m_view->findItem(m_urlCurrent.url());
    if (item)
    {
        ((AlbumIconItem*)item)->imageInfo()->setTag(tagID);
                
    }
}

void ImageWindow::slotRemoveTag(int tagID)
{
    IconItem* item = m_view->findItem(m_urlCurrent.url());
    if (item)
    {
        ((AlbumIconItem*)item)->imageInfo()->removeTag(tagID);
                
    }
}

ImageWindow* ImageWindow::imagewindow()
{
    if (!m_instance)
        new ImageWindow();

    return m_instance;
}

bool ImageWindow::imagewindowCreated()
{
    return m_instance;
}

ImageWindow* ImageWindow::m_instance = 0;

}  // namespace Digikam

#include "imagewindow.moc"

