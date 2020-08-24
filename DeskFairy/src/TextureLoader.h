#pragma once

#include "GLFuncs.h"
#include <vector>

class TextureLoader
{
public:
    struct TextureInfo
    {
        GLuint id;           
        int width;           
        int height;             
        std::string fileName;
    };

    static TextureLoader* GetInstance();

    static void ReleaseInstance();

    TextureInfo* CreateTextureFromPngFile(std::string fileName);

    void ReleaseTextures();

    void ReleaseTexture(GLuint textureId);

    void ReleaseTexture(std::string fileName);

    TextureInfo* GetTextureInfo(GLuint textureId) const;

private:
    TextureLoader();

    ~TextureLoader();

    std::vector<TextureInfo*> _textures;
};