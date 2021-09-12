#pragma once

#include "gfx/device/DxDevice.h"

class BeginRenderPassScoped
{
public:
	BeginRenderPassScoped(DxDevice& device, const std::string& debugName):
		m_Device(device)
	{
		m_Device.BeginPass(debugName);
	}

	~BeginRenderPassScoped()
	{
		m_Device.EndPass();
	}

private:
	DxDevice& m_Device;
};

class DeviceStateScoped
{
public:
	DeviceStateScoped(DxDevice& device, DxDeviceState* state) :
		m_Device(device),
		m_LastState(device.GetState())
	{
		m_Device.BindState(state);
	}

	~DeviceStateScoped()
	{
		m_Device.BindState(m_LastState);
	}

private:
	DxDevice& m_Device;
	DxDeviceState* m_LastState;
};

class RenderTargetScoped
{
public:
	RenderTargetScoped(DxDevice& device, DxRenderTarget* rt, DxRenderTarget* ds = nullptr):
		m_Device(device),
		m_LastRT(device.GetRenderTarget()),
		m_LastDS(device.GetDepthStencil())
	{
		m_Device.SetRenderTarget(rt);
		m_Device.SetDepthStencil(ds);
	}

	~RenderTargetScoped()
	{
		m_Device.SetRenderTarget(m_LastRT);
		m_Device.SetDepthStencil(m_LastDS);
	}

private:
	DxDevice& m_Device;
	DxRenderTarget* m_LastRT;
	DxRenderTarget* m_LastDS;
};