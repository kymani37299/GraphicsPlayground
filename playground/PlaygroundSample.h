#pragma once

class PlaygroundSample
{
public:
	virtual ~PlaygroundSample() { }
	virtual void SetupRenderer() = 0;
};