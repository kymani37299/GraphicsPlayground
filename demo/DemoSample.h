#pragma once

class DemoSample
{
public:
	virtual ~DemoSample() { }
	virtual void SetupRenderer() = 0;
};