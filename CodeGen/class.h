#ifndef CLASS_H
#define CLASS_H
/*
#include "method.h"

#include <QMap>
#include <QList>
#include <QString>
#include <QSharedPointer>
#include <QDebug>

class Class
{
public:
	Class(const QString& name) :
		_name(name),
		_base(nullptr)
	{}

	void setBase(Class* base)
	{
		_base = base;
		if (base)
			base->addDerived(this);
	}

	void addMethod(QSharedPointer<Methodd> method)
	{
		_methods[method->name()] = method;
	}

	QString name() const
	{
		return _name;
	}

	void print() const
	{
		qDebug() << _name;
		qDebug() << '\t' << _methods.keys();

		for (Class* c : _derived)
			c->print();
	}

private:

	void addDerived(Class* derived)
	{
		// This must only be called from setBase()
		// ASSUMPTION: derived is never NULL
		_derived << derived;
	}

	QString _name;
	Class* _base;
	QList<Class*> _derived;
	QMap<QString, QSharedPointer<Methodd>> _methods;
};
*/
#endif // CLASS_H
