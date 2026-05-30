#pragma once

class IWebView
{
  public:
    virtual ~IWebView() = default;

    virtual void Initialize() = 0;
    virtual void Render() = 0;

    [[nodiscard]] virtual void* GetTexture() const = 0;
};