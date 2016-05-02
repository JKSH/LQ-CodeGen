#ifndef LQBRIDGE_H
#define LQBRIDGE_H

#include <QtWidgets>
#include <QtSvg>
#include <QtWinExtras>
#include "lqapplication.h"
#include "lqtypes.h"

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

#endif // LQBRIDGE_H
