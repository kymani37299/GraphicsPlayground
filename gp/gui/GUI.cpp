#include "GUI.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include <Windows.h>

#include "gfx/GfxDevice.h"
#include "core/RenderPass.h"
#include "gui/LoggerGUI.h"
#include "gui/ProfilerGUI.h"
#include "gui/RuntimeVariableGUI.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace GP
{
	GUI* g_GUI = nullptr;

	void GUI::Reset()
	{
		for (GUIElement* element : m_Elements)
		{
			element->Reset();
		}
	}

	void GUI::InitializeDefaultScene()
	{
		g_GUI->AddElement(new LoggerGUI());
		g_GUI->AddElement(new ProfilerGUI());
		g_GUI->AddElement(g_RuntimeVariableGUI);
	}

	GUI::GUI(void* hwnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(hwnd);
		ImGui_ImplDX11_Init(device, deviceContext);
	}

	GUI::~GUI()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	bool GUI::HandleWndProc(void* hwnd, unsigned int msg, unsigned int wparam, long lparam)
	{
		return ImGui_ImplWin32_WndProcHandler((HWND) hwnd, msg, wparam, lparam);
	}

	void GUI::Update(float dt)
	{
		for (GUIElement* element : m_Elements)
		{
			element->Update(dt);
		}
	}

	void GUI::Render()
	{
		if (!m_Visible) return;

		GP_SCOPED_PROFILE("ImGui");

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		for (GUIElement* element : m_Elements)
		{
			element->Render();
		}

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

}