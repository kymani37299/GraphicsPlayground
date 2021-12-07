#include "CurrentSample.h"

// Sample example as Sample1_HelloTriangle, but now with addition that we can override color that we pass using constant buffer

// Constant buffers are used to pass some constant data to the shader

#if CURRENT_SAMPLE == 4

#include <GP.h>

// This is the struct we will use in constant buffer
// The memory for ConstantBuffer must be packed into 4 bytes, more info on: https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules
// Variables in this struct should be in the exact same order as they are in the shader
struct ColorOverride
{
	alignas(16) bool enable; // We must align this variable to 4 bytes since it uses just 1 byte but  Vec3 should begin after 4 bytes
	Vec3 color;
};

// TIPS: If you want to be safe with memory just declare every memeber as Vec4 so for this example it should like
// Example:
//struct ColorOverride
//{
//	Vec4 enable; // We would just use enable.x, for example if enable.x < 0.0f then it is false , if it is >1.0f then it is true
//	Vec4 color; // Just use color.rgb
//};

struct Vertex
{
	Vec2 position;
	Vec3 color;
};

class BasicRenderPass : public GP::RenderPass
{
public:

	virtual void Init(GP::GfxContext* context) override
	{
		Vertex vertices[] = {
			{ Vec2(0.0f, 1.0f), Vec3(1.0f,0.0f,0.0f) },
			{ Vec2(-1.0f, -1.0f), Vec3(0.0f, 1.0f, 0.0f) },
			{ Vec2(1.0f, -1.0f), Vec3(0.0f, 0.0f, 1.0f) }
		};

		m_TriangleVertexBuffer = new GP::GfxVertexBuffer<Vertex>(vertices, 3);

		// Uploading to constant buffer could be also done here since we are uploading the same data every frame, so it can happen just once
	}

	virtual ~BasicRenderPass()
	{
		delete m_TriangleVertexBuffer;
	}

	virtual void Render(GP::GfxContext* context) override
	{
		// Step 1: Fill the data in struct
		ColorOverride colorOverride;
		colorOverride.enable = true;
		colorOverride.color = Vec3(1.0f, 1.0f, 1.0f); // White color

		// Step 2: Upload to buffer
		// To upload to constant buffer just use UploadToBuffer
		// First param is constant buffer, second param is reference to struct you are uploading
		context->UploadToBuffer(&m_ColorOverrideConstantBuffer, colorOverride);

		context->Clear(Vec4(0.05f, 0.05f, 0.05f, 1.0f));
		context->BindShader(&m_BasicShader);
		context->BindVertexBuffer(m_TriangleVertexBuffer);

		// Step 3: Bind constant buffer
		// 
		// First parameter: Shader stage we passed GP::PS because we want to use it for pixel shader.
		// If we wanted to use it for a vertex shader we would pass GP::VS, 
		// Also you can combine flagsfor example GP::VS | GP::PS will use this buffer both in Vertex and Pixel shader
		// There are flags for every shader type, so GP::CS (Compute) , GP::GS (Geometry), GP::DS (Domain), GP::HS (Hull)
		// 
		// Second parameter: Pointer to constant buffer
		//
		// Third parameter: Constant buffer slot we are binding this constant buffer
		// This should match the register(bN) you are using in the shader. 
		// For this example we are using register(b0) so we are binding it to slot 0
		context->BindConstantBuffer(GP::PS, &m_ColorOverrideConstantBuffer, 0);

		context->Draw(3);
	}

private:
	GP::GfxShader m_BasicShader{ "samples/shaders/sample4/basic_shader.hlsl" };
	GP::GfxVertexBuffer<Vertex>* m_TriangleVertexBuffer;

	// To declare constant buffer just pass struct type as template paramter
	GP::GfxConstantBuffer<ColorOverride> m_ColorOverrideConstantBuffer;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
	GP::Init(hInstance, 1024, 768, "Demo");

	GP::AddRenderPass(new BasicRenderPass());

	GP::Run();

	GP::Deinit();
}

#endif // CURRENT_SAMPLE

