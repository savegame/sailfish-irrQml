#include "ScreenNode.h"
#include <irrlicht.h>

using namespace irr;
using namespace video;
using namespace core;

ScreenNode::ScreenNode(scene::ISceneNode *parent, scene::ISceneManager *mgr, s32 id)
	: scene::ISceneNode(parent, mgr, id)
	, m_shader(NULL)
{
	bool UseHighLevelShaders = true;
	Material.Wireframe = false;
	Material.Lighting = false;
	Material.Thickness = 0.f;
	Vertices[0] = video::S3DVertex(-1, -1.0, 0,   5, 1, 0,
	        video::SColor(255, 0, 255, 255), 0, 1);
	Vertices[1] = video::S3DVertex(-1, 1, 0,   10, 0, 0,
	        video::SColor(255, 255, 0, 255), 0, 0);
	Vertices[2] = video::S3DVertex(1, 1, 0,   20, 1, 1,
	        video::SColor(255, 255, 255, 0), 1, 0);
	Vertices[3] = video::S3DVertex(1, -1, 0,   40, 0, 1,
	        video::SColor(255, 0, 255, 0), 1, 1);
	Vertices[0].TCoords = position2df(1, 1);
	Vertices[0].Pos = vector3df(-1, -1, -1);

	Vertices[1].TCoords = position2df(1, 0);
	Vertices[1].Pos = vector3df(-1, 1, -1);

	Vertices[2].TCoords = position2df(0, 0);
	Vertices[2].Pos = vector3df(1, 1, -1);

	Vertices[3].TCoords = position2df(0, 1);
	Vertices[3].Pos = vector3df(1, -1, -1);

	Box.reset(Vertices[0].Pos);
	for (s32 i = 1; i < 4; ++i)
		Box.addInternalPoint(Vertices[i].Pos);
	//shaders
#if !defined(_DEBUG) || defined(_IRR_COMPILE_WITH_X11_DEVICE_)
	io::path mediaPath = _MEDIA_PATH;
	mediaPath = ":/irrlicht/media/";
#elif defined(_IRR_COMPILE_WITH_SAILFISH_DEVICE_)
	io::path mediaPath = getExampleMediaPath();// "/home/src1/OpenGL/harbour-irrlicht/irrlicht/media/";
#else
	io::path mediaPath = getExampleMediaPath();
#endif
	io::path psFileName = mediaPath + "Shaders/DFGLES2Screen.fsh";
	io::path vsFileName = mediaPath + "Shaders/DFGLES2Screen.vsh";
#if defined(_OUT_PWD_PATH) && !defined(_IRR_COMPILE_WITH_SAILFISH_DEVICE_)
	psFileName = io::path(_OUT_PWD_PATH) + io::path("/") + psFileName;
	vsFileName = io::path(_OUT_PWD_PATH) + io::path("/") + vsFileName;
#endif
	video::IGPUProgrammingServices* gpu = mgr->getVideoDriver()->getGPUProgrammingServices();
	ShaderMaterial = 0;

	if (gpu)
	{
		m_shader = new ScreenShaderCB();

		if (true)
		{
			// Choose the desired shader type. Default is the native
			// shader type for the driver
			const video::E_GPU_SHADING_LANGUAGE shadingLanguage = video::EGSL_DEFAULT;

			// create material from high level shaders (hlsl, glsl)
//			irr::os::Printer::log( "Loading shaders: ", irr::ELL_DEBUG );
			core::stringc m = "  ";
			m += mediaPath;
//			irr::os::Printer::log( m.c_str(), irr::ELL_DEBUG );

			ShaderMaterial = gpu->addHighLevelShaderMaterialFromFiles(
			        vsFileName, "main", video::EVST_VS_1_1,
			        psFileName, "main", video::EPST_PS_1_1,
			        m_shader, video::EMT_SOLID, 0, shadingLanguage);
			Material.MaterialType = ((video::E_MATERIAL_TYPE)ShaderMaterial);
		}
	}
	u16 indices[] = {0, 1, 2, 0, 2, 3, };
	memcpy(m_indices, indices, 6 * sizeof(u16));
}

void ScreenNode::OnRegisterSceneNode()
{
	if (IsVisible)
		SceneManager->registerNodeForRendering(this);

	ISceneNode::OnRegisterSceneNode();
}

void ScreenNode::render()
{
	u16 indices[] = {	0, 1, 2, 0, 2, 3, 2, 1, 0, 3, 2, 0	};
	video::IVideoDriver* driver = SceneManager->getVideoDriver();

	driver->setMaterial(Material);
	driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
	driver->drawVertexPrimitiveList(&Vertices[0], 4, &indices[0], 2, video::EVT_STANDARD, scene::EPT_TRIANGLES, video::EIT_16BIT);
}

void ScreenNode::draw(video::IVideoDriver *driver)
{
	driver->setMaterial(video::SMaterial());
	driver->setTransform ( video::ETS_PROJECTION, core::IdentityMatrix );
	driver->setTransform ( video::ETS_VIEW, core::IdentityMatrix );
	driver->setTransform ( video::ETS_WORLD, core::IdentityMatrix );

	u16 indices[] = {	0, 1, 2, 0, 2, 3 };
	driver->setMaterial(Material);
	driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
	driver->drawVertexPrimitiveList(Vertices, 4, m_indices, 2, video::EVT_STANDARD, scene::EPT_TRIANGLES, video::EIT_16BIT);
}

const core::aabbox3d<f32> &ScreenNode::getBoundingBox() const
{
	return Box;
}

u32 ScreenNode::getMaterialCount() const
{
	return 1;
}

SMaterial &ScreenNode::getMaterial(u32 i)
{
	return Material;
}

void ScreenNode::setOrientation(int o)
{
	/**
	 * @brief m_orientation
	 * 0 - noraml
	 * 1 - 90  degrees
	 * 2 - 180 degrees
	 * 3 - 270 degrees
	 */
	if (m_shader)
		m_shader->setOrientation(o);
}
