#pragma once

#include <sstream>
#include <string>
#include <vector>

#include <cef_v8.h>

namespace V8Helpers
{
static std::string EscapeJson(const std::string& str)
{
    std::string result;

    for (char c : str)
    {
        switch (c)
        {
        case '"':
            result += "\\\"";
            break;
        case '\\':
            result += "\\\\";
            break;
        case '\n':
            result += "\\n";
            break;
        case '\r':
            result += "\\r";
            break;
        case '\t':
            result += "\\t";
            break;
        default:
            result += c;
            break;
        }
    }

    return result;
}

static std::string V8ToJson(CefRefPtr<CefV8Value> value)
{
    if (!value)
        return "null";

    if (value->IsNull() || value->IsUndefined())
    {
        return "null";
    }

    if (value->IsBool())
    {
        return value->GetBoolValue() ? "true" : "false";
    }

    if (value->IsInt())
    {
        return std::to_string(value->GetIntValue());
    }

    if (value->IsUInt())
    {
        return std::to_string(value->GetUIntValue());
    }

    if (value->IsDouble())
    {
        return std::to_string(value->GetDoubleValue());
    }

    if (value->IsString())
    {
        return "\"" + EscapeJson(value->GetStringValue()) + "\"";
    }

    if (value->IsArray())
    {
        std::string json = "[";

        int length = value->GetArrayLength();

        for (int i = 0; i < length; ++i)
        {
            if (i > 0)
                json += ",";

            json += V8ToJson(value->GetValue(i));
        }

        json += "]";

        return json;
    }

    if (value->IsObject())
    {
        std::string json = "{";

        std::vector<CefString> keys;

        value->GetKeys(keys);

        bool first = true;

        for (auto& key : keys)
        {
            if (!first)
                json += ",";

            first = false;

            json += "\"";
            json += EscapeJson(key);
            json += "\":";

            json += V8ToJson(value->GetValue(key));
        }

        json += "}";

        return json;
    }

    return "null";
}

class LuaV8Handler : public CefV8Handler
{
  public:
    bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments,
                 CefRefPtr<CefV8Value>& retval, CefString& exception) override
    {
        auto context = CefV8Context::GetCurrentContext();

        auto browser = context->GetBrowser();

        auto message = CefProcessMessage::Create("BindFunctionCall");

        auto args = message->GetArgumentList();

        std::string json = "[";

        for (size_t i = 1; i < arguments.size(); ++i)
        {
            if (i > 1)
                json += ",";

            json += V8ToJson(arguments[i]);
        }

        json += "]";

        args->SetString(0, arguments[0]->GetStringValue());

        args->SetString(1, json);

        browser->GetMainFrame()->SendProcessMessage(PID_BROWSER, message);

        retval = CefV8Value::CreateUndefined();

        return true;
    }

    IMPLEMENT_REFCOUNTING(LuaV8Handler);
};
} // namespace V8Helpers