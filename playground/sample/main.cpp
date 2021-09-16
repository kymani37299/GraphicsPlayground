#include <Engine.h>

#include <memory>

//#define RUN_SAMPLE

class DrawUVRenderPass : public GP::RenderPass
{
public:
    virtual void Init()
    {
        GP::ShaderDesc shaderDesc = {};
        shaderDesc.path = "playground/sample/draw_uv.hlsl";
        shaderDesc.inputs.resize(2);
        shaderDesc.inputs[0] = { GP::ShaderInputFormat::Float2 , "POS" };
        shaderDesc.inputs[1] = { GP::ShaderInputFormat::Float2 , "TEXCOORD" };
        m_Shader.reset(new GP::GfxShader(shaderDesc));

        m_DeviceState.reset(new GP::GfxDeviceState());
        m_DeviceState->EnableBackfaceCulling(false);
        m_DeviceState->Compile();
    }

    virtual void Render(GP::GfxDevice* device) override
    {
        using namespace GP;
        RENDER_PASS("DrawUVRenderPass");

        DeviceStateScoped _dss(m_DeviceState.get());
        RenderTargetScoped _Rts(device->GetFinalRT(), nullptr);
        device->BindShader(m_Shader.get());
        device->DrawFullSceen();
    }

    virtual void ReloadShaders() override
    {
        m_Shader->Reload();
    }

private:
    std::unique_ptr<GP::GfxShader> m_Shader;
    std::unique_ptr<GP::GfxDeviceState> m_DeviceState;
};

class AlbedoPass : public GP::RenderPass
{
public:
    AlbedoPass(GP::Scene& scene, GP::Camera& camera):
        m_Scene(scene),
        m_Camera(camera)
    { }

    virtual void Init() override
    {
        GP::ShaderDesc shaderDesc = {};
        shaderDesc.path = "playground/sample/opaque.hlsl";
        shaderDesc.inputs.resize(3);
        shaderDesc.inputs[0] = { GP::ShaderInputFormat::Float3 , "POS" };
        shaderDesc.inputs[1] = { GP::ShaderInputFormat::Float3 , "NORMAL" };
        shaderDesc.inputs[2] = { GP::ShaderInputFormat::Float2 , "TEXCOORD" };
        m_Shader.reset(new GP::GfxShader(shaderDesc));

        m_DeviceState.reset(new GP::GfxDeviceState());
        m_DeviceState->EnableDepthTest(true);
        m_DeviceState->Compile();
    }

    virtual void Render(GP::GfxDevice* device) override
    {
        using namespace GP;
        RENDER_PASS("AlbedoPass");

        DeviceStateScoped _dss(m_DeviceState.get());
        RenderTargetScoped _Rts(device->GetFinalRT(), device->GetFinalRT());
        device->BindShader(m_Shader.get());
        device->BindConstantBuffer(VS, m_Camera.GetBuffer(), 0);

        for(GP::SceneObject* obj : m_Scene.GetObjects())
        {
            device->BindVertexBuffer(obj->GetMesh()->GetVertexBuffer());
            device->BindIndexBuffer(obj->GetMesh()->GetIndexBuffer());
            device->BindConstantBuffer(VS, obj->GetInstanceBuffer(), 1);
            device->BindTexture(PS, obj->GetMaterial()->GetDiffuseTexture(), 0);
            device->DrawIndexed(obj->GetMesh()->GetIndexBuffer()->GetNumIndices());
        }
    }

    virtual void ReloadShaders() override
    {
        m_Shader->Reload();
    }

private:
    GP::Scene& m_Scene;
    GP::Camera& m_Camera;

    std::unique_ptr<GP::GfxShader> m_Shader;
    std::unique_ptr<GP::GfxDeviceState> m_DeviceState;
};

#ifdef RUN_SAMPLE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
    GP::Init(hInstance);
   
    GP::Scene SCENE;
    SCENE.Load("playground/sample/resources/CofeeCup/coffee_cup_obj.obj");

    GP::Camera playerCamera;
    playerCamera.SetPosition({ -5.0f, 5.0f, -5.0f });
    playerCamera.LookAt({ 0.1f,0.1f,0.1f });

    GP::ShowCursor(false);
    GP::SetDefaultController(&playerCamera);

    GP::AddRenderPass(new DrawUVRenderPass());
    GP::AddRenderPass(new AlbedoPass(SCENE, playerCamera));
    
    GP::Run();

    GP::Deinit();
    
    return 0;
}
#endif // RUN_SAMPLE