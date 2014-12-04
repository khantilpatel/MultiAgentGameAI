////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "graphicsclass.h"


GraphicsClass::GraphicsClass()
{
	m_D3D = 0;
	m_Camera = 0;
	m_Model = 0;
	m_TextureShader = 0;
	m_SimpleShader =0;

	m_ParticleShader = 0;
	m_ParticleSystem = 0;

	m_FireParticleShader=0;
	m_RainParticleSystem = 0;

	m_skyBox = 0;

	m_AStar_Type1_ShaderClass = 0;
	m_AStar_Type2_ShaderClass = 0;

	m_MultiAgentDrawClass = 0;
	toggle_WireFrame_Mode = false;
	executeOnceAStar_Type1 = true;

	frame_count = 0;

}


GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
	m_ParticleShader = 0;
	m_ParticleSystem = 0;
}


GraphicsClass::~GraphicsClass()
{
}
void GraphicsClass::toggle_RasterState_WireFrame()
{
	if (toggle_WireFrame_Mode)
	{
		m_D3D->ToggleRasterState_WireFrame(false);
		toggle_WireFrame_Mode = false;
	}
	else
	{
		m_D3D->ToggleRasterState_WireFrame(true);
		toggle_WireFrame_Mode = true;
	}
	
}

void GraphicsClass::toggle_RestartPathFinding(){
	
	executeOnceAStar_Type1 = true;

}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;


	// Create the Direct3D object.
	m_D3D = new D3DClass;
	if(!m_D3D)
	{
		return false;
	}

	// Initialize the Direct3D object.
	result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	// Create the camera object.
	m_Camera = new CameraClass;
	if(!m_Camera)
	{
		return false;
	}

	m_skyBox = new SkyBox ;
		if(!m_skyBox)
	{
		return false;
	}

	result = m_skyBox->Initiaize(m_D3D->GetDevice(), hwnd);
			if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the skybox object.", L"Error", MB_OK);
		return false;
	}

	m_AStar_Type1_ShaderClass = new AStar_Type1_ShaderClass;
	result = m_AStar_Type1_ShaderClass->Initialize(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the m_RainParticleSystem  object", L"Error", MB_OK);
		return false;
	}

	m_AStar_Type2_ShaderClass = new AStar_Type2_ShaderClass;
	result = m_AStar_Type2_ShaderClass->Initialize(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), hwnd);
	if (!result)
	{

		MessageBox(hwnd, L"Could not initialize the m_RainParticleSystem  object", L"Error", MB_OK);
		return false;
	}

	

	m_MultiAgentDrawClass = new MultiAgentDrawClass;
	result = m_MultiAgentDrawClass->Initialize(m_D3D->GetDevice(), m_D3D->GetDeviceContext(),hwnd);
	if (!result)
	{

		MessageBox(hwnd, L"Could not initialize the m_RainParticleSystem  object", L"Error", MB_OK);
		return false;
	}
	return true;
}


bool GraphicsClass::UpdateFrame(float frameTime, float totalTime, float posX, float posY, float posZ, float rotX, float rotY, float rotZ)
{
	bool result;
	m_frameTime = frameTime;
	m_TotalTime = totalTime;

	m_Camera->SetPosition(posX, posY, posZ);
	m_Camera->SetRotation(rotX, rotY, rotZ);

		// Run the frame processing for the particle system.
	//m_ParticleSystem->UpdateFrame((frameTime), m_D3D->GetDeviceContext());

	// Redsfnder the graphics scene.
	result = Render();

	if(!result)
	{
		return false;
	}

	return true;
}

int counter = 0;
bool GraphicsClass::Render()
{
	D3DXMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result =true;


	// Clear the buffers to begin the scene.
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);


	/////////////////////////////////////////////////////////////////////
	// Render Diamonds Particle System
	//m_D3D->EnableAlphaBlending();

	//	// Put the particle system vertex and index buffers on the graphics pipeline to prepare them for drawing.
	//m_ParticleSystem->Render(m_D3D->GetDeviceContext());

	//// Render the model using the texture shader.
	//result = m_ParticleShader->Render(m_D3D->GetDeviceContext(), m_ParticleSystem->GetIndexCount(), 
	//	worldMatrix, viewMatrix, projectionMatrix, m_ParticleSystem->GetTexture());
	//if(!result)
	//{
	//	return false;
	//}	
	//m_D3D->DisableAlphaBlending();
	///////////////////////////////////////////////////////////////////
	// RENDER SKYBOX
	/////////////////////////////////////////////////////////////////////
	m_D3D->SetDepthStencilState_Less_Equal();
	m_D3D->SetRasterState_Nocull();

	m_skyBox->RenderShader(m_D3D->GetDeviceContext(),	worldMatrix, viewMatrix, 
	projectionMatrix, m_Camera->GetPosition_XM());

	m_D3D->EnableDepthStencilState();
	m_D3D->SetRasterState_Default();
	/////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////
	// RENDER FIRE PARTICLES
	/////////////////////////////////////////////////////////////////////
	// Render Fire Particle System
	//m_FireParticleShader->setEmit_Position(XMFLOAT3(-0.26f, -0.6f, -10.0f)); 	
	//m_D3D->DisableDepthStencilState();
	//m_FireParticleShader->Render(m_D3D->GetDeviceContext(),	worldMatrix, viewMatrix, 
	//projectionMatrix, m_frameTime/1000, m_TotalTime/1000, m_Camera->GetPosition_XM());
	//m_D3D->EnableAdditiveBlending();
	//m_D3D->NoDepthWriteStencilState();
	//m_D3D->EnableAlphaBlending();
	//m_FireParticleShader->RenderShader_Draw(m_D3D->GetDeviceContext());
	//m_D3D->EnableDepthStencilState();
	/////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////
	// RENDER RAIN PARTICLES
	/////////////////////////////////////////////////////////////////////
	//m_RainParticleSystem->setEmit_Position(m_Camera->GetPosition_XM());
	//m_RainParticleSystem->Render(m_D3D->GetDeviceContext(),	worldMatrix, viewMatrix, 
	//projectionMatrix, m_frameTime/1000, m_TotalTime/1000, m_Camera->GetPosition_XM());
	//m_RainParticleSystem->RenderShader_Draw(m_D3D->GetDeviceContext());
	/////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////
	// AI A*- Type-1 RENDERING
	/////////////////////////////////////////////////////////////////////
	

	 if (executeOnceAStar_Type1){
		// m_AStar_Type1_ShaderClass->Initialize(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), nullptr);
		// m_MultiAgentDrawClass->Initialize(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), nullptr);

		 m_AStar_Type1_ShaderClass->Render(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), 0, 0, NULL, NULL);
		 executeOnceAStar_Type1 = false;
		
	 }
	
	/*int  *agentRenderPathList;
	AStar_Type1_ShaderClass::AgentRender *agentRenderList;
	agentRenderList = m_AStar_Type1_ShaderClass->agentRenderList;
	agentRenderPathList = m_AStar_Type1_ShaderClass->agentRenderPathList;*/

	/////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////
	// RENDER Multi Agent Display
	/////////////////////////////////////////////////////////////////////
	m_MultiAgentDrawClass->Render(m_D3D->GetDevice(),m_D3D->GetDeviceContext(), worldMatrix, viewMatrix,
		projectionMatrix, m_frameTime / 1000, m_TotalTime / 1000, frame_count, m_Camera->GetPosition_XM(),
	m_AStar_Type1_ShaderClass->m_BufRenderAgentList_URV, m_AStar_Type1_ShaderClass->m_BufRenderAgentPathList_URV);

		///////////////////////////////////////////////////////////////////////////////
		// Debug Agent List Data
		//ComputeShaderHelperClass* m_computeshader_helper = new ComputeShaderHelperClass;
		//ID3D11Buffer* debugbuf0 = m_computeshader_helper->CreateAndCopyToDebugBuf(m_D3D->GetDevice(), m_D3D->GetDeviceContext(),
		//	m_AStar_Type1_ShaderClass->m_Buffer_RenderAgentList);
		//D3D11_MAPPED_SUBRESOURCE MappedResource1;
		//AStar_Type1_ShaderClass::AgentRender  *agentListCheck;
		//m_D3D->GetDeviceContext()->Map(debugbuf0, 0, D3D11_MAP_READ, 0, &MappedResource1);

		//// Set a break point here and put down the expression "p, 1024" in your watch window to see what has been written out by our CS
		//// This is also a common trick to debug CS programs.
		//agentListCheck = (AStar_Type1_ShaderClass::AgentRender*)MappedResource1.pData;

		//cout << " Render Agent List check:\n";
		//const int count = 4096;
		//AStar_Type1_ShaderClass::AgentRender nodes2[count];
		//for (int i = 2001; i < count; i++)
		//{
		//	nodes2[i] = agentListCheck[i];
		//	cout <<  "\nAgentId=" << agentListCheck[i].agentId << ":Counter= " << agentListCheck[i].currentInterpolationId
		//		<< " frameTime:"<< agentListCheck[i].u;
		//}
		//m_D3D->GetDeviceContext()->Unmap(debugbuf0, 0);

		//debugbuf0->Release();
		//debugbuf0 = 0;
		////////////////////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////////////////////////
		//// Debug PathList Data
		//ID3D11Buffer* debugbuf1 = m_computeshader_helper->CreateAndCopyToDebugBuf(m_D3D->GetDevice(), m_D3D->GetDeviceContext(),
		//	m_AStar_Type1_ShaderClass->m_Buffer_RenderAgentPathList);
		//D3D11_MAPPED_SUBRESOURCE MappedResource2;
		//int  *pathListCheck;
		//m_D3D->GetDeviceContext()->Map(debugbuf1, 0, D3D11_MAP_READ, 0, &MappedResource2);

		//// Set a break point here and put down the expression "p, 1024" in your watch window to see what has been written out by our CS
		//// This is also a common trick to debug CS programs.
		//pathListCheck = (int*)MappedResource2.pData;

		//cout << " Render Agent List check:\n";

		//int nodes1[2000];
		//for (int i = 0; i < 2000; i++)
		//{
		//	nodes1[i] = pathListCheck[i];
		//	cout <<"\n"<< i << ". Node =" << pathListCheck[i];
		//}
		//m_D3D->GetDeviceContext()->Unmap(debugbuf1, 0);

		//debugbuf1->Release();
		//debugbuf1 = 0;
		//////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////
	// AI A*- Type-2 RENDERING
	/////////////////////////////////////////////////////////////////////
	//m_AStar_Type2_ShaderClass->Render(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), 0, 0, NULL, NULL);
	/////////////////////////////////////////////////////////////////////
		// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	//m_Model->Render(m_D3D->GetDeviceContext());



	// Render the model using the texture shader.
	///result = m_TextureShader-> Render(m_D3D->GetDeviceContext(), m_Model->GetVertexCount(), m_Model->GetInstanceCount(),
	//	worldMatrix, viewMatrix, projectionMatrix,m_Model->getVertexBuffer(), m_Model->getInstanceBuffer(), 
	//	m_Model->getStreamOutBuffer(), m_Model->GetTexture());

	//m_SimpleShader->Render(m_D3D->GetDeviceContext(), m_Model->GetVertexCount(), m_Model->GetInstanceCount(),
	//		m_FireParticleShader->m_DrawBuffer, m_Model->GetTexture());
	//if(!result)
	//{
	//	return false;
	//}

	// Present the rendered scene to the screen.
	m_D3D->EndScene();
	frame_count++;
	return true;
}


void GraphicsClass::Shutdown()
{

	// Release the camera object.
	if(m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	// Release the D3D object.
	if(m_D3D)
	{
		m_D3D->Shutdown();
		delete m_D3D;
		m_D3D = 0;
	}

	if (m_AStar_Type1_ShaderClass)
	{
		m_AStar_Type1_ShaderClass->Shutdown();
		delete m_AStar_Type1_ShaderClass;
		m_AStar_Type1_ShaderClass = 0;
	}

	if (m_AStar_Type2_ShaderClass)
	{
		m_AStar_Type2_ShaderClass->Shutdown();
		delete m_AStar_Type2_ShaderClass;
		m_AStar_Type2_ShaderClass = 0;
	}
	
	if (m_skyBox)
	{
		m_skyBox->Shutdown();
		delete m_skyBox;
		m_skyBox = 0;
	}

	if (m_MultiAgentDrawClass)
	{
		m_MultiAgentDrawClass->Shutdown();
		delete m_MultiAgentDrawClass;
		m_MultiAgentDrawClass = 0;
	}

	return;
}

