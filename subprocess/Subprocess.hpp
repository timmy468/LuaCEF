#pragma once

#include <iostream>

#include <cef_app.h>
#include <cef_browser.h>

class Subprocess : public CefApp, public CefRenderProcessHandler
{

  public:
    Subprocess() = default;
    ~Subprocess() = default;

    virtual void OnBeforeCommandLineProcessing(const CefString& process_type,
                                               CefRefPtr<CefCommandLine> command_line) override
    {
        command_line->AppendSwitch("disable-gpu");
        command_line->AppendSwitch("disable-gpu-vsync");
        command_line->AppendSwitch("enable-begin-frame-scheduling");

        command_line->AppendSwitchWithValue("autoplay-policy", "no-user-gesture-required");

        command_line->AppendSwitch("enable-media-stream");
        command_line->AppendSwitch("no-proxy-server");

        command_line->AppendSwitchWithValue("allow-browser-signin", "false");

        if (process_type.empty())
        {
            command_line->AppendSwitchWithValue("autoplay-policy", "no-user-gesture-required");
            command_line->AppendSwitchWithValue("enable-blink-features", "ShadowDOMV0,CustomElementsV0,HTMLImports");
        }
    }

    virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override
    {
        return this;
    }

    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefV8Context> context) override
    {
    }

    virtual void OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                   CefRefPtr<CefV8Context> context) override
    {
    }

  private:
    IMPLEMENT_REFCOUNTING(Subprocess);
};