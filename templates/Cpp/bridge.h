#ifndef BRIDGE_H
#define BRIDGE_H

#include <QtWidgets>
#include <QtSvg>
#include <QtWinExtras>
#include "lqapplication.h"
#include "utils.h"

#include <qwt_thermo.h>
#include <qwt_slider.h>

class Bridge : public QObject
{
	Q_OBJECT

public:
	explicit Bridge(QObject* parent = nullptr) : QObject(parent) {}

public slots:
//[TEMPLATE]
};

#endif
