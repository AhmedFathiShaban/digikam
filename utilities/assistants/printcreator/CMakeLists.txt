#
# Copyright (c) 2010-2017, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::PrintSupport,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

set(libprintcreator_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/advprintutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/atkinspagelayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/templateicon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/advprintlayouttree.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/advprintphoto.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/advprintcropframe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/advprintphotopage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/advprintcroppage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/advprintcustomdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/advprintoptionspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/advprintintropage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/advprintimagesconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/advprintwizard.cpp
)

ki18n_wrap_ui(libprintcreator_SRCS
              ${CMAKE_CURRENT_SOURCE_DIR}/ui/advprintphotopage.ui
              ${CMAKE_CURRENT_SOURCE_DIR}/ui/advprintcroppage.ui
              ${CMAKE_CURRENT_SOURCE_DIR}/ui/advprintcustomlayout.ui
              ${CMAKE_CURRENT_SOURCE_DIR}/ui/advprintoptionspage.ui
)

add_library(printcreator_src OBJECT ${libprintcreator_SRCS})
