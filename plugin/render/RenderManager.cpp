#include "RenderManager.hpp"

#include <plugin.h>

#include <kthook/kthook.hpp>

struct Vertex
{
    float x, y, z, rhw;
    D3DCOLOR color;
    float u, v;
};

#define FVF_VERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

void RenderManager::Initialize()
{
    // TODO: Так и не реализованная до конца поддержка аттача цйефа к объектам
    // InitHooks();
}

void RenderManager::Shutdown()
{
    if (stateBlock_)
        stateBlock_->Release();
}

void RenderManager::OnDeviceLost()
{
    if (stateBlock_)
    {
        stateBlock_->Release();
        stateBlock_ = nullptr;
    }
}

void RenderManager::OnDeviceReset()
{
    // ничего
}

void RenderManager::RenderTexture(IDirect3DTexture9* texture, float x, float y, float width, float height)
{
    auto* device = GetD3DDevice();
    if (!device || !texture)
        return;

    if (!stateBlock_)
    {
        auto* device = GetD3DDevice();
        device->CreateStateBlock(D3DSBT_ALL, &stateBlock_);
    }

    stateBlock_->Capture();

    device->SetPixelShader(nullptr);
    device->SetVertexShader(nullptr);

    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    device->SetRenderState(D3DRS_LIGHTING, FALSE);
    device->SetRenderState(D3DRS_ZENABLE, FALSE);
    device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

    device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

    device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

    device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    device->SetRenderState(D3DRS_FOGENABLE, FALSE);

    device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

    device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

    device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    device->SetFVF(FVF_VERTEX);
    device->SetTexture(0, texture);

    // D3D9 half-pixel offset: pixel centers sit at (0.5, 0.5), so shift the
    // quad by half a pixel to sample texels exactly 1:1 without blurring.
    const float x0 = x - 0.5f;
    const float y0 = y - 0.5f;

    Vertex vertices[] = {{x0, y0, 0.0f, 1.0f, 0xFFFFFFFF, 0.0f, 0.0f},
                         {x0 + width, y0, 0.0f, 1.0f, 0xFFFFFFFF, 1.0f, 0.0f},
                         {x0, y0 + height, 0.0f, 1.0f, 0xFFFFFFFF, 0.0f, 1.0f},

                         {x0 + width, y0, 0.0f, 1.0f, 0xFFFFFFFF, 1.0f, 0.0f},
                         {x0 + width, y0 + height, 0.0f, 1.0f, 0xFFFFFFFF, 1.0f, 1.0f},
                         {x0, y0 + height, 0.0f, 1.0f, 0xFFFFFFFF, 0.0f, 1.0f}};

    device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, vertices, sizeof(Vertex));

    // enb fix
    device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

    stateBlock_->Apply();
}

void RenderManager::AddToEntityTexture(CEntity* entity, const std::string& textureName, IDirect3DTexture9* texture)
{
    if (!entity || !texture)
        return;

    auto rwRaster = RwRasterCreate(0, 0, 8, rwRASTERTYPETEXTURE);
    // RasterExtOffset
    {
        auto offset = *reinterpret_cast<IDirect3DTexture9***>(0xB4E9E0);
        auto& rasterTexture = *reinterpret_cast<IDirect3DTexture9**>((uintptr_t)rwRaster + (uintptr_t)offset);
        rasterTexture = texture;
    }

    auto* rwTexture = RwTextureCreate(rwRaster);
    RwTextureSetName(rwTexture, textureName.c_str());

    entityTexturePool_[entity][textureName] = rwTexture;
}

void RenderManager::RemoveFromEntityTexture(CEntity* entity, const std::string& textureName)
{
    if (!entity)
        return;

    auto it = entityTexturePool_.find(entity);
    if (it == entityTexturePool_.end())
        return;

    auto it2 = it->second.find(textureName);
    if (it2 == it->second.end())
        return;

    RwTextureDestroy(it2->second);
    it2->second = nullptr;

    entityTexturePool_[entity].erase(textureName);
}

void RenderManager::InitHooks()
{
    static kthook::kthook_simple<void(__thiscall*)(CEntity * entity)> entityRenderHook(
        0x534310,
        [this](auto& hook, CEntity* entity)
        {
            CEntityRenderBeforeHook(entity);
            hook.call_trampoline(entity);
            CEntityRenderAfterHook(entity);
        });
}

void RenderManager::CEntityRenderBeforeHook(CEntity* entity)
{
    const auto texturesIt = entityTexturePool_.find(entity);

    if (texturesIt == entityTexturePool_.end())
        return;

    if (entity->m_pRwObject == nullptr)
        return;

    static auto MaterialCallback = [](RpMaterial* material, void* data) -> RpMaterial*
    {
        if (material->texture != nullptr)
        {
            const std::string textureName = RwTextureGetName(material->texture);
            if (textureName.empty())
                return material;

            auto& [renderManager, textures] = *reinterpret_cast<std::pair<RenderManager*, TextureMap>*>(data);
            auto it = textures.find(textureName);
            if (it != textures.end())
            {
                renderManager->AddStoredMaterial(RpMaterialGetTexture(material));
                RpMaterialSetTexture(material, it->second);
            }
        }

        return material;
    };

    auto data = std::make_pair(this, texturesIt->second);
    if (entity->m_pRwObject->type == rpCLUMP)
    {
        RpClumpForAllAtomics(
            entity->m_pRwClump,
            [](RpAtomic* atomic, void* data) -> RpAtomic*
            {
                RpGeometryForAllMaterials(atomic->geometry, MaterialCallback, data);

                return atomic;
            },
            reinterpret_cast<void*>(&data));
    }
    else if (entity->m_pRwObject->type == rpATOMIC)
    {
        RpGeometryForAllMaterials(entity->m_pRwAtomic->geometry, MaterialCallback, reinterpret_cast<void*>(&data));
    }
}

void RenderManager::CEntityRenderAfterHook(CEntity* entity)
{
    for (auto& p : storedMaterials_)
    {
        *p.first = p.second;
    }
    storedMaterials_.clear();
}