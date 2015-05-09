#include "lqwidgets.h"
#include "bridge.h"
#include "errors.h"
#include <thread>
#include <QThread>

// STRATEGY: The Bridge pointer is always NULL before the GUI event loop is running,
// and NULL after the event loop has stopped
static Bridge* bridge = nullptr;

static void
run()
{
	// Note: The Bridge needs an active event loop to run slots

	int            argc = 1;
	QByteArray     argv0("LQWidgets.dll\0");
	QVector<char*> argv{argv0.data(), nullptr};

	QApplication app(argc, argv.data());
	app.setQuitOnLastWindowClosed(false); // Only quit explicitly when commanded from LabVIEW

	// TODO: Find a way to track all top-level widgets for deletion. CANNOT parent to qApp!

	// Tie the Bridge's lifetime to the QApplication object
	bridge = new Bridge(&app);

	qDebug() << "Starting Qt event loop in" << QThread::currentThread();
	app.exec();

	// Clean up
	bridge = nullptr;
	qDebug("Qt event loop stopped");
}

// TODO: Return version info to protect against mismatched VI-DLL combos
// TODO: Decide if the LV dev needs access to QApplication methods
qint32 Q_DECL_EXPORT
startWidgetEngine(LStrHandle pluginDir)
{
	if (bridge)
		return LQ::EngineAlreadyRunningError;

	QCoreApplication::addLibraryPath(QString::fromUtf8( (char*)(*pluginDir)->str, LStrLen(*pluginDir) ));
	std::thread t(&run);
	t.detach();

	// Block until the engine has been initialized
	while (!bridge)
		std::this_thread::sleep_for(std::chrono::milliseconds(20));

	return LQ::NoError;
}

qint32 Q_DECL_EXPORT
stopWidgetEngine()
{
	if (!bridge)
		return LQ::EngineNotRunningError;
	QMetaObject::invokeMethod(qApp, "quit", Qt::BlockingQueuedConnection);
	return LQ::NoError;
}

qint32 Q_DECL_EXPORT
registerEventRefs(LVUserEventRef* voidRef, LVUserEventRef* boolRef, LVUserEventRef* i32Ref, LVUserEventRef* stringRef)
{
	if (!bridge)
		return LQ::EngineNotRunningError;

	bridge->registerEventRef_void(voidRef);
	bridge->registerEventRef_bool(boolRef);
	bridge->registerEventRef_i32(i32Ref);
	bridge->registerEventRef_string(stringRef);
	return LQ::NoError;
}

// LabVIEW needs to prepend "2" to the string
qint32 Q_DECL_EXPORT
connect_void(quint32 qobject, const char* encodedSignal)
{
	if (!bridge)
		return LQ::EngineNotRunningError;

	QObject::connect((QObject*)qobject, encodedSignal,
			bridge, SLOT(postLVEvent_void()));

	// TODO: Return error code if connection failed
	return LQ::NoError;
}
qint32 Q_DECL_EXPORT
connect_bool(quint32 qobject, const char* encodedSignal)
{
	if (!bridge)
		return LQ::EngineNotRunningError;

	QObject::connect((QObject*)qobject, encodedSignal,
			bridge, SLOT(postLVEvent_bool(bool)));

	return LQ::NoError;
}
qint32 Q_DECL_EXPORT
connect_i32(quint32 qobject, const char* encodedSignal)
{
	if (!bridge)
		return LQ::EngineNotRunningError;

	QObject::connect((QObject*)qobject, encodedSignal,
			bridge, SLOT(postLVEvent_i32(int)));

	return LQ::NoError;
}
qint32 Q_DECL_EXPORT
connect_string(quint32 qobject, const char* encodedSignal)
{
	if (!bridge)
		return LQ::EngineNotRunningError;

	QObject::connect((QObject*)qobject, encodedSignal,
			bridge, SLOT(postLVEvent_string(QString)));

	return LQ::NoError;
}

/*!
	Finds the value returned by QObject::senderSignalIndex() as called
	from a slot that was executed in response to a signal emission.

	This function is like QMetaObject::indexOfSignal(), except that it returns
	the index of the "full" version of a signal that has default parameters.
*/
qint32 Q_DECL_EXPORT
findSignalIndex(qint64* retVal, quint32 qobject, const char* normalizedSignal)
{
	if (!bridge)
		return LQ::EngineNotRunningError;

	QString head = QString::fromLatin1(normalizedSignal);
	head.chop(1);

	*retVal = -1;
	int maxLength = 0;
	auto metaObject = ((QObject*)qobject)->metaObject();
	for(int i = 0; i < metaObject->methodCount(); ++i)
	{
		const QMetaMethod candidateMethod = metaObject->method(i);
		const QString candidateStr = QString::fromLatin1(candidateMethod.methodSignature());
		if (candidateMethod.methodType() == QMetaMethod::Signal
				&& candidateStr.startsWith(head)
				&& candidateStr.length() >= maxLength)
		{
			maxLength = candidateStr.length();
			*retVal = i;
		}

		// Possible optimization: moc always generates meta method for the full version
		// before the reduced version(s). We could break the loop at the first match.
		// However, that would be the wrong index for an overridden virtual signal (not
		// that I can think of any such cases in Qt)
	}
	return LQ::NoError;
}

//[TEMPLATE]
