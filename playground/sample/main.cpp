#include <Engine.h>

#include <memory>

#define RUN_SAMPLE

class DrawUVRenderPass : public GP::RenderPass
{
public:
    DrawUVRenderPass()
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
    AlbedoPass(GP::Scene& scene):
        m_Scene(scene)
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
        device->BindConstantBuffer(VS, m_Scene.GetCameraBuffer(), 0);

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

    std::unique_ptr<GP::GfxShader> m_Shader;
    std::unique_ptr<GP::GfxDeviceState> m_DeviceState;
};

class PlayerController : public GP::Controller
{
public:
    PlayerController(GP::Scene& scene) :
        m_Scene(scene) {}

    void UpdateInput(float dt) override
    {
        static const Vec3 UP_DIR = Vec3(0.0f, 1.0f, 0.0f);
        static const Vec3 RIGHT_DIR = Vec3(1.0f, 0.0f, 0.0f);
        static const Vec3 FORWARD_DIR = Vec3(0.0f, 0.0f, -1.0f);

        Vec3 moveDir = VEC3_ZERO;
        Vec3 rotation = VEC3_ZERO;

        if (GP::Input::IsKeyPressed(VK_ESCAPE))
        {
            GP::Shutdown();
            return;
        }

        if (GP::Input::IsKeyPressed('W'))
        {
            moveDir += FORWARD_DIR;
        }

        if (GP::Input::IsKeyPressed('S'))
        {
            moveDir -= FORWARD_DIR;
        }

        if (GP::Input::IsKeyPressed('A'))
        {
            moveDir -= RIGHT_DIR;
        }

        if (GP::Input::IsKeyPressed('D'))
        {
            moveDir += RIGHT_DIR;
        }

        if (GP::Input::IsKeyPressed('Q'))
        {
            moveDir += UP_DIR;
        }

        if (GP::Input::IsKeyPressed('E'))
        {
            moveDir -= UP_DIR;
        }

        if (GP::Input::IsKeyPressed(VK_UP))
        {
            rotation.x += 1.0f;
        }

        if (GP::Input::IsKeyPressed(VK_DOWN))
        {
            rotation.x -= 1.0f;
        }

        if (GP::Input::IsKeyPressed(VK_LEFT))
        {
            rotation.y -= 1.0f;
        }

        if (GP::Input::IsKeyPressed(VK_RIGHT))
        {
            rotation.y += 1.0f;
        }

        if (GP::Input::IsKeyPressed('R'))
        {
            GP::ReloadShaders();
        }

        m_Scene.MovePlayer(moveDir);

    }

private:
    GP::Scene& m_Scene;
};

#ifdef RUN_SAMPLE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
    GP::Scene SCENE;
    GP::Init(hInstance);
    GP::SetController(new PlayerController(SCENE));

    SCENE.Init();
    SCENE.Load("playground/sample/resources/CofeeCup/coffee_cup_obj.obj");

    GP::AddRenderPass(new DrawUVRenderPass());
    GP::AddRenderPass(new AlbedoPass(SCENE));
    
    GP::Run();

    GP::Deinit();
    return 0;
}
#endif // RUN_SAMPLE