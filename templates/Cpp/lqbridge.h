/*\
 * Copyright (c) 2016 Sze Howe Koh
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
\*/

#ifndef LQBRIDGE_H
#define LQBRIDGE_H

#include <QtWidgets>
#include <QtSvg>
#include <QtWinExtras>
#include "lqapplication.h"
#include "lqtypes.h"

#include <qwt_thermo.h>
#include <qwt_slider.h>

// NOTE: This file contains auto-generated code. Do not modify by hand.

class Bridge : public QObject
{
	Q_OBJECT

public:
	explicit Bridge(QObject* parent = nullptr) : QObject(parent) {}

public slots:
//[TEMPLATE]
};

// This is to be defined in lqmain.cpp
extern Bridge* bridge;

#endif // LQBRIDGE_H
