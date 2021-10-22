#pragma once

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace GP
{
	class GUI
	{
	public:
		GUI(void* hwnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
		~GUI();

		void Render();
	};

	extern GUI* g_GUI;
}