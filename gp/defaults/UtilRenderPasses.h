#pragma once

#include "core/RenderPass.h"
#include "gfx/GfxDevice.h"
#include "gfx/GfxTexture.h"
#include "gfx/GfxShader.h"

namespace GP
{
	class CopyToRTPass : public RenderPass
	{
	public:
		CopyToRTPass(GfxBaseTexture2D* src, GfxRenderTarget* dst) :
			m_Src(src),
			m_Dst(dst) {}

		virtual void Init(GfxContext*) override {}

		virtual void Render(GfxContext* context) override 
		{
			GP_SCOPED_PROFILE("Utils::CopyToRTPass");

			if (m_Dst)
			{
				GP_SCOPED_RT(context, m_Dst, nullptr);
				context->BindShader(&m_CopyShader);
				context->BindTexture2D(GP::PS, m_Src, 0);
				context->DrawFC();
			}
			else
			{
				context->BindShader(&m_CopyShader);
				context->BindTexture2D(GP::PS, m_Src, 0);
				context->DrawFC();
			}

		}
	private:
		GfxShader m_CopyShader{ "gp/shaders/copy.hlsl" };
		GfxBaseTexture2D* m_Src;
		GfxRenderTarget* m_Dst;
	};
}