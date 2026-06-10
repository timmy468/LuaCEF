#include "RenderManager.hpp"

#include <plugin.h>

struct Vertex
{
    float x, y, z, rhw;
    D3DCOLOR color;
    float u, v;
};

#define FVF_VERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

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

    IDirect3DStateBlock9* stateBlock{};

    if (FAILED(device->CreateStateBlock(D3DSBT_ALL, &stateBlock)))
        return;

    stateBlock->Capture();

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

    device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    device->SetFVF(FVF_VERTEX);
    device->SetTexture(0, texture);

    Vertex vertices[] = {{x, y, 0.0f, 1.0f, 0xFFFFFFFF, 0.0f, 0.0f},
                         {x + width, y, 0.0f, 1.0f, 0xFFFFFFFF, 1.0f, 0.0f},
                         {x, y + height, 0.0f, 1.0f, 0xFFFFFFFF, 0.0f, 1.0f},

                         {x + width, y, 0.0f, 1.0f, 0xFFFFFFFF, 1.0f, 0.0f},
                         {x + width, y + height, 0.0f, 1.0f, 0xFFFFFFFF, 1.0f, 1.0f},
                         {x, y + height, 0.0f, 1.0f, 0xFFFFFFFF, 0.0f, 1.0f}};

    device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, vertices, sizeof(Vertex));

    // enb fix
    device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

    stateBlock->Apply();
    stateBlock->Release();
}