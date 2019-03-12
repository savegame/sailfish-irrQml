#ifndef SCREENNODE_H
#define SCREENNODE_H

#include <ISceneNode.h>
#include <S3DVertex.h>
#include <IVideoDriver.h>
#include "ScreenShaderCB.h"

using namespace irr;

class ScreenNode : public scene::ISceneNode
{
public:
	ScreenNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id);

	~ScreenNode()
	{
		m_shader->drop();
	}

	ScreenShaderCB* getShader() const { return m_shader; }

	virtual void OnRegisterSceneNode() override;

	virtual void render() override;

	void draw(video::IVideoDriver* driver);

	virtual const core::aabbox3d<f32>& getBoundingBox() const override;

	virtual u32 getMaterialCount() const override;

	virtual video::SMaterial& getMaterial(u32 i) override;

	void setOrientation(int o);
protected:
	ScreenShaderCB*  m_shader;
	core::aabbox3d<f32> Box;
	video::S3DVertex Vertices[4];
	video::SMaterial Material;
	s32 ShaderMaterial;
	u16 m_indices[6];

	int m_prevOrientation;
};
#endif // SCREENNODE_H
