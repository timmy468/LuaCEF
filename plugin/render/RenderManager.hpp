#pragma once

#include "IRenderManager.hpp"

class RenderManager : public IRenderManager
{
  public:
    RenderManager() = default;
    ~RenderManager() = default;

    void Initialize() override;
    void Shutdown() override;

    void RenderTexture(IDirect3DTexture9* texture, float x, float y, float width, float height) override;
};