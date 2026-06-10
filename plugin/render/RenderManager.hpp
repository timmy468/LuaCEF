#pragma once

#include "IRenderManager.hpp"

#include <d3d9.h>

class RenderManager : public IRenderManager
{
  private:
    IDirect3DStateBlock9* stateBlock_{};

  public:
    RenderManager() = default;
    ~RenderManager() = default;

    void Initialize() override;
    void Shutdown() override;

    void RenderTexture(IDirect3DTexture9* texture, float x, float y, float width, float height) override;
};