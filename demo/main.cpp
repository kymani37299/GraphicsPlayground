#include <GP.h>

#include "DemoSample.h"
#include "sponza/Sponza.h"
#include "nature/Nature.h"

class MainController;

GP::Camera* g_Camera = nullptr;
DemoSample* g_CurrentSample = nullptr;
MainController* g_Controller = nullptr;
static constexpr unsigned int NUM_SAMPLES = 2;

DemoSample* GetSample(unsigned int sampleNumber)
{
	switch (sampleNumber)
	{
	case 0:
		return new NatureSample::NatureSample();
	case 1:
		return new SponzaSample::SponzaSample();
	}

	return nullptr;
}

void SetSample(unsigned int sampleNumber)
{
	if (g_CurrentSample) delete g_CurrentSample;
	GP::Reset();
	g_CurrentSample = GetSample(sampleNumber);
	g_CurrentSample->SetupRenderer();
}

class MainController : public GP::DefaultController
{
public:
	MainController(GP::Camera& camera) :
		GP::DefaultController(camera)
	{ }

	void UpdateInput(float dt)
	{
		GP::DefaultController::UpdateInput(dt);

		if (GP::Input::IsKeyJustPressed('E'))
		{
			m_CurrentSample = (m_CurrentSample + 1) % NUM_SAMPLES;
			SetSample(m_CurrentSample);
		}
		else if (GP::Input::IsKeyJustPressed('Q'))
		{
			m_CurrentSample = (m_CurrentSample - 1) % NUM_SAMPLES;
			SetSample(m_CurrentSample);
		}
	}

private:
	unsigned int m_CurrentSample = 0;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
	GP::Init(hInstance, 768, 768, "Demo");

	g_Camera = new GP::Camera();
	g_Camera->SetPosition({ 0.0,100.0,0.0 });

	g_Controller = new MainController(*g_Camera);

	GP::ShowCursor(false);
	GP::SetController((GP::Controller*)g_Controller);
	SetSample(0);
	GP::Run();
	
	GP::Deinit();
}