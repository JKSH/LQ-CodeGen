#include "labqt.h"
#include "bridge.h"
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
	QByteArray     argv0("LabQt.dll\0");
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
startWidgetEngine()
{
	if (bridge)
	{
		// The engine is already running
		return -1;
	}

	std::thread t(&run);
	t.detach();

	// Block until the engine has been initialized
	while (!bridge)
		std::this_thread::sleep_for(std::chrono::milliseconds(20));

	return 0;
}

qint32 Q_DECL_EXPORT
stopWidgetEngine()
{
	if (bridge)
		QMetaObject::invokeMethod(qApp, "quit", Qt::BlockingQueuedConnection);
	return 0;
}

//[TEMPLATE]
