#include "LuaBinding.hpp"

LuaBinding::LuaBinding(sol::table module, std::shared_ptr<IWebCore> webCore) : module_(module), webCore_(webCore)
{
}

void LuaBinding::Bind()
{
    module_.set_function("version", []() { return "0.1.0"; });

    module_.set_function("inited", [this]() { return webCore_ && webCore_->IsInitialized(); });

    // module_.set_function("input_active", []() { return false; });

    module_.set_function("create",
                         [this](const std::string& url, int x, int y, int width, int height)
                         {
                             // if (!webCore_)
                             //     return sol::nil;

                             // auto webView = webCore_->CreateWebView(url, x, y, width, height);
                             // if (!webView)
                             //     return sol::nil;

                             // return webView;
                         });

    module_.set_function("create_fullscreen",
                         [this](const std::string& url)
                         {
                             // if (!webCore_)
                             //     return sol::nil;

                             // auto webView = webCore_->CreateFullscreenWebView(url);
                             // if (!webView)
                             //     return sol::nil;

                             // return webView;
                         });

    module_.set_function("close", [this](std::shared_ptr<IWebView> webView) { webCore_->CloseWebView(webView); });

    // webcore.version() -- возвращает версию плагина
    // webcore.inited() -- возвращает true, если инициализирован
    // webcore.input_active() -- возвращает true, если активен ввод в любом фрейме (даже в чужом)
    // WebFrame webcore:create(url, x, y, w, h[, cb]) -- создаёт фрейм с указанными параметрами и вызывает callback
    // после создания браузера WebFrame webcore:create_fullscreen(url[, cb]) -- создаёт полноэкранный фрейм
    // webcore:close(WebFrame) -- закрывает фрейм и удаляет связанные с ним данные
}