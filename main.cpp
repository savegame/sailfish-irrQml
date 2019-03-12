#include <QtCore/qglobal.h>
#if QT_VERSION >= 0x050000
	#include <QtGui/QGuiApplication>
	#include <QtQml/QQmlApplicationEngine>
#else
#endif

#include <IrrQuick.h>
#include "SGIrrNode.h"
#include "IrrQuickWindow.h"

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QGuiApplication app(argc, argv);

//	qmlRegisterType<IrrQuickItem>("ru.sashikknox",1,0,"IrrQuickItem");
	qmlRegisterType<IrrQuickWindow>("ru.sashikknox", 1, 0, "IrrQuickWindow");
	qmlRegisterType<IrrlichtQuickItem>("ru.sashikknox", 1, 0, "IrrQuickItem");

	QQmlApplicationEngine engine;
	engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
	if (engine.rootObjects().isEmpty())
		return -1;
	return app.exec();
}
