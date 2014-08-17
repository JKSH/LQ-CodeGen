#ifndef BRIDGE_H
#define BRIDGE_H

#include <QtWidgets>

class Bridge : public QObject
{
	Q_OBJECT

public:
	explicit Bridge(QObject* parent = nullptr) : QObject(parent) {}

public slots:
//[TEMPLATE]
};

#endif
