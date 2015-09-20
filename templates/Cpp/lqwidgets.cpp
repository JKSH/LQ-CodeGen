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

	LQApplication app(argc, argv.data());
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
qint32
startWidgetEngine(quintptr* _retVal, LStrHandle pluginDir)
{
	if (bridge)
	{
		*_retVal = 0;
		return LQ::EngineAlreadyRunningError;
	}

	QCoreApplication::addLibraryPath(QString::fromUtf8( (char*)(*pluginDir)->str, LStrLen(*pluginDir) ));
	std::thread t(&run);
	t.detach();

	// Block until the engine has been initialized
	while (!bridge)
		std::this_thread::sleep_for(std::chrono::milliseconds(20));

	*_retVal = (quintptr)qApp;
	return LQ::NoError;
}

qint32
stopWidgetEngine()
{
	if (!bridge)
		return LQ::EngineNotRunningError;
	QMetaObject::invokeMethod(qApp, "quit", Qt::BlockingQueuedConnection);
	return LQ::NoError;
}

qint32
registerEventRefs(LVUserEventRef* voidRef, LVUserEventRef* boolRef, LVUserEventRef* i32Ref, LVUserEventRef* dblRef, LVUserEventRef* stringRef)
{
	if (!bridge)
		return LQ::EngineNotRunningError;

	bridge->registerEventRef_void(voidRef);
	bridge->registerEventRef_bool(boolRef);
	bridge->registerEventRef_i32(i32Ref);
	bridge->registerEventRef_dbl(dblRef);
	bridge->registerEventRef_string(stringRef);
	return LQ::NoError;
}

// LabVIEW needs to prepend "2" to the string
qint32
connect_void(quintptr _instance, const char* encodedSignal)
{
	if (!bridge)
		return LQ::EngineNotRunningError;

	QObject::connect((QObject*)_instance, encodedSignal,
			bridge, SLOT(postLVEvent_void()));

	// TODO: Return error code if connection failed
	return LQ::NoError;
}
qint32
connect_bool(quintptr _instance, const char* encodedSignal)
{
	if (!bridge)
		return LQ::EngineNotRunningError;

	QObject::connect((QObject*)_instance, encodedSignal,
			bridge, SLOT(postLVEvent_bool(bool)));

	return LQ::NoError;
}
qint32
connect_i32(quintptr _instance, const char* encodedSignal)
{
	if (!bridge)
		return LQ::EngineNotRunningError;

	QObject::connect((QObject*)_instance, encodedSignal,
			bridge, SLOT(postLVEvent_i32(int)));

	return LQ::NoError;
}
qint32
connect_dbl(quintptr _instance, const char* encodedSignal)
{
	if (!bridge)
		return LQ::EngineNotRunningError;

	QObject::connect((QObject*)_instance, encodedSignal,
			bridge, SLOT(postLVEvent_dbl(double)));

	return LQ::NoError;
}
qint32
connect_string(quintptr _instance, const char* encodedSignal)
{
	if (!bridge)
		return LQ::EngineNotRunningError;

	QObject::connect((QObject*)_instance, encodedSignal,
			bridge, SLOT(postLVEvent_string(QString)));

	return LQ::NoError;
}

/*!
	Finds the value returned by QObject::senderSignalIndex() as called
	from a slot that was executed in response to a signal emission.

	This function is like QMetaObject::indexOfSignal(), except that it returns
	the index of the "full" version of a signal that has default parameters.
*/
qint32
findSignalIndex(qint64* _retVal, quintptr _instance, const char* normalizedSignal)
{
	if (!bridge)
		return LQ::EngineNotRunningError;

	QString head = QString::fromLatin1(normalizedSignal);
	head.chop(1);

	*_retVal = -1;
	int maxLength = 0;
	auto metaObject = ((QObject*)_instance)->metaObject();
	for(int i = 0; i < metaObject->methodCount(); ++i)
	{
		const QMetaMethod candidateMethod = metaObject->method(i);
		const QString candidateStr = QString::fromLatin1(candidateMethod.methodSignature());
		if (candidateMethod.methodType() == QMetaMethod::Signal
				&& candidateStr.startsWith(head)
				&& candidateStr.length() >= maxLength)
		{
			maxLength = candidateStr.length();
			*_retVal = i;
		}

		// Possible optimization: moc always generates meta method for the full version
		// before the reduced version(s). We could break the loop at the first match.
		// However, that would be the wrong index for an overridden virtual signal (not
		// that I can think of any such cases in Qt)
	}
	return LQ::NoError;
}

//[TEMPLATE]
