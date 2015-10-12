#ifndef LQAPPLICATION_H
#define LQAPPLICATION_H

#include <private/qmetaobjectbuilder_p.h>
#include <QApplication>
#include <QVector>
#include "errors.h"
#include "utils.h"

class LQApplication : public QApplication
{
	Q_OBJECT

public:
	enum BindingStatus
	{
		Unbound,
		Started,
		Finalized
	};

	LQApplication(int& argc, char** argv) : QApplication(argc, argv) {}

	~LQApplication();
	BindingStatus bindingStatus(const QByteArray& className) const;
	int initializeBinding(const QByteArray& className, QMetaObject* prototype);

	Q_INVOKABLE LQ::Error finalizeBinding(QObject* _instance, LVArray<LStrHandle>** signalList, LStrHandle superClassName);

	int lqIndex(const QByteArray& className) const
	{ return bindingMap.indexOf(className); }

	const QMetaObject* lqMetaObject(int index) const
	{ return lqMetaObjects[index]; }

private:
	QByteArrayList bindingMap;
	QVector<bool> bindingFinalists;
	QVector<QMetaObject*> lqMetaObjects;
	QVector<QMetaObject*> staleMetaObjects;
};

class LQSignalRelay : public QObject
{
	Q_OBJECT

public slots:
	// ASSUMPTION: Only one of these slots will be connected per class instance
	void activate() const                    { activate_impl(nullptr); }
	void activate(bool data) const           { activate_impl(&data); }
	void activate(int data) const            { activate_impl(&data); }
	void activate(double data) const         { activate_impl(&data); }
	void activate(const QString& data) const { activate_impl( (void*)&data ); }

public:
	Q_INVOKABLE LQSignalRelay() : QObject() {}
	Q_INVOKABLE void finalize(QObject* target, int signalIndex);

private:
	void activate_impl(void* dataAddr) const;
	int signalIndex;
};

/*!
	LQMetaBinding is used to modify a native C++ QObject when the user
	creates a custom subclass in LabVIEW. It enables user-defined signals
	via dynamic meta objects.

	\a T must be a native C++ subclass of QObject (or be QObject itself).

	\note Members injected into different classes will likely have different
	memory layout offsets, so this template cannot provide a single, unified
	interface to member variables/functions.
*/
template <class T>
class LQMetaBinding : public T
{
public:
	template <typename ... Uu>
	LQMetaBinding(const QByteArray& className, Uu ... args) : T(args ...)
	{
		auto app = static_cast<LQApplication*>(qApp);

		// ASSUMPTION: className is not empty
		int i = app->lqIndex(className);
		if (i != -1)
		{
			// Just retrieve the previously-registered index.
			lqIndex = i;
			return;
		}

		// Create a stub of a dynamic meta object, which can be updated later.
		// The main purpose of this is to capture the static meta object of the native class.
		QMetaObjectBuilder b;
		b.setClassName(className);
		b.setSuperClass( &(this->staticMetaObject) ); // GCC 4.9.2 needs the 'this' pointer. MSVC 2013 doesn't.

		lqIndex = app->initializeBinding(className, b.toMetaObject());
	}

	const QMetaObject* metaObject() const override
	{ return static_cast<LQApplication*>(qApp)->lqMetaObject(lqIndex); }

private:
	int lqIndex;
};

/*!
	A wrapper function for QObject-derived constructors.

	If LabVIEW requests an instance of a native C++ class, then the raw class
	will be constructed. Otherwise (if LabVIEW requests an instance of a LabVIEW-
	implemented subclass), the closest native C++ ancestor will be wrapped with
	an LQMetaBinding to facilitate a dynamic meta object.

	\a T must be a native C++ subclass of QObject (or be QObject itself).
*/
template<class T, typename ... Uu> T*
newLQObject(const char* className, Uu ... args)
{
	if (  strcmp( className, T::staticMetaObject.className() ) == 0  )
		return new T(args ...);

	return new LQMetaBinding<T>(className, args ...);
}

#endif // LQAPPLICATION_H
