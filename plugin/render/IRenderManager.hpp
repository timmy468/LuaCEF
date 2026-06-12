#pragma once

#include <string>

#include <d3d9.h>

#include <CEntity.h>

class IRenderManager
{
  public:
    virtual ~IRenderManager() = default;

    virtual void Initialize() = 0;
    virtual void Shutdown() = 0;

    virtual void OnDeviceLost() = 0;
    virtual void OnDeviceReset() = 0;

    virtual void RenderTexture(IDirect3DTexture9* texture, float x, float y, float width, float height) = 0;
    virtual void AddToEntityTexture(CEntity* entity, const std::string& textureName, IDirect3DTexture9* texture) = 0;
    virtual void RemoveFromEntityTexture(CEntity* entity, const std::string& textureName) = 0;
};