#include "CurrentSample.h"

#if CURRENT_SAMPLE == 2

#include <GP.h>

struct Vertex
{
	Vec2 position;
	Vec3 color;
};

class DrawIndexedRenderPass : public GP::RenderPass
{
public:

	virtual void Init(GP::GfxContext* context) override
	{
		// Data that we will upload to vertex buffer
		Vertex vertices[] = {
			{ Vec2(-0.8f, 0.8f), Vec3(0.05f,0.59f,0.83f) },
			{ Vec2(-0.8f, -0.8f), Vec3(0.05f,0.59f,0.83f) },
			{ Vec2(0.8f, -0.8f), Vec3(0.05f,0.59f,0.83f) },
			{ Vec2(0.8f, 0.8f), Vec3(0.05f,0.59f,0.83f) }
		};

		// Data that we will upload to index buffer
		// Default type for index buffer is unsigned int
		unsigned int indices[] = {
			0,1,2,0,2,3
		};

		// Best way to initialize vertex buffer that aren't going to change anymore is to pass the data as constructor arguments
		// First argument is pointer to a data, sencond argument is number of vertices
		m_VertexBuffer = new GP::GfxVertexBuffer<Vertex>(vertices, 4);

		// Best way to initialize vertex buffer that aren't going to change anymore is to pass the data as constructor arguments
		// First argument is pointer to a data, sencond argument is number of indices
		m_IndexBuffer = new GP::GfxIndexBuffer(indices, 6);
	}

	// All resource deinitialization should happen in the virtual desctuctor
	// This is executed only once after GP::Deinit is called
	virtual ~DrawIndexedRenderPass()
	{
		delete m_VertexBuffer;
		delete m_IndexBuffer;
	}

	// This is the main function where rendering is happening
	// This is executed once every frame
	virtual void Render(GP::GfxContext* context) override
	{
		// Clears the current render target ( screen in this case ) with the color specified in arguments
		// Default color is black
		context->Clear(Vec4(0.05f, 0.05f, 0.05f, 1.0f));

		// Bind shader to the pipeline
		context->BindShader(&m_BasicShader);

		// Bind vertex buffer to the pipeline
		context->BindVertexBuffer(m_VertexBuffer);

		// Bind index buffer to the pipeline
		context->BindIndexBuffer(m_IndexBuffer);

		// Draw 6 indices
		context->DrawIndexed(6);
	}

private:
	GP::GfxShader m_BasicShader{ "samples/shaders/sample2/basic_shader.hlsl" };
	GP::GfxVertexBuffer<Vertex>* m_VertexBuffer;

	GP::GfxIndexBuffer* m_IndexBuffer;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
	GP::Init(hInstance, 1024, 768, "Demo");

	GP::AddRenderPass(new DrawIndexedRenderPass());

	GP::Run();

	GP::Deinit();
}

#endif // CURRENT_SAMPLE

