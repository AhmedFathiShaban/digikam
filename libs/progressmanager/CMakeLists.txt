#
# Copyright (c) 2010-2017 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(libprogressmanager_SRCS
    dhistoryview.cpp
    dprogresswdg.cpp
    overlaywidget.cpp
    progressview.cpp
    progressmanager.cpp
    statusbarprogresswidget.cpp
    statusprogressbar.cpp
    workingwidget.cpp
    dworkingpixmap.cpp
)

include_directories(
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(progressmanager_src OBJECT ${libprogressmanager_SRCS})
