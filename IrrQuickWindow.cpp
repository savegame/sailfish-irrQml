#include "IrrQuickWindow.h"


IrrQuickWindow::IrrQuickWindow(QWindow *parent)
	: QQuickWindow( parent )
{
	setPersistentOpenGLContext(true);
	setPersistentSceneGraph(true);
}

