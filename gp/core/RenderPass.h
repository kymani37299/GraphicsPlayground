#pragma once

#include "Common.h"
#include "gfx/ScopedOperations.h"

namespace GP
{
	class GfxContext;

	class RenderPass
	{
	public:
		virtual ~RenderPass() {}

		virtual void Init(GfxContext* context) = 0;
		virtual void Render(GfxContext* context) = 0;
		virtual void ReloadShaders() {};
		virtual void OnWindowResized(GfxContext* context, unsigned int newWidth, unsigned int newHeight) {}

		inline void SetInitialized(bool value) { m_Initialized = value; }
		inline bool IsInitialized() const { return m_Initialized; }

	private:
		bool m_Initialized = false;
	};
}