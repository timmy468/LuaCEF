#pragma once

#include <d3d9.h>

class IRenderManager
{
  public:
    virtual ~IRenderManager() = default;

    virtual void Initialize() = 0;
    virtual void Shutdown() = 0;

    virtual void RenderTexture(IDirect3DTexture9* texture, float x, float y, float width, float height) = 0;
};