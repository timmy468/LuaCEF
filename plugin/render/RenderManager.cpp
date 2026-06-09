#include "RenderManager.hpp"

#include <plugin.h>

struct Vertex
{
    float x, y, z, rhw;
    float u, v;
};

#define FVF_VERTEX (D3DFVF_XYZRHW | D3DFVF_TEX1)

void RenderManager::Initialize()
{
}

void RenderManager::Shutdown()
{
}

void RenderManager::RenderTexture(IDirect3DTexture9* texture, float x, float y, float width, float height)
{
    auto* device = GetD3DDevice();
    if (!device || !texture)
        return;

    Vertex vertices[] = {{x, y, 0.0f, 1.0f, 0.0f, 0.0f},          {x + width, y, 0.0f, 1.0f, 1.0f, 0.0f},
                         {x, y + height, 0.0f, 1.0f, 0.0f, 1.0f},

                         {x + width, y, 0.0f, 1.0f, 1.0f, 0.0f},  {x + width, y + height, 0.0f, 1.0f, 1.0f, 1.0f},
                         {x, y + height, 0.0f, 1.0f, 0.0f, 1.0f}};

    device->SetTexture(0, texture);

    device->SetFVF(FVF_VERTEX);

    device->SetRenderState(D3DRS_LIGHTING, FALSE);
    device->SetRenderState(D3DRS_ZENABLE, FALSE);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

    device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

    device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, vertices, sizeof(Vertex));
}