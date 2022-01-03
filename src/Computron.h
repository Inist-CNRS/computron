#pragma once

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
    // Asynchronously load a stylesheet from a path.
    Napi::Value LoadStylesheet(const Napi::CallbackInfo &info);

    // Asynchronously apply the previously loaded stylesheet.
    Napi::Value Apply(const Napi::CallbackInfo &info);

    // Free the memory allocated by libxml.
    void Cleanup();
private:
    // Pointer to the stylesheet.
    xsltStylesheet *m_StylesheetPtr;
};
