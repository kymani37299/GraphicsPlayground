#include "GUI.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

namespace GP
{
	GUI* g_GUI = nullptr;

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

	void GUI::Update(float dt)
	{
		for (GUIElement* element : m_Elements)
		{
			element->Update(dt);
		}
	}

	void GUI::Render()
	{
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