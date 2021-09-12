#pragma once

class DxDevice;
class DxTexture;
class DxCubemapRenderTarget;

class DxConverter
{
public:
	static DxTexture* ToDxTexture(DxDevice* device, DxCubemapRenderTarget*& rt);
};