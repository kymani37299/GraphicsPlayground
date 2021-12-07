#include "CurrentSample.h"

#if CURRENT_SAMPLE == 3

#include <GP.h>

class BasicRenderPass : public GP::RenderPass
{
public:

	virtual void Init(GP::GfxContext* context) override
	{
		Vec2 positionData[] = {
			Vec2(0.0f, 1.0f), Vec2(-1.0f, -1.0f), Vec2(1.0f, -1.0f)
		};

		Vec3 colorData[] = {
			Vec3(1.0f,0.0f,0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)
		};

		m_PositionVertexBuffer = new GP::GfxVertexBuffer<Vec2>(positionData, 3);
		m_ColorVertexBuffer = new GP::GfxVertexBuffer<Vec3>(colorData, 3);
	}

	virtual ~BasicRenderPass()
	{
		delete m_PositionVertexBuffer;
		delete m_ColorVertexBuffer;
	}

	virtual void Render(GP::GfxContext* context) override
	{
		context->Clear(Vec4(0.05f, 0.05f, 0.05f, 1.0f));

		context->BindShader(&m_BasicShader);

		// When binding multiple buffer slots we must respect the oreder how VS_Input is declared in hlsl
		// When using multiple vertex buffers we can use just one attribute per slot
		// Use BindVertexBufferSlot instead of BindVertexBuffer for this approach
		// First argument is vertex buffer second argument is slot index
		context->BindVertexBufferSlot(m_PositionVertexBuffer, 0);
		context->BindVertexBufferSlot(m_ColorVertexBuffer, 1);

		// Draw 3 vertices
		context->Draw(3);

		// Unbind the resources from the pipeline
		context->BindVertexBuffer(nullptr);
	}

private:
	GP::GfxShader m_BasicShader{ "samples/shaders/sample3/basic_shader.hlsl" };

	// When using multiple vertex buffers we can use just one attribute per slot
	GP::GfxVertexBuffer<Vec2>* m_PositionVertexBuffer;
	GP::GfxVertexBuffer<Vec3>* m_ColorVertexBuffer;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
	GP::Init(hInstance, 1024, 768, "Demo");

	GP::AddRenderPass(new BasicRenderPass());

	GP::Run();

	GP::Deinit();
}

#endif // CURRENT_SAMPLE

