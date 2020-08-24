#pragma once

#include <CubismFramework.hpp>
#include <Model/CubismUserModel.hpp>
#include <ICubismModelSetting.hpp>
#include <Type/csmRectF.hpp>
#include <Rendering/OpenGL/CubismOffscreenSurface_OpenGLES2.hpp>
#include <string>

class Model : public Csm::CubismUserModel
{
public:
    Model();

    virtual ~Model();

    void LoadAssets(const Csm::csmChar* dir, const  Csm::csmChar* fileName);

    void ReloadRenderer();

    void Update(Csm::csmFloat32 deltaTimeSeconds);

    void Draw(Csm::CubismMatrix44& matrix);

    virtual Csm::csmBool HitTest(const Csm::csmChar* hitAreaName, Csm::csmFloat32 x, Csm::csmFloat32 y);

    virtual Csm::csmBool HitTest(Csm::csmFloat32 x, Csm::csmFloat32 y);

    Csm::Rendering::CubismOffscreenFrame_OpenGLES2& GetRenderBuffer();

    Csm::csmFloat32 GetParamValue(std::string name) const;

    Csm::csmFloat32 GetParamValueDefault(std::string name) const;

    Csm::csmFloat32 GetParamValueMax(std::string name) const;

    Csm::csmFloat32 GetParamValueMin(std::string name) const;

    void SetParamValue(std::string name, Csm::csmFloat32 value, Csm::csmFloat32 weight = 1.0);

    void AddParamValue(std::string name, Csm::csmFloat32 value, Csm::csmFloat32 weight = 1.0);

    void LerpParamValue(std::string name, Csm::csmFloat32 value, Csm::csmFloat32 delta);

    void SetWind(double x, double y);

protected:
    void DoDraw();

private:
    void SetupModel(Csm::ICubismModelSetting* setting);

    void SetupTextures();

    Csm::ICubismModelSetting* _modelSetting;
    Csm::csmString _modelHomeDir;
    Csm::csmFloat32 _userTimeSeconds;
    Csm::csmVector<Csm::csmRectF> _hitArea;
    Csm::csmVector<Csm::csmRectF> _userArea;

    Csm::Rendering::CubismOffscreenFrame_OpenGLES2  _renderBuffer;   ///< ¥Õ¥ì©`¥à¥Ð¥Ã¥Õ¥¡ÒÔÍâ¤ÎÃè»­ÏÈ
};