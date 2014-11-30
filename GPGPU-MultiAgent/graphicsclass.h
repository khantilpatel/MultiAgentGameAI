////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "textureshaderclass.h"
#include "simpleshaderclass.h"

#include "particleshaderclass.h"
#include "particlesystemclass.h"
#include "FireParticalSystemShader.h"
#include "RainParticleSystem.h"
#include "SkyBox.h"
#include "AStar_Type1_ShaderClass.h"
#include "AStar_Type2_ShaderClass.h"
#include "MultiAgentDrawClass.h"
/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;


////////////////////////////////////////////////////////////////////////////////
// Class name: GraphicsClass
////////////////////////////////////////////////////////////////////////////////
class GraphicsClass
{
public:
	GraphicsClass();
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool UpdateFrame(float,float, float, float, float, float, float, float);
	void toggle_RasterState_WireFrame();
	void toggle_RestartPathFinding();
	float m_frameTime;
	float m_TotalTime;
	bool toggle_WireFrame_Mode;
	bool executeOnceAStar_Type1;
private:
	bool Render();

private:
	D3DClass* m_D3D;
	CameraClass* m_Camera;
	ModelClass* m_Model;
	TextureShaderClass* m_TextureShader;
	SimpleShaderClass* m_SimpleShader;

	ParticleShaderClass* m_ParticleShader;
	ParticleSystemClass* m_ParticleSystem;

	FireParticalSystemShader* m_FireParticleShader;
	RainParticleSystem* m_RainParticleSystem;

	SkyBox* m_skyBox;

	AStar_Type1_ShaderClass* m_AStar_Type1_ShaderClass;
	AStar_Type2_ShaderClass* m_AStar_Type2_ShaderClass;

	MultiAgentDrawClass* m_MultiAgentDrawClass;
};

#endif