#include "Model.h"
#include <fstream>
#include <vector>
#include <CubismModelSettingJson.hpp>
#include <Type/CubismBasicType.hpp>
#include <Motion/CubismMotion.hpp>
#include <Physics/CubismPhysics.hpp>
#include <CubismDefaultParameterId.hpp>
#include <Rendering/OpenGL/CubismRenderer_OpenGLES2.hpp>
#include <Utils/CubismString.hpp>
#include <Id/CubismIdManager.hpp>
#include <Motion/CubismMotionQueueEntry.hpp>

#include "TextureLoader.h"
#include "FileLoader.h"
#include "Defines.h"
#include "Logger.h"

using namespace Live2D::Cubism::Framework;
using namespace Live2D::Cubism::Framework::DefaultParameterId;

Model::Model()
    : CubismUserModel()
    , _modelSetting(NULL)
    , _userTimeSeconds(0.0f)
{

}

Model::~Model()
{
    _renderBuffer.DestroyOffscreenFrame();

    for (csmInt32 i = 0; i < _modelSetting->GetMotionGroupCount(); i++)
    {
        const csmChar* group = _modelSetting->GetMotionGroupName(i);
    }
    delete(_modelSetting);
}

void Model::LoadAssets(const csmChar* dir, const csmChar* fileName)
{
    _modelHomeDir = dir;

    Logger::Print("Load model setting: %s", fileName);

    csmSizeInt size;
    const csmString path = csmString(dir) + fileName;

    csmByte* buffer = FileLoader::Load(path.GetRawString(), &size);
    ICubismModelSetting* setting = new CubismModelSettingJson(buffer, size);
    FileLoader::Release(buffer);

    SetupModel(setting);

    CreateRenderer();

    SetupTextures();
}

void Model::SetupModel(ICubismModelSetting* setting)
{
    _updating = true;
    _initialized = false;

    _modelSetting = setting;

    csmByte* buffer;
    csmSizeInt size;

    //加载模型文件
    if (strcmp(_modelSetting->GetModelFileName(), "") != 0)
    {
        csmString path = _modelSetting->GetModelFileName();
        path = _modelHomeDir + path;

        Logger::Print("Create model: %s", setting->GetModelFileName());

        buffer = FileLoader::Load(path.GetRawString(), &size);
        LoadModel(buffer, size);
        FileLoader::Release(buffer);
    }

    //加载物理文件
    if (strcmp(_modelSetting->GetPhysicsFileName(), "") != 0)
    {
        csmString path = _modelSetting->GetPhysicsFileName();
        path = _modelHomeDir + path;

        buffer = FileLoader::Load(path.GetRawString(), &size);
        LoadPhysics(buffer, size);
        FileLoader::Release(buffer);
    }

    //加载用户数据文件
    if (strcmp(_modelSetting->GetUserDataFile(), "") != 0)
    {
        csmString path = _modelSetting->GetUserDataFile();
        path = _modelHomeDir + path;
        buffer = FileLoader::Load(path.GetRawString(), &size);
        LoadUserData(buffer, size);
        FileLoader::Release(buffer);
    }

    //加载layout
    csmMap<csmString, csmFloat32> layout;
    _modelSetting->GetLayoutMap(layout);
    _modelMatrix->SetupFromLayout(layout);

    _updating = false;
    _initialized = true;
}

void Model::Update(csmFloat32 deltaTimeSeconds)
{
    _userTimeSeconds += deltaTimeSeconds;

    if (_physics != NULL)
    {
        _physics->Evaluate(_model, deltaTimeSeconds);
    }

    _model->Update();
}

void Model::DoDraw()
{
    if (_model == NULL)
    {
        return;
    }

    GetRenderer<Rendering::CubismRenderer_OpenGLES2>()->DrawModel();
}

void Model::Draw(CubismMatrix44& matrix)
{
    if (_model == NULL)
    {
        return;
    }

    matrix.MultiplyByMatrix(_modelMatrix);

    GetRenderer<Rendering::CubismRenderer_OpenGLES2>()->SetMvpMatrix(&matrix);

    DoDraw();
}

csmBool Model::HitTest(const csmChar* hitAreaName, csmFloat32 x, csmFloat32 y)
{
    if (_opacity < 1)
    {
        return false;
    }
    const csmInt32 count = _modelSetting->GetHitAreasCount();
    for (csmInt32 i = 0; i < count; i++)
    {
        if (strcmp(_modelSetting->GetHitAreaName(i), hitAreaName) == 0)
        {
            const CubismIdHandle drawID = _modelSetting->GetHitAreaId(i);
            return IsHit(drawID, x, y);
        }
    }
    return false;
}

csmBool Model::HitTest(csmFloat32 x, csmFloat32 y)
{
    if (_opacity < 1)
    {
        return false;
    }
    const csmInt32 count = _modelSetting->GetHitAreasCount();
    for (csmInt32 i = 0; i < count; i++)
    {
        const CubismIdHandle drawID = _modelSetting->GetHitAreaId(i);
        if (IsHit(drawID, x, y))
            return true;
    }
    return false;
}

void Model::ReloadRenderer()
{
    DeleteRenderer();

    CreateRenderer();

    SetupTextures();
}

void Model::SetupTextures()
{
    for (csmInt32 modelTextureNumber = 0; modelTextureNumber < _modelSetting->GetTextureCount(); modelTextureNumber++)
    {
        if (strcmp(_modelSetting->GetTextureFileName(modelTextureNumber), "") == 0)
        {
            continue;
        }

        csmString texturePath = _modelSetting->GetTextureFileName(modelTextureNumber);
        texturePath = _modelHomeDir + texturePath;

        TextureLoader::TextureInfo* texture = TextureLoader::GetInstance()->CreateTextureFromPngFile(texturePath.GetRawString());
        const csmInt32 glTextueNumber = texture->id;

        GetRenderer<Rendering::CubismRenderer_OpenGLES2>()->BindTexture(modelTextureNumber, glTextueNumber);
    }

    GetRenderer<Rendering::CubismRenderer_OpenGLES2>()->IsPremultipliedAlpha(false);
}

Csm::Rendering::CubismOffscreenFrame_OpenGLES2& Model::GetRenderBuffer()
{
    return _renderBuffer;
}

Csm::csmFloat32 Model::GetParamValue(std::string name) const
{
    Csm::CubismIdHandle id = Csm::CubismFramework::GetIdManager()->GetId(name.c_str());
    return _model->GetParameterValue(_model->GetParameterIndex(id));
}

Csm::csmFloat32 Model::GetParamValueDefault(std::string name) const
{
    Csm::CubismIdHandle id = Csm::CubismFramework::GetIdManager()->GetId(name.c_str());
    return _model->GetParameterDefaultValue(_model->GetParameterIndex(id));
}

Csm::csmFloat32 Model::GetParamValueMax(std::string name) const
{
    Csm::CubismIdHandle id = Csm::CubismFramework::GetIdManager()->GetId(name.c_str());
    return _model->GetParameterMaximumValue(_model->GetParameterIndex(id));
}

Csm::csmFloat32 Model::GetParamValueMin(std::string name) const
{
    Csm::CubismIdHandle id = Csm::CubismFramework::GetIdManager()->GetId(name.c_str());
    return _model->GetParameterMinimumValue(_model->GetParameterIndex(id));
}

void Model::SetParamValue(std::string name, Csm::csmFloat32 value, Csm::csmFloat32 weight)
{
    Csm::CubismIdHandle id = Csm::CubismFramework::GetIdManager()->GetId(name.c_str());
    _model->SetParameterValue(_model->GetParameterIndex(id), value, weight);
}

void Model::AddParamValue(std::string name, Csm::csmFloat32 value, Csm::csmFloat32 weight)
{
    Csm::CubismIdHandle id = Csm::CubismFramework::GetIdManager()->GetId(name.c_str());
    _model->AddParameterValue(_model->GetParameterIndex(id), value, weight);
}

void Model::LerpParamValue(std::string name, Csm::csmFloat32 value, Csm::csmFloat32 delta)
{
    Csm::csmFloat32 v = GetParamValue(name);
    v = Def::Lerp(v, value, delta);
    SetParamValue(name, v);
}

void Model::SetWind(double x, double y)
{
    if (_physics)
    {
        Csm::CubismPhysics::Options opt;
        opt.Gravity.X = 0;
        opt.Gravity.Y = -1;
        opt.Wind.X = x;
        opt.Wind.Y = y;
        _physics->SetOptions(opt);
    }
}
