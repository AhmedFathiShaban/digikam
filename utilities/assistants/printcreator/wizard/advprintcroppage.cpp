/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to print images
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "advprintcroppage.h"

// Qt includes

#include <QIcon>
#include <QWidget>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "advprintwizard.h"

namespace Digikam
{

class AdvPrintCropPage::Private
{
public:

    template <class Ui_Class>

    class WizardUI : public QWidget, public Ui_Class
    {
    public:

        WizardUI(QWidget* const parent)
            : QWidget(parent)
        {
            this->setupUi(this);
        }
    };

    typedef WizardUI<Ui_AdvPrintCropPage> CropUI;

public:

    Private(QWidget* const parent)
    {
        cropUi = new CropUI(parent);
    }

    CropUI* cropUi;
};

AdvPrintCropPage::AdvPrintCropPage(QWizard* const wizard, const QString& title)
    : DWizardPage(wizard, title),
      d(new Private(this))
{
    d->cropUi->BtnCropRotateRight->setIcon(QIcon::fromTheme(QLatin1String("object-rotate-right"))
                                           .pixmap(16, 16));
    d->cropUi->BtnCropRotateLeft->setIcon(QIcon::fromTheme(QLatin1String("object-rotate-left"))
                                          .pixmap(16, 16));

    connect(d->cropUi->m_disableCrop, SIGNAL(stateChanged(int)),
            this, SLOT(slotCropSelection(int)));

    // -----------------------------------

    connect(d->cropUi->BtnCropPrev, SIGNAL(clicked()),
            wizard, SLOT(slotBtnCropPrevClicked()));

    connect(d->cropUi->BtnCropNext, SIGNAL(clicked()),
            wizard, SLOT(slotBtnCropNextClicked()));

    connect(d->cropUi->BtnCropRotateRight, SIGNAL(clicked()),
            wizard, SLOT(slotBtnCropRotateRightClicked()));

    connect(d->cropUi->BtnCropRotateLeft, SIGNAL(clicked()),
            wizard, SLOT(slotBtnCropRotateLeftClicked()));

    connect(d->cropUi->BtnSaveAs, SIGNAL (clicked()),
            wizard, SLOT (slotBtnSaveAsClicked()));

    // -----------------------------------

    setPageWidget(d->cropUi);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("transform-crop")));
}

AdvPrintCropPage::~AdvPrintCropPage()
{
    delete d;
}

Ui_AdvPrintCropPage* AdvPrintCropPage::ui() const
{
    return d->cropUi;
}

void AdvPrintCropPage::updateUi()
{
    d->cropUi->update();
}

void AdvPrintCropPage::slotCropSelection(int)
{
    d->cropUi->cropFrame->drawCropRectangle(!d->cropUi->m_disableCrop->isChecked());
    updateUi();
}

} // namespace Digikam
