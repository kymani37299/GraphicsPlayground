#include "CurrentSample.h"

// Similar example as Sample2 but instead of color we are using texture for the quad

#if CURRENT_SAMPLE == 5

#include <GP.h>

struct Vertex
{
	Vec2 position;
	Vec2 texcoord;
};

class DrawIndexedRenderPass : public GP::RenderPass
{
public:

	virtual void Init(GP::GfxContext* context) override
	{
		// Vertex data, positions and texture coordiantes
		Vertex vertices[] = {
			{ Vec2(-0.8f, 0.8f), Vec2(0.0f, 0.0f) },
			{ Vec2(-0.8f, -0.8f), Vec2(0.0f, 1.0f) },
			{ Vec2(0.8f, -0.8f), Vec2(1.0f, 1.0f) },
			{ Vec2(0.8f, 0.8f), Vec2(1.0f, 0.0f) }
		};

		// Indices
		unsigned int indices[] = {
			0,1,2,0,2,3
		};

		m_VertexBuffer = new GP::GfxVertexBuffer<Vertex>(vertices, 4);
		m_IndexBuffer = new GP::GfxIndexBuffer(indices, 6);
	}

	virtual ~DrawIndexedRenderPass()
	{
		delete m_VertexBuffer;
		delete m_IndexBuffer;
	}

	virtual void Render(GP::GfxContext* context) override
	{
		context->Clear(Vec4(0.05f, 0.05f, 0.05f, 1.0f));

		context->BindShader(&m_BasicShader);
		context->BindVertexBuffer(m_VertexBuffer);
		context->BindIndexBuffer(m_IndexBuffer);

		// Bind texture
		// 
		// First parameter: Shader stage we passed GP::PS because we want to use it for pixel shader.
		// If we wanted to use it for a vertex shader we would pass GP::VS, 
		// Also you can combine flagsfor example GP::VS | GP::PS will use this buffer both in Vertex and Pixel shader
		// There are flags for every shader type, so GP::CS (Compute) , GP::GS (Geometry), GP::DS (Domain), GP::HS (Hull)
		// 
		// Second parameter: Pointer to a texture
		//
		// Third parameter: Texture slot we are binding this constant buffer
		// This should match the register(tN) you are using in the shader. 
		// For this example we are using register(t0) so we are binding it to slot 0
		context->BindTexture2D(GP::PS, &m_ExampleTexture, 0);

		context->DrawIndexed(6);

		// Unbind the resources from the pipeline
		context->BindVertexBuffer(nullptr);
		context->BindIndexBuffer(nullptr);
		context->UnbindTexture(GP::PS, 0);
	}

private:
	GP::GfxShader m_BasicShader{ "samples/shaders/sample5/texture_shader.hlsl" };
	GP::GfxVertexBuffer<Vertex>* m_VertexBuffer;
	GP::GfxIndexBuffer* m_IndexBuffer;

	// To load a texture just pass a path to a constructor
	GP::GfxTexture2D m_ExampleTexture{ "samples/resources/example.jpg" };
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
	GP::Init(hInstance, 1024, 768, "Demo");

	GP::AddRenderPass(new DrawIndexedRenderPass());

	GP::Run();

	GP::Deinit();
}

#endif // CURRENT_SAMPLE

