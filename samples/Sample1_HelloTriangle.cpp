#include "CurrentSample.h"

#if CURRENT_SAMPLE == 1

#include <GP.h>

// We are making struct so we can store information into vertex buffer
struct Vertex
{
	// Vec2 is equivalent to float2 in hlsl, we will use this as position on screen
	Vec2 position;

	// Vec3 is equivalent to float3 in hlsl, we will use this as vertex color
	Vec3 color;
};

// Every renderpass must inherit GP::RenderPass and override functions that we want to make
class BasicRenderPass : public GP::RenderPass
{
public:

	// This function is called only once, in initialization phase of the renderer
	// The purpose of this function is for resource initialization
	// This is the place where we should compute everything and fill the buffers
	virtual void Init(GP::GfxContext* context) override
	{
		// Data that we will upload to vertex buffer
		Vertex vertices[] = {
			{ Vec2(0.0f, 1.0f), Vec3(1.0f,0.0f,0.0f) },
			{ Vec2(-1.0f, -1.0f), Vec3(0.0f, 1.0f, 0.0f) },
			{ Vec2(1.0f, -1.0f), Vec3(0.0f, 0.0f, 1.0f) }
		};

		// Best way to initialize vertex buffer that aren't going to change anymore is to pass the data as constructor arguments
		// First argument is pointer to a data, sencond argument is number of vertices
		m_TriangleVertexBuffer = new GP::GfxVertexBuffer<Vertex>(vertices, 3);
	}

	// All resource deinitialization should happen in the virtual desctuctor
	// This is executed only once after GP::Deinit is called
	virtual ~BasicRenderPass()
	{
		delete m_TriangleVertexBuffer;
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
		context->BindVertexBuffer(m_TriangleVertexBuffer);

		// Draw 3 vertices
		context->Draw(3);
	}

private:
	// To load shader we just pass a path to a shader file in constructor
	GP::GfxShader m_BasicShader{ "samples/shaders/sample1/basic_shader.hlsl" };

	// To declare vertex buffer pass format as template argument
	GP::GfxVertexBuffer<Vertex>* m_TriangleVertexBuffer;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
	GP::Init(hInstance, 1024, 768, "Demo");

	// Before we call GP::Run we need to setup renderpass schedule

	// This will add BasicRenderPass to the render schedule
	// The passes are executed in the same order they are added
	// In this case we have just one render pass so only that render pass will be executed
	GP::AddRenderPass(new BasicRenderPass());

	GP::Run();

	GP::Deinit();
}

#endif // CURRENT_SAMPLE

