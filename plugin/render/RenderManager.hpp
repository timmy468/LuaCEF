#pragma once

#include "IRenderManager.hpp"

#include <list>
#include <unordered_map>

#include <RenderWare.h>
#include <d3d9.h>

class RenderManager : public IRenderManager
{
  private:
    IDirect3DStateBlock9* stateBlock_{};

    using TextureMap = std::unordered_map<std::string, RwTexture*>;
    std::unordered_map<CEntity*, TextureMap> entityTexturePool_;

    std::list<std::pair<uintptr_t*, uintptr_t>> storedMaterials_;

  public:
    RenderManager() = default;
    ~RenderManager() = default;

    void Initialize() override;
    void Shutdown() override;

    void RenderTexture(IDirect3DTexture9* texture, float x, float y, float width, float height) override;
    void AddToEntityTexture(CEntity* entity, const std::string& textureName, IDirect3DTexture9* texture) override;
    void RemoveFromEntityTexture(CEntity* entity, const std::string& textureName) override;

    template <typename Addr> inline void AddStoredMaterial(Addr addr)
    {
        storedMaterials_.emplace_back((uintptr_t*)addr, *(uintptr_t*)addr);
    }

  private:
    void InitHooks();

    void CEntityRenderBeforeHook(CEntity* entity);
    void CEntityRenderAfterHook(CEntity* entity);
};