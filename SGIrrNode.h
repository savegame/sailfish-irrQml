#ifndef SGIRRNODE_H
#define SGIRRNODE_H

#include <QtCore/qglobal.h>
#include <QQuickItem>
#include <QRect>
#include <IrrlichtDevice.h>
#include <IVideoDriver.h>
#include <SMeshBuffer.h>
#include "ScreenNode.h"
#include <SIrrCreationParameters.h>

using namespace irr;
using namespace video;
using namespace core;
using namespace scene;

class IrrlichtQuickItem : public QQuickItem
{
	Q_OBJECT

	Q_PROPERTY(int renderOrientation WRITE setOrientation READ getOrientation NOTIFY onOrientationCahnged)
	Q_PROPERTY(QPointF joystick WRITE setJoystick NOTIFY onJoystickChanged)
	Q_PROPERTY(qreal cameraSpeed WRITE setSpeed READ getSpeed NOTIFY onSpeedChanged)
public:
	IrrlichtQuickItem(QQuickItem *parent = Q_NULLPTR);
	~IrrlichtQuickItem();

	typedef void (IrrlichtQuickItem::*initFunc)();
	typedef void (IrrlichtQuickItem::*renderInitFunc)(SIrrlichtCreationParameters &params);
	initFunc init;
	renderInitFunc init_render;
	initFunc render;

	Q_INVOKABLE void loadExample(int index);

	void setOrientation(int o);
	int  getOrientation() const;
	void setJoystick(QPointF v);

	void setSpeed(qreal s);
	qreal getSpeed() const { return m_speed; }

Q_SIGNALS:
	void onOrientationCahnged();
	void onJoystickChanged();
	void onSpeedChanged();


protected:
	/**
	 * @brief m_orientation
	 * 0 - noraml
	 * 1 - 90  degrees
	 * 2 - 180 degrees
	 * 3 - 270 degrees
	 */
	int m_orientation;
	int m_prevOrientation;

	QPointF m_joyStick;


protected:
	void sendKeyEventToIrrlicht( QKeyEvent* event, bool pressedDown );
	virtual void keyPressEvent( QKeyEvent* event ) override;
	virtual void keyReleaseEvent( QKeyEvent* event ) override;

	void sendMouseEventToIrrlicht( QMouseEvent* event, bool pressedDown );
	virtual void mousePressEvent( QMouseEvent* event ) override;
	virtual void mouseReleaseEvent( QMouseEvent* event ) override;
	virtual void mouseMoveEvent( QMouseEvent* event ) override;
	virtual void wheelEvent( QWheelEvent* event ) override;
	virtual void touchEvent(QTouchEvent* event) override;

	void focusInEvent(QFocusEvent * event) override
	{
		forceActiveFocus();
		event->accept();
	}

protected:
	void createCube();
	bool clear_scene();

	void _first_init();
	void _init_rt_render(SIrrlichtCreationParameters &params);
	void _init_direct_render(SIrrlichtCreationParameters &params);
	void _empty_init() {}

	const io::path getExampleMediaPath();

	void _load_example_1();
	void _load_example_2();
	void _load_example_11();
	void _load_example_16();

	void _init_render_rt();

	void _standart_render();
	void _render_rt();
	void _render_direct();
signals:
	void updateSignal();

protected slots:
	void windowChangedSlot( QQuickWindow* window );

protected:
	QSGNode* updatePaintNode( QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData ) override;

	QRectF _geometry;
	IVideoDriver    *m_driver;
	ISceneManager   *m_scene;
	IrrlichtDevice  *m_device;

	//------------------------
	ITexture        *m_normal;
	ITexture        *m_depth;
	ITexture        *m_color;
	IRenderTarget   *m_rt;
	ScreenNode      *m_node;
	SMeshBuffer     m_mesh;

	irr::u32        m_lastTime;
	qreal           m_speed;
};

#endif // SGIRRNODE_H
