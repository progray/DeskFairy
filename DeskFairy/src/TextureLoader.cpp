#include "TextureLoader.h"
#include <iostream>
#include <algorithm>
#include "FileLoader.h"

#define STBI_NO_STDIO
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace
{
    TextureLoader* instance = nullptr;
}

TextureLoader::TextureLoader()
{

}

TextureLoader::~TextureLoader()
{
    ReleaseTextures();
}

TextureLoader* TextureLoader::GetInstance()
{
    if (!instance)
    {
        instance = new TextureLoader();
    }
    return instance;
}

void TextureLoader::ReleaseInstance()
{
    if (instance)
    {
        delete instance;
    }
    instance = nullptr;
}

TextureLoader::TextureInfo* TextureLoader::CreateTextureFromPngFile(std::string fileName)
{
    //寻找是否已经加载过该文件
    for (auto tex : _textures)
    {
        if (tex->fileName == fileName)
        {
            return tex;
        }
    }

    GLuint textureId;
    int width, height, channels;
    unsigned int size;
    unsigned char* png;
    unsigned char* address;

    address = FileLoader::Load(fileName, &size);

    png = stbi_load_from_memory(
        address,
        static_cast<int>(size),
        &width,
        &height,
        &channels,
        STBI_rgb_alpha);

    GLFuncs()->glGenTextures(1, &textureId);
    GLFuncs()->glBindTexture(GL_TEXTURE_2D, textureId);
    GLFuncs()->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, png);
    GLFuncs()->glGenerateMipmap(GL_TEXTURE_2D);
    GLFuncs()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    GLFuncs()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GLFuncs()->glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(png);
    FileLoader::Release(address);

    TextureLoader::TextureInfo* textureInfo = new TextureLoader::TextureInfo();
    if (textureInfo)
    {
        textureInfo->fileName = fileName;
        textureInfo->width = width;
        textureInfo->height = height;
        textureInfo->id = textureId;

        _textures.push_back(textureInfo);
    }

    return textureInfo;
}

void TextureLoader::ReleaseTextures()
{
    for (auto tex : _textures)
    {
        delete tex;
    }

    _textures.clear();
}

void TextureLoader::ReleaseTexture(GLuint textureId)
{
    for (auto tex : _textures)
    {
        if (tex->id != textureId)
        {
            continue;
        }

        delete tex;
        _textures.erase(std::find(_textures.begin(), _textures.end(), tex));

        break;
    }
}

void TextureLoader::ReleaseTexture(std::string fileName)
{
    for (auto tex : _textures)
    {
        if (tex->fileName != fileName)
        {
            continue;
        }

        delete tex;
        _textures.erase(std::find(_textures.begin(), _textures.end(), tex));

        break;
    }
}

TextureLoader::TextureInfo* TextureLoader::GetTextureInfo(GLuint textureId) const
{
    for (auto tex : _textures)
    {
        if (tex->id != textureId)
        {
            continue;
        }

        return tex;
    }

    return nullptr;
}
