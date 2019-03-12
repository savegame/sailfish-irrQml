#ifndef SCREENSHADERCB_H
#define SCREENSHADERCB_H

#include <IShaderConstantSetCallBack.h>
#include <vector2d.h>
#include <dimension2d.h>

using namespace irr;

class ScreenShaderCB : public video::IShaderConstantSetCallBack
{
public:
	enum ScreenOrientation : int
	{
		Normal    = 0,
		Rotate90  = 1,
		Rotate180  = 2,
		Rotate270 = 3
	};
public:
	ScreenShaderCB();

	virtual void OnSetConstants(video::IMaterialRendererServices* services,
	    s32 userData);

	void setOrientation(s32 val);

private:
	s32 WorldViewProjID;
//	s32 TransWorldID;
//	s32 InvWorldID;
//	s32 PositionID;
//	s32 ColorID;
	s32 TextureID0;
	s32 TextureID1;
	s32 OrientationID;
	s32 ResolutionID;
	s32 DepthNearID;
	s32 DepthFarID;
	s32 IsUseDepthID;

public:
	s32  m_screenOrientation;
	core::vector2df  m_depth_near;
	core::vector2df  m_depth_far;
	core::dimension2df m_resolution;
	bool FirstUpdate;
};

#endif // SCREENSHADERCB_H
