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
