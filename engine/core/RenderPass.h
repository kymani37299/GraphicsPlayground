#pragma once

#include "Common.h"

#define RENDER_PASS(debugName) ::GP::BeginRenderPassScoped JOIN(brps,__LINE__)(debugName)

namespace GP
{
	class GfxDevice;

	class RenderPass
	{
	public:
		virtual ~RenderPass() {}

		virtual void Init(GfxDevice* device) = 0;
		virtual void Render(GfxDevice* device) = 0;
		virtual void ReloadShaders() {};

		inline void SetInitialized(bool value) { m_Initialized = value; }
		inline bool IsInitialized() const { return m_Initialized; }

	private:
		bool m_Initialized = false;
	};
}