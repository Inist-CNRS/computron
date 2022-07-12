#pragma once

#include <string>
#include <vector>
#include <napi.h>
#include <libxslt/xsltInternals.h>


class Computron : public Napi::ObjectWrap<Computron>
{
public:
    Computron(const Napi::CallbackInfo &info);
    ~Computron();

    // Define the class that will get passed to the JS environment.
    static Napi::Function GetClass(Napi::Env env);
private:
    // Load a stylesheet from a path.
    Napi::Value LoadStylesheet(const Napi::CallbackInfo &info);

    // Apply the previously loaded stylesheet.
    Napi::Value Apply(const Napi::CallbackInfo &info);

    // Build the vector of strings holding the params for the stylesheet.
    std::vector<std::string> BuildParams(const Napi::Object &paramsObj);
private:
    // Pointer to the stylesheet.
    xsltStylesheet *m_StylesheetPtr;
};
