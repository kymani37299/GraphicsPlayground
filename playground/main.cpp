#include <Engine.h>

#include "PlaygroundSample.h"
#include "sponza/Sponza.h"
#include "nature/Nature.h"

PlaygroundSample* GetSample(unsigned int sampleNumber)
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
	GP::Init(hInstance, "Playground");

	unsigned int sampleNumber = 1;
	PlaygroundSample* sample = GetSample(sampleNumber);
	if (!sample) return -1;

	sample->SetupRenderer();
	
	GP::Run();

	delete sample;
	
	GP::Deinit();
}