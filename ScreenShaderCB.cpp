#include "ScreenShaderCB.h"

#include <irrlicht.h>

ScreenShaderCB::ScreenShaderCB()
	: WorldViewProjID(-1),
	  //	    TransWorldID(-1),
	  //	    InvWorldID(-1),
	  //	    PositionID(-1),
	  //	    ColorID(-1),
	  TextureID0(-1), TextureID1(-1), FirstUpdate(true),
#ifdef _IRR_COMPILE_WITH_SAILFISH_DEVICE_
	  m_screenOrientation(Rotate270),
#else
	  m_screenOrientation(Normal),
#endif
	  OrientationID(-1),
	  ResolutionID(-1)
{
	m_resolution = core::dimension2df(640.0, 480.0);
	m_depth_far = core::vector2df(0.999f, 0.95f);
	m_depth_near = core::vector2df(0.005f, 0.01f);
}

void ScreenShaderCB::OnSetConstants(video::IMaterialRendererServices *services, s32 userData)
{
	video::IVideoDriver* driver = services->getVideoDriver();

	// get shader constants id.

	if (FirstUpdate)
	{
		WorldViewProjID = services->getVertexShaderConstantID("mWorldViewProj");
		TextureID0 = services->getPixelShaderConstantID("Texture0");
		TextureID1 = services->getPixelShaderConstantID("Texture1");
		OrientationID = services->getPixelShaderConstantID("inScreenOrientation");
		ResolutionID = services->getPixelShaderConstantID("inResolution");
		DepthNearID = services->getPixelShaderConstantID("inDepthNear");
		DepthFarID = services->getPixelShaderConstantID("inDepthFar");
		IsUseDepthID  = services->getPixelShaderConstantID("inIsUseDepth");
		FirstUpdate = false;
	}

	core::matrix4 invWorld = driver->getTransform(video::ETS_WORLD);
	invWorld.makeInverse();

	core::matrix4 worldViewProj;
	worldViewProj = driver->getTransform(video::ETS_PROJECTION);
	worldViewProj *= driver->getTransform(video::ETS_VIEW);
	worldViewProj *= driver->getTransform(video::ETS_WORLD);

	services->setVertexShaderConstant(WorldViewProjID, worldViewProj.pointer(), 16);

	video::SColorf col(0.0f, 1.0f, 1.0f, 0.0f);
	core::matrix4 world = driver->getTransform(video::ETS_WORLD);
	world = world.getTransposed();

	s32 TextureLayerID = 0;
	services->setPixelShaderConstant(TextureID0, &TextureLayerID, 1);
	s32 TextureLayerID1 = 1;
	s32 d = 1;
	services->setPixelShaderConstant(TextureID1, &TextureLayerID1, 1);
	services->setPixelShaderConstant(OrientationID, &m_screenOrientation, 1);
	services->setPixelShaderConstant(IsUseDepthID, (s32*)(&d), 1);
	services->setPixelShaderConstant(ResolutionID, reinterpret_cast<f32*>(&m_resolution), 2);
	services->setPixelShaderConstant(DepthNearID, reinterpret_cast<f32*>(&m_depth_near), 2);
	services->setPixelShaderConstant(DepthFarID, reinterpret_cast<f32*>(&m_depth_far), 2);
}

void ScreenShaderCB::setOrientation(s32 val)
{
	m_screenOrientation = val;
}


