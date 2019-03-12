#ifndef IRRQUICKWINDOW_H
#define IRRQUICKWINDOW_H

#include <QQuickWindow>

class IrrQuickWindow : public QQuickWindow
{
	Q_OBJECT
public:
	IrrQuickWindow(QWindow *parent = nullptr);

	void setOrientation(int o);
	int  getOrientation() const;
Q_SIGNALS:
	void onOrientationCahnged();
protected:
	/**
	 * @brief m_orientation
	 * 0 - noraml
	 * 1 - 90  degrees
	 * 2 - 180 degrees
	 * 3 - 270 degrees
	 */
	int m_orientation;
};

#endif // IRRQUICKWINDOW_H
