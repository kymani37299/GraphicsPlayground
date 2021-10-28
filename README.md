# GraphicsPlayground

## Using GP

The full support for library use is still in progress. <br />
For now the best way to use the lib is to clone the project in your own repository and edit playground project for your own use. <br />
Dependencies:
  - DirectX sdk
  
## Documentation

### Setup

1. Initialize settings you want for the renderer. [settings docs in progress] <br />
2. Setup render passes
3. Call GP::Run() to actually run the application
4. Call GP::Deinit() for freeing the resources

Example:
```
#include <GP.h>

class RenderPassX : public GP::RenderPass
{
public:
	virtual ~RenderPassX()
	{
		// Delete resources
	}

	virtual void Init(GP::GfxDevice* device)
	{
		// Initialize resources
	}

	virtual void Render(GP::GfxDevice* device)
	{
		// Render code
	}

	virtual void ReloadShaders()
	{
		// You can use this optionally so you can recompile shaders in runtime with GP::ReloadShaders() in your controller
		// 
		// Example:
		// 
		// shader1->Reload();
		// shader2->Reload();
	}
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
	// 1.
	GP::Init(hInstance);

	GP::Camera playerCamera;
	playerCamera.SetPosition({ 0.0,100.0,0.0 });

	GP::ShowCursor(false);
	GP::SetDefaultController(&playerCamera);

	// 2.
	GP::AddRenderPass(new RenderPassX());

	// 3.
	GP::Run();

	// 4.
	GP::Deinit();
}
```

For more information how to use resources look at the playground example

[Documentation in progress]
