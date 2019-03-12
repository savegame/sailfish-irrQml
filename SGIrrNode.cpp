#include "SGIrrNode.h"
#include <QQuickWindow>
#include <QSGNode>
#include <QOpenGLFunctions>
#include <CQGLFunctionsDriver.h>
#include <IrrlichtDevice.h>
#include <QOpenGLContext>
#include <QTouchEvent>
#include <irrlicht.h>

using namespace irr;
using namespace video;
using namespace core;
using namespace scene;
using namespace gui;

// static
struct SIrrlichtKey
{
	irr::EKEY_CODE code;
	wchar_t ch;
};

SIrrlichtKey convertToIrrlichtKey( int key )
{
	SIrrlichtKey irrKey;
	irrKey.code = (irr::EKEY_CODE)(0);
	irrKey.ch = (wchar_t)(0);

	// Letters A..Z and numbers 0..9 are mapped directly
	if ( (key >= Qt::Key_A && key <= Qt::Key_Z) || (key >= Qt::Key_0 && key <= Qt::Key_9) )
	{
		irrKey.code = (irr::EKEY_CODE)( key );
		irrKey.ch = (wchar_t)( key );
	}
	else

		// Dang, map keys individually
		switch ( key )
		{
		case Qt::Key_Up:
			irrKey.code = irr::KEY_UP;
			break;

		case Qt::Key_Down:
			irrKey.code = irr::KEY_DOWN;
			break;

		case Qt::Key_Left:
			irrKey.code = irr::KEY_LEFT;
			break;

		case Qt::Key_Right:
			irrKey.code = irr::KEY_RIGHT;
			break;
		}
	return irrKey;
}

// all other
IrrlichtQuickItem::IrrlichtQuickItem(QQuickItem *parent)
	: QQuickItem(parent)
	, m_device(nullptr)
	, m_driver(nullptr)
	, m_scene(nullptr)
	, m_node(nullptr)
	, m_prevOrientation(0)
	, m_orientation(0)
{
	setFlag( ItemHasContents, true );
	setAcceptedMouseButtons( Qt::AllButtons );
	setAcceptHoverEvents( true );

	connect( this, &QQuickItem::windowChanged, this, &IrrlichtQuickItem::windowChangedSlot );
	connect( this, &IrrlichtQuickItem::updateSignal, this, &QQuickItem::update );

	init = &IrrlichtQuickItem::_first_init;
	init_render = &IrrlichtQuickItem::_init_direct_render;
}

IrrlichtQuickItem::~IrrlichtQuickItem()
{
	if (!m_device)
		return;
}

void IrrlichtQuickItem::sendKeyEventToIrrlicht( QKeyEvent* event, bool pressedDown )
{
	irr::SEvent irrEvent;

	irrEvent.EventType = irr::EET_KEY_INPUT_EVENT;

	SIrrlichtKey irrKey = convertToIrrlichtKey( event->key() );

	if ( irrKey.code == 0 ) return; // Could not find a match for this key

	irrEvent.KeyInput.Key = irrKey.code;
	irrEvent.KeyInput.Control = ((event->modifiers() & Qt::ControlModifier) != 0);
	irrEvent.KeyInput.Shift = ((event->modifiers() & Qt::ShiftModifier) != 0);
	irrEvent.KeyInput.Char = irrKey.ch;
	irrEvent.KeyInput.PressedDown = pressedDown;

	m_device->postEventFromUser( irrEvent );
}

void IrrlichtQuickItem::keyPressEvent( QKeyEvent* event )
{
	if ( m_device != nullptr )
	{
		sendKeyEventToIrrlicht( event, true );
		update();
	}
	else
		event->ignore();
}

void IrrlichtQuickItem::keyReleaseEvent( QKeyEvent* event )
{
	if ( m_device != nullptr )
	{
		sendKeyEventToIrrlicht( event, false );
//		update();
	}
	else
		event->ignore();
}

void IrrlichtQuickItem::sendMouseEventToIrrlicht( QMouseEvent* event, bool pressedDown )
{
	irr::SEvent irrEvent;

	irrEvent.EventType = irr::EET_MOUSE_INPUT_EVENT;

	switch ( event->button() )
	{
	case Qt::LeftButton:
		irrEvent.MouseInput.Event = pressedDown ? irr::EMIE_LMOUSE_PRESSED_DOWN : irr::EMIE_LMOUSE_LEFT_UP;
		break;

	case Qt::MidButton:
		irrEvent.MouseInput.Event = pressedDown ? irr::EMIE_MMOUSE_PRESSED_DOWN : irr::EMIE_MMOUSE_LEFT_UP;
		break;

	case Qt::RightButton:
		irrEvent.MouseInput.Event = pressedDown ? irr::EMIE_RMOUSE_PRESSED_DOWN : irr::EMIE_RMOUSE_LEFT_UP;
		break;

	default:
		return; // Cannot handle this mouse event
	}

	irrEvent.MouseInput.X = event->x();
	irrEvent.MouseInput.Y = event->y();
	irrEvent.MouseInput.Wheel = 0.0f; // Zero is better than undefined

	m_device->postEventFromUser( irrEvent );
}

void IrrlichtQuickItem::mousePressEvent( QMouseEvent* event )
{
	if ( m_device != nullptr )
	{
		sendMouseEventToIrrlicht( event, true );
		update();
	}
	else
		event->ignore();
}

void IrrlichtQuickItem::mouseReleaseEvent( QMouseEvent* event )
{
	if ( m_device != nullptr )
	{
		sendMouseEventToIrrlicht( event, false );
		update();
	}
	else
		event->ignore();
}

void IrrlichtQuickItem::mouseMoveEvent(QMouseEvent *event)
{
	if ( m_device != nullptr )
	{
		//sendMouseEventToIrrlicht( event, false );
		irr::SEvent irrEvent;
		irrEvent.EventType = irr::EET_MOUSE_INPUT_EVENT;

		irrEvent.MouseInput.X = event->x();
		irrEvent.MouseInput.Y = event->y();
		irrEvent.MouseInput.Wheel = 0.0f; // Zero is better than undefined
		irrEvent.MouseInput.Event = irr::EMIE_MOUSE_MOVED;

		m_device->postEventFromUser( irrEvent );
		update();
	}
	else
		event->ignore();
}

void IrrlichtQuickItem::wheelEvent( QWheelEvent* event )
{
	if ( m_device != nullptr && event->orientation() == Qt::Vertical )
	{
		irr::SEvent irrEvent;

		irrEvent.EventType = irr::EET_MOUSE_INPUT_EVENT;
		irrEvent.MouseInput.Event = irr::EMIE_MOUSE_WHEEL;
		irrEvent.MouseInput.X = 0; // We don't know these,
		irrEvent.MouseInput.Y = 0; // but better zero them instead of letting them be undefined
		irrEvent.MouseInput.Wheel = event->delta() / 120.0f;

		if ( !m_device->postEventFromUser( irrEvent ) )
			event->ignore();
		update();
	}
	else
		event->ignore();
}

void IrrlichtQuickItem::touchEvent(QTouchEvent *event)
{
	if ( m_device != nullptr )
	{
		bool ignore = true;
		foreach (QTouchEvent::TouchPoint p, event->touchPoints())
		{
			irr::SEvent irrEvent;
			irrEvent.EventType = irr::EET_TOUCH_INPUT_EVENT;
			switch (p.state())
			{
//			The touch point is now pressed.
			case Qt::TouchPointPressed:
				irrEvent.TouchInput.Event = irr::ETIE_PRESSED_DOWN;
				break;
//			The touch point moved.
			case Qt::TouchPointMoved:
				irrEvent.TouchInput.Event = irr::ETIE_MOVED;
				break;
//			The touch point did not move.
			case Qt::TouchPointStationary:
				continue;
				break;
//			The touch point was released.
			case Qt::TouchPointReleased:
				irrEvent.TouchInput.Event = irr::ETIE_LEFT_UP;
				break;
			}
			irrEvent.TouchInput.ID = p.id();
			irrEvent.TouchInput.X = p.pos().x() * (qreal)m_driver->getScreenSize().Width; // We don't know these,
			irrEvent.TouchInput.Y = p.pos().y() * (qreal)m_driver->getScreenSize().Height; // but better zero them instead of letting them be undefined
			if ( m_device->postEventFromUser( irrEvent ) )
				ignore = false;
		}
		if (ignore)
			event->ignore();
		update();
	}
	else
		event->ignore();
}

void IrrlichtQuickItem::createCube()
{
	if (!m_device)
		return;
	scene::ISceneManager *scene = m_device->getSceneManager();
	video::IVideoDriver *driver = m_device->getVideoDriver();
	scene::ISceneNode *cube = scene->addCubeSceneNode(2);
	io::path p = ":/irrlicht/media/"; _MEDIA_PATH;
	p += "irrlicht2_lf.jpg";
	video::ITexture *texture = driver->getTexture(p);
	if (texture)
		cube->setMaterialTexture(0, texture);

	if (scene->getActiveCamera() == NULL)
	{
		scene::ICameraSceneNode *cam = scene->addCameraSceneNode();
		cam->setPosition( core::vector3df(-5, 0, 0) );
		cam->setTarget(cube->getPosition());
	}

	scene::ISceneNodeAnimator *anim = scene->createRotationAnimator( core::vector3df(1.0, 1.0, 0) );
	cube->addAnimator(anim);
	anim->drop();
//	return;
	cube = scene->addBillboardSceneNode(NULL, core::dimension2df(3, 3) );
	cube->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL);
	cube->setMaterialTexture(0, texture);
}

bool IrrlichtQuickItem::clear_scene()
{
	if (!m_device)
		return  false;
	m_device->getSceneManager()->clear();
	m_device->getGUIEnvironment()->clear();
	return true;
}

void IrrlichtQuickItem::loadExample(int index)
{

	if (!clear_scene())
	{
		init = &IrrlichtQuickItem::_empty_init;
		return;
	}

	switch (index)
	{
	case 1:
		init = &IrrlichtQuickItem::_load_example_1;
		createCube(); // this garantue of two differend materials in scene
		break;
	case 2:
		init = &IrrlichtQuickItem::_load_example_2;
		break;
	case 11:
		init = &IrrlichtQuickItem::_load_example_11;
//		createCube(); // this garantue of two differend materials in scene
		break;
	case 16:
		init = &IrrlichtQuickItem::_load_example_16;
		break;
	default:
		createCube();
		break;
	}


}

void IrrlichtQuickItem::setOrientation(int o)
{
	if ( o < 0 )
		o = 0;
	else if ( o > 3 )
		o = 3;
	m_orientation = o;
	if (m_node)
		m_node->setOrientation(o);

	emit onOrientationCahnged();
}

int IrrlichtQuickItem::getOrientation() const
{
	return m_orientation;
}

void IrrlichtQuickItem::setJoystick(QPointF v)
{
	m_joyStick = v;
}

void IrrlichtQuickItem::setSpeed(qreal s)
{
	m_speed = s;
	emit onSpeedChanged();
}

void IrrlichtQuickItem::_first_init()
{
	if ( !window()->openglContext() || !window()->openglContext()->functions() )
		return;
	window()->openglContext()->functions()->initializeOpenGLFunctions();

	SIrrlichtCreationParameters params;

	params.DriverType = EDT_QOGLDUNCTIONS;//driverType;
	params.DeviceType = EIDT_CONSOLE;
	params.WindowId = NULL;
	params.Bits = 32;
	params.Doublebuffer = true;
	params.Fullscreen = false;
	params.IgnoreInput = false;
	params.Stencilbuffer = true;
	params.Stereobuffer = false;
	params.WindowSize.Width = window()->geometry().width();
	params.WindowSize.Height = window()->geometry().height();
	params.AntiAlias = 0;
	params.OGLES2ShaderPath = _MEDIA_PATH;
	params.OGLES2ShaderPath += "Shaders/";
	params.Vsync = false;
	params.qOpenGLFunctions = window()->openglContext()->functions();

	m_device = createDeviceEx(params);
	m_driver = m_device->getVideoDriver();
	m_scene = m_device->getSceneManager();

	(this->*init_render)(params);

	createCube();

	init = &IrrlichtQuickItem::_empty_init;
	init = &IrrlichtQuickItem::_load_example_16;
	//	init = &IrrlichtQuickItem::_init_render_rt;
}

void IrrlichtQuickItem::_init_rt_render(SIrrlichtCreationParameters &params)
{
	m_color = m_driver->addRenderTargetTexture( params.WindowSize, "rt_color", ECF_A8R8G8B8 );
	m_depth = m_driver->addRenderTargetTexture( params.WindowSize, "rt_depth", ECF_D16 );

	m_rt = m_driver->addRenderTarget();
	m_rt->setTexture( m_color, m_depth );

	m_node = new ScreenNode(nullptr, m_scene, -1);
	m_node->setMaterialTexture(0, m_color);

	m_color->drop();
	m_depth->drop();
	render = &IrrlichtQuickItem::_render_rt;
}

void IrrlichtQuickItem::_init_direct_render(SIrrlichtCreationParameters &params)
{
	render = &IrrlichtQuickItem::_render_direct;
}



const io::path IrrlichtQuickItem::getExampleMediaPath()
{
	io::path p = _MEDIA_PATH;
	return  p;
}

void IrrlichtQuickItem::_load_example_1()
{
	/** begin example 1 code */
	IVideoDriver* driver = m_device->getVideoDriver();
	ISceneManager* smgr = m_device->getSceneManager();
	IGUIEnvironment* guienv = m_device->getGUIEnvironment();

	guienv->addStaticText(L"Hello World! This is Irrlicht with the burnings software renderer!",
	    rect<s32>(10, 10, 260, 22), true);

	const io::path mediaPath =  getExampleMediaPath();

	IAnimatedMesh* mesh = smgr->getMesh(mediaPath + "sydney.md2");
	if (!mesh)
	{
		// TODO here need use signal of bad scene, for QML
		clear_scene();
		init = &IrrlichtQuickItem::_empty_init;
		return;
	}
	IAnimatedMeshSceneNode* node = smgr->addAnimatedMeshSceneNode( mesh );

	if (node)
	{
		node->setMaterialFlag(EMF_LIGHTING, false);
		node->setMD2Animation(scene::EMAT_STAND);
		node->setMaterialTexture( 0, driver->getTexture(mediaPath + "sydney.bmp") );
	}

	smgr->addCameraSceneNode(0, vector3df(0, 30, -40), vector3df(0, 5, 0));
	/** end example 1 */

	/** set epmty fuctions pointer */
	init = &IrrlichtQuickItem::_empty_init;
}

void IrrlichtQuickItem::_load_example_2()
{
	video::IVideoDriver* driver = m_device->getVideoDriver();
	scene::ISceneManager* smgr = m_device->getSceneManager();

	m_device->getFileSystem()->addFileArchive(getExampleMediaPath() + "map-20kdm2.pk3");
	scene::IAnimatedMesh* mesh = smgr->getMesh("20kdm2.bsp");
	scene::ISceneNode* node = 0;

	if (mesh)
		node = smgr->addOctreeSceneNode(mesh->getMesh(0), 0, -1, 1024);
//		node = smgr->addMeshSceneNode(mesh->getMesh(0));

	if (node)
		node->setPosition(core::vector3df(-1300, -144, -1249));

	smgr->addCameraSceneNodeFPS(0, 1.0f);

	/*
	The mouse cursor needs not be visible, so we hide it via the
	irr::IrrlichtDevice::ICursorControl.
	*/
//	m_device->getCursorControl()->setVisible(false);

	/** set epmty fuctions pointer */
	init = &IrrlichtQuickItem::_empty_init;
}

void IrrlichtQuickItem::_load_example_11()
{
	video::IVideoDriver* driver = m_device->getVideoDriver();
	scene::ISceneManager* smgr = m_device->getSceneManager();
	gui::IGUIEnvironment* env = m_device->getGUIEnvironment();

	driver->setTextureCreationFlag(video::ETCF_ALWAYS_32_BIT, true);

	const io::path mediaPath = getExampleMediaPath();

	// add irrlicht logo
	env->addImage(driver->getTexture(mediaPath + "irrlichtlogo3.png"),
	    core::position2d<s32>(10, 10));

	// add camera
	scene::ICameraSceneNode* camera = smgr->addCameraSceneNodeFPS();
	camera->setPosition(core::vector3df(-200, 200, -200));

	// disable mouse cursor
//	m_device->getCursorControl()->setVisible(false);
	driver->setFog(video::SColor(0, 138, 125, 81), video::EFT_FOG_LINEAR, 250, 1000, .003f, true, false);

	scene::IAnimatedMesh* roomMesh = smgr->getMesh(mediaPath + "room.3ds");
	scene::ISceneNode* room = 0;
	scene::ISceneNode* earth = 0;

	if (roomMesh)
	{
		smgr->getMeshManipulator()->makePlanarTextureMapping(
		    roomMesh->getMesh(0), 0.003f);

		video::ITexture* normalMap =
		    driver->getTexture(mediaPath + "rockwall_height.bmp");

		if (normalMap)
			driver->makeNormalMapTexture(normalMap, 9.0f);
		/*
				// The Normal Map and the displacement map/height map in the alpha channel
				video::ITexture* normalMap =
					driver->getTexture(mediaPath + "rockwall_NRM.tga");
		*/
		scene::IMesh* tangentMesh = smgr->getMeshManipulator()->
		    createMeshWithTangents(roomMesh->getMesh(0));

		room = smgr->addMeshSceneNode(tangentMesh);
		room->setMaterialTexture(0,
		    driver->getTexture(mediaPath + "rockwall.jpg"));
		room->setMaterialTexture(1, normalMap);

		// Stones don't glitter..
		room->getMaterial(0).SpecularColor.set(0, 0, 0, 0);
		room->getMaterial(0).Shininess = 0.f;

		room->setMaterialFlag(video::EMF_FOG_ENABLE, true);
		room->setMaterialType(video::EMT_PARALLAX_MAP_SOLID);
		// adjust height for parallax effect
		room->getMaterial(0).MaterialTypeParam = 1.f / 64.f;

		// drop mesh because we created it with a create.. call.
		tangentMesh->drop();
	}

	scene::IAnimatedMesh* earthMesh = smgr->getMesh(mediaPath + "earth.x");
	if (earthMesh)
	{
		//perform various task with the mesh manipulator
		scene::IMeshManipulator *manipulator = smgr->getMeshManipulator();

		// create mesh copy with tangent information from original earth.x mesh
		scene::IMesh* tangentSphereMesh =
		    manipulator->createMeshWithTangents(earthMesh->getMesh(0));

		// set the alpha value of all vertices to 200
		manipulator->setVertexColorAlpha(tangentSphereMesh, 200);

		// scale the mesh by factor 50
		core::matrix4 m;
		m.setScale ( core::vector3df(50, 50, 50) );
		manipulator->transform( tangentSphereMesh, m );

		earth = smgr->addMeshSceneNode(tangentSphereMesh);

		earth->setPosition(core::vector3df(-70, 130, 45));

		// load heightmap, create normal map from it and set it
		video::ITexture* earthNormalMap = driver->getTexture(mediaPath + "earthbump.jpg");
		if (earthNormalMap)
		{
			driver->makeNormalMapTexture(earthNormalMap, 20.0f);
			earth->setMaterialTexture(1, earthNormalMap);
			earth->setMaterialType(video::EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA);
		}

		// adjust material settings
		earth->setMaterialFlag(video::EMF_FOG_ENABLE, true);

		// add rotation animator
		scene::ISceneNodeAnimator* anim =
		    smgr->createRotationAnimator(core::vector3df(0, 0.1f, 0));
		earth->addAnimator(anim);
		anim->drop();

		// drop mesh because we created it with a create.. call.
		tangentSphereMesh->drop();
	}

	scene::ILightSceneNode* light1 =
	    smgr->addLightSceneNode(0, core::vector3df(0, 0, 0),
	        video::SColorf(0.5f, 1.0f, 0.5f, 0.0f), 800.0f);

	// add fly circle animator to light 1
	scene::ISceneNodeAnimator* anim =
	    smgr->createFlyCircleAnimator (core::vector3df(50, 300, 0), 190.0f, -0.003f);
	light1->addAnimator(anim);
	anim->drop();

	// attach billboard to the light
	scene::IBillboardSceneNode* bill =
	    smgr->addBillboardSceneNode(light1, core::dimension2d<f32>(60, 60));

	bill->setMaterialFlag(video::EMF_LIGHTING, false);
	bill->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
	bill->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
	bill->setMaterialTexture(0, driver->getTexture(mediaPath + "particlegreen.jpg"));

	// add light 2 (red)
	scene::ISceneNode* light2 =
	    smgr->addLightSceneNode(0, core::vector3df(0, 0, 0),
	        video::SColorf(1.0f, 0.2f, 0.2f, 0.0f), 800.0f);

	// add fly circle animator to light 2
	anim = smgr->createFlyCircleAnimator(core::vector3df(0, 150, 0), 200.0f,
	        0.001f, core::vector3df(0.2f, 0.9f, 0.f));
	light2->addAnimator(anim);
	anim->drop();

	// attach billboard to light
	bill = smgr->addBillboardSceneNode(light2, core::dimension2d<f32>(120, 120));
	bill->setMaterialFlag(video::EMF_LIGHTING, false);
	bill->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
	bill->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
	bill->setMaterialTexture(0, driver->getTexture(mediaPath + "particlered.bmp"));

	// add particle system
	scene::IParticleSystemSceneNode* ps =
	    smgr->addParticleSystemSceneNode(false, light2);

	// create and set emitter
	scene::IParticleEmitter* em = ps->createBoxEmitter(
	        core::aabbox3d<f32>(-3, 0, -3, 3, 1, 3),
	        core::vector3df(0.0f, 0.03f, 0.0f),
	        80, 100,
	        video::SColor(10, 255, 255, 255), video::SColor(10, 255, 255, 255),
	        400, 1100);
	em->setMinStartSize(core::dimension2d<f32>(30.0f, 40.0f));
	em->setMaxStartSize(core::dimension2d<f32>(30.0f, 40.0f));

	ps->setEmitter(em);
	em->drop();

	// create and set affector
	scene::IParticleAffector* paf = ps->createFadeOutParticleAffector();
	ps->addAffector(paf);
	paf->drop();

	// adjust some material settings
	ps->setMaterialFlag(video::EMF_LIGHTING, false);
	ps->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
	ps->setMaterialTexture(0, driver->getTexture(mediaPath + "fireball.bmp"));
	ps->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);

	/** set epmty fuctions pointer */
	init = &IrrlichtQuickItem::_empty_init;
}

void IrrlichtQuickItem:: _load_example_16()
{
	if (!m_device)
		return;

	io::path mediaPath = getExampleMediaPath();

	//! add our private media directory to the file system
	//m_device->getFileSystem()->addFileArchive(mediaPath);

	/*
		To display the Quake 3 map, we first need to load it. Quake 3 maps
		are packed into .pk3 files, which are nothing other than .zip files.
		So we add the .pk3 file to our FileSystem. After it was added,
		we are able to read from the files in that archive as they would
		directly be stored on disk.
		*/
	//:/irrlicht/media/map-20kdm2.pk3
	mediaPath = ":/irrlicht/media/";
	m_device->getFileSystem()->addFileArchive(mediaPath + "map-20kdm2.pk3");


	// Quake3 Shader controls Z-Writing
	m_scene->getParameters()->setAttribute(scene::ALLOW_ZWRITE_ON_TRANSPARENT, true);

	/*
		Now we can load the mesh by calling getMesh(). We get a pointer returned
		to a IAnimatedMesh. As you know, Quake 3 maps are not really animated,
		they are only a huge chunk of static geometry with some materials
		attached. Hence the IAnimated mesh consists of only one frame,
		so we get the "first frame" of the "animation", which is our quake level
		and create an Octree scene node with it, using addOctreeSceneNode().
		The Octree optimizes the scene a little bit, trying to draw only geometry
		which is currently visible. An alternative to the Octree would be a
		AnimatedMeshSceneNode, which would draw always the complete geometry of
		the mesh, without optimization. Try it out: Write addAnimatedMeshSceneNode
		instead of addOctreeSceneNode and compare the primitives drawn by the
		video driver. (There is a getPrimitiveCountDrawed() method in the
		IVideoDriver class). Note that this optimization with the Octree is only
		useful when drawing huge meshes consisting of lots of geometry.
		*/
	scene::IQ3LevelMesh* const mesh =
	    (scene::IQ3LevelMesh*) m_scene->getMesh("maps/20kdm2.bsp");

	/*
			add the geometry mesh to the Scene ( polygon & patches )
			The Geometry mesh is optimised for faster drawing
		*/
	scene::ISceneNode* node = 0;
	if (mesh)
	{
		scene::IMesh * const geometry = mesh->getMesh(quake3::E_Q3_MESH_GEOMETRY);
		node = m_scene->addOctreeSceneNode(geometry, 0, -1, 4096);
	}

	// create an event receiver for making screenshots
//	CScreenShotFactory screenshotFactory(m_device, mapname, node);
//	device->setEventReceiver(&screenshotFactory);

	/*
			now construct SceneNodes for each Shader
			The Objects are stored in the quake mesh scene::E_Q3_MESH_ITEMS
			and the Shader ID is stored in the MaterialParameters
			mostly dark looking skulls and moving lava.. or green flashing tubes?
		*/
	if ( mesh )
	{
		// the additional mesh can be quite huge and is unoptimized
		const scene::IMesh * const additional_mesh = mesh->getMesh(quake3::E_Q3_MESH_ITEMS);

#ifdef SHOW_SHADER_NAME
		gui::IGUIFont *font = m_device->getGUIEnvironment()->getFont(mediaPath + "fontlucida.png");
		u32 count = 0;
#endif

		for ( u32 i = 0; i != additional_mesh->getMeshBufferCount(); ++i )
		{
			const IMeshBuffer* meshBuffer = additional_mesh->getMeshBuffer(i);
			const video::SMaterial& material = meshBuffer->getMaterial();

			// The ShaderIndex is stored in the material parameter
			const s32 shaderIndex = (s32) material.MaterialTypeParam2;

			// the meshbuffer can be rendered without additional support, or it has no shader
			const quake3::IShader *shader = mesh->getShader(shaderIndex);
			if (0 == shader)
			{
				continue;
			}

			// we can dump the shader to the console in its
			// original but already parsed layout in a pretty
			// printers way.. commented out, because the console
			// would be full...
			// quake3::dumpShader ( Shader );

			node = m_scene->addQuake3SceneNode(meshBuffer, shader);

#ifdef SHOW_SHADER_NAME
			count += 1;
			core::stringw name( node->getName() );
			node = m_scene->addBillboardTextSceneNode(
			        font, name.c_str(), node,
			        core::dimension2d<f32>(80.0f, 8.0f),
			        core::vector3df(0, 10, 0));
#endif
		}
	}

	/*
		Now we only need a Camera to look at the Quake 3 map. And we want to
		create a user controlled camera. There are some different cameras
		available in the Irrlicht engine. For example the Maya Camera which can
		be controlled comparable to the camera in Maya: Rotate with left mouse
		button pressed, Zoom with both buttons pressed, translate with right
		mouse button pressed. This could be created with
		addCameraSceneNodeMaya(). But for this example, we want to create a
		camera which behaves like the ones in first person shooter games (FPS).
		*/

	scene::ICameraSceneNode* viewCamera = m_scene->addCameraSceneNodeFPS(0, 1.0f);

	if ( mesh )
	{
		quake3::tQ3EntityList &entityList = mesh->getEntityList();

		quake3::IEntity search;
		search.name = "info_player_deathmatch";

		s32 index = entityList.binary_search(search);
		if (index >= 0)
		{
			s32 notEndList;
			do
			{
				const quake3::SVarGroup *group = entityList[index].getGroup(1);

				u32 parsepos = 0;
				const core::vector3df pos =
				    quake3::getAsVector3df(group->get("origin"), parsepos);

				parsepos = 0;
				const f32 angle = quake3::getAsFloat(group->get("angle"), parsepos);

				core::vector3df target(0.f, 0.f, 1.f);
				target.rotateXZBy(angle);

				//				viewCamera->setPosition(pos);
				//				camera->setTarget(pos + target);

				++index;
				/*
					notEndList = (	index < (s32) entityList.size () &&
									entityList[index].name == search.name &&
									(device->getTimer()->getRealTime() >> 3 ) & 1
								);
				*/
				notEndList = index == 2;
			}
			while ( notEndList );
		}
	}
	viewCamera->setPosition(core::vector3df(1300, 144, 1249));
	/*
		The mouse cursor needs not to be visible, so we make it invisible.
		*/

//	m_device->getCursorControl()->setVisible(false);

	// load the engine logo
	m_device->getGUIEnvironment()->addImage(m_driver->getTexture(mediaPath + "irrlichtlogo2.png"),
	    core::position2d<s32>(10, 10));

	// show the driver logo
	const core::position2di pos((s32)window()->geometry().width() - 128, (s32)window()->geometry().height() - 64);

	init = &IrrlichtQuickItem::_empty_init;
}

void IrrlichtQuickItem::_init_render_rt()
{
	dimension2du size(640, 480);
	m_color = m_driver->addRenderTargetTexture( size, "rt_color", ECF_A8R8G8B8 );
	m_normal = m_driver->addRenderTargetTexture(size, "rt_normal", ECF_R8G8B8 );
	m_depth = m_driver->addRenderTargetTexture( size, "rt_depth", ECF_D32 );

//	ICameraSceneNode *cam = m_scene->addCameraSceneNodeMaya();

	init = &IrrlichtQuickItem::_empty_init;
	render = &IrrlichtQuickItem::_render_rt;
}

void IrrlichtQuickItem::_standart_render()
{
	m_scene->drawAll();
	m_device->getGUIEnvironment()->drawAll();
}

void IrrlichtQuickItem::_render_rt()
{
	m_driver->setRenderTargetEx(m_rt, video::ECBF_COLOR | video::ECBF_DEPTH, video::SColor(0, 0, 0, 255));

	m_scene->drawAll();
	m_device->getGUIEnvironment()->drawAll();

	m_driver->setRenderTargetEx(nullptr, 0);

	if ( m_node )
		m_node->draw(m_driver);
}

void IrrlichtQuickItem::_render_direct()
{
	m_scene->drawAll();
	m_device->getGUIEnvironment()->drawAll();
}

void IrrlichtQuickItem::windowChangedSlot(QQuickWindow *window)
{
	if ( window != NULL )
		window->setClearBeforeRendering( false );
//	else if( m_device )
//		m_device->drop();
}

QSGNode *IrrlichtQuickItem::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *updatePaintNodeData)
{
	QSGNode* node = oldNode;
	if ( node == NULL )
		node = new QSGNode();

	QPointF pos = mapToScene( QPointF( 0.0f, 0.0f ) );
	pos.setY( window()->height() - height() - pos.y() );
	QRectF geometry = QRectF( pos.x(), pos.y(), width(), height() );

	QOpenGLFunctions* openGLFunctions = window()->openglContext()->functions();

	(this->*init)();

	if (!m_device)
		return node;

	((CQGLFunctionsDriver*)m_driver)->m_functions = openGLFunctions;

	if ( geometry != _geometry)
	{
		dimension2du size;
		size.Width = (u32)geometry.width();
		size.Height = (u32)geometry.height();
		_geometry = geometry;
		m_device->setWindowSize( size );
		m_driver->OnResize(size);

		// refresh render targets
		if (false)
		{
			m_color = m_driver->addRenderTargetTexture( size, "rt_color", ECF_A8R8G8B8 );
			m_depth = m_driver->addRenderTargetTexture( size, "rt_depth", ECF_D16 );
			m_rt->setTexture( m_color, m_depth );
			m_node->setMaterialTexture(0, m_color);
			m_color->drop();
			m_depth->drop();
		}

		if ( m_scene->getActiveCamera() )
		{
			if (m_orientation == 1 || m_orientation == 3)
				m_scene->getActiveCamera()->setAspectRatio( _geometry.height() / _geometry.width() );
			else
				m_scene->getActiveCamera()->setAspectRatio( _geometry.width() / _geometry.height() );
		}
	}
	else if ( m_prevOrientation != m_orientation )
	{
		m_prevOrientation = m_orientation ;
		dimension2du size;
		if ( m_scene->getActiveCamera() )
		{
			if (m_orientation == 1 || m_orientation == 3)
			{
				size.Width = (u32)geometry.height();
				size.Height = (u32)geometry.width();
				m_scene->getActiveCamera()->setAspectRatio( _geometry.height() / _geometry.width() );
			}
			else
			{
				size.Width = (u32)geometry.width();
				size.Height = (u32)geometry.height();
				m_scene->getActiveCamera()->setAspectRatio( _geometry.width() / _geometry.height() );
			}
			if (false)
			{
				m_color = m_driver->addRenderTargetTexture( size, "rt_color", ECF_A8R8G8B8 );
				m_depth = m_driver->addRenderTargetTexture( size, "rt_depth", ECF_D16 );
				m_rt->setTexture( m_color, m_depth );
				m_node->setMaterialTexture(0, m_color);
				m_color->drop();
				m_depth->drop();
			}
		}
	}
	irr::f32 current_delta  = (m_device->getTimer()->getRealTime() - m_lastTime) / 1000.0;
	if (m_scene->getActiveCamera())
	{
		qreal speed = m_speed/* * current_delta*/;
		ICameraSceneNode * camera = m_scene->getActiveCamera();
		core::vector3df relativeRotation = camera->getRotation();
		vector3df translate = vector3df(m_joyStick.x() * speed, 0, m_joyStick.y() * speed);
		core::matrix4 mat;
		mat.setRotationDegrees(core::vector3df(relativeRotation.X, relativeRotation.Y, 0));
		mat.transformVect(translate);
		camera->setPosition( camera->getPosition() + translate );
//		camera->set
	}
	m_device->run();

	m_driver->beginScene(true, true, SColor(255, 140, 140, 140) );
	QOpenGLContext::currentContext()->functions()->glUseProgram(0);
	m_driver->setMaterial(IdentityMaterial);
//	m_driver->get

	(this->*render)();

	m_driver->endScene();
	window()->resetOpenGLState();

	node->markDirty( QSGNode::DirtyForceUpdate );
	update();
	m_lastTime = m_device->getTimer()->getRealTime();
	return node;
}
