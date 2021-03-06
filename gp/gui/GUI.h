#pragma once

#include <vector>

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace GP
{
	class GUIElement
	{
	public:
		GUIElement() {}
		virtual ~GUIElement() {}

		virtual void Reset() {}
		virtual void Update(float dt) = 0;
		virtual void Render() = 0;
	};

	class GUI
	{
	public:
		GUI(void* hwnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
		~GUI();

		void Reset();

		void InitializeDefaultScene();
		inline void AddElement(GUIElement* element) { m_Elements.push_back(element); }

		bool HandleWndProc(void* hwnd, unsigned int msg, unsigned int wparam, long lparam);

		void Update(float dt);
		void Render();

		inline void SetVisible(bool value) { m_Visible = value; }

	private:
		bool m_Visible = false;
		std::vector<GUIElement*> m_Elements;
	};

	extern GUI* g_GUI;
}