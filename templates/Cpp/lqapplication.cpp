#include "lqapplication.h"

/*!
	\class LQApplication

	The LQApplication class extends QApplication with glue code required
	to run a LabVIEW-driven Qt application.
*/

LQApplication::~LQApplication()
{
	// QMetaObjectBuilder creates these objects using malloc(),
	// so they must be free()'d.
	for(auto obj : lqMetaObjects)
		free(obj);
	for(auto obj : staleMetaObjects)
		free(obj);
}

LQApplication::BindingStatus
LQApplication::bindingStatus(const QByteArray& className) const
{
	int index = lqIndex(className);
	if (index == -1)
		return Unbound;
	if (bindingFinalists[index])
		return Finalized;
	return Started;
}

/*!
	This function is intended to be called by the LQMetaBinding constructor,
	to register a dynamic meta object for a LabVIEW-created subclass of QObject
	(LQObject).

	The caller must ensure that:
		- \a className is non-empty, and is the unique name of the LabVIEW class.
		- \a prototype is a stub QMetaObject that has

	Returns the index number of the newly-registered LQObject.
*/
int
LQApplication::initializeBinding(const QByteArray& className, QMetaObject* prototype)
{
	int index = lqMetaObjects.count();

	bindingMap << className;
	bindingFinalists << false;
	lqMetaObjects << prototype;

	return index;
}

/*!
	This function is intended to be called very shortly after the construction
	of a custom QObject subclass, to register the signals of this custom class.
	It takes pointers to raw LabVIEW datatypes to avoid the conversion overhead
	if the object is already finalized.

	The caller must ensure that all \a signalList elements are well-formed.
*/
LQ::Error
LQApplication::finalizeBinding(QObject* _instance, LVArray<LStrHandle>** signalList, LStrHandle superClassName)
{
	QByteArray className = _instance->metaObject()->className();

	auto status = bindingStatus(className);
	if (status == Finalized)
		return LQ::NoError; // Already finalized; don't need to do anything
	if (status == Unbound)
		return LQ::NotAnLQObjectError;

	// Augment the stub QMetaObject

	int index = lqIndex(className);
	QMetaObjectBuilder b(lqMetaObjects[index]);

	int superIndex = lqIndex(copyFromLStr(superClassName));
	if (superIndex >= 0)
		b.setSuperClass(lqMetaObjects[superIndex]);

	// TODO: Sanitize the signal signatures
	// TODO: Use a nicer API that converts LVArray<LStrHandle> to QByteArrayList (requires improved array support)
	for (int i = 0; i < (*signalList)->dimSize; ++i)
		b.addSignal(  copyFromLStr( (*signalList)->elt[i] )  );

	// Avoid race condition: Don't delete the stub QMetaObject yet, in case a
	// different thread started reading from it just before we finalize the
	// class. Defer deletion till shutdown.
	staleMetaObjects << lqMetaObjects[index];
	lqMetaObjects[index] = b.toMetaObject();
	bindingFinalists[index] = true;

	return LQ::NoError;
}
