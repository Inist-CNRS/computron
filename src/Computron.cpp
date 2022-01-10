#include "Computron.h"

#include <string>
#include <libxslt/xsltutils.h>
#include <libexslt/exslt.h>

#include "LoadStylesheetWorker.h"
#include "ApplyWorker.h"


extern int xmlLoadExtDtdDefaultValue;

static void BlankHandler(void *, const char *, ...)
{
    // Do nothing to prevent libxml from printing the error
    // messages to the standard output. The error messages
    // are passed to the JS environment instead.
}

Computron::Computron(const Napi::CallbackInfo &info)
    : ObjectWrap(info), m_StylesheetPtr(nullptr)
{
    // Register all XSLT extensions
    exsltRegisterAll();

    // Enable entity substitutions
    xmlSubstituteEntitiesDefault(1);

    // Allow loading external entity subsets
    xmlLoadExtDtdDefaultValue = 1;

    // Overwrite the default generic error handler
    xsltSetGenericErrorFunc(nullptr, BlankHandler);
}

Computron::~Computron()
{
    // Free the stylesheet and the XML documents
    Cleanup();

    // Cleanup libxml and libxslt globals
    xsltCleanupGlobals();
    xmlCleanupParser();
}

Napi::Value Computron::LoadStylesheet(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    // Argument checks
    if (info.Length() < 2)
        Napi::Error::New(env, "You need to pass 2 arguments").ThrowAsJavaScriptException();

    if (!info[0].IsString())
        Napi::TypeError::New(env, "String expected as first argument").ThrowAsJavaScriptException();

    if (!info[1].IsFunction())
        Napi::TypeError::New(env, "Callback expected as second argument").ThrowAsJavaScriptException();

    std::string stylesheetPath = info[0].As<Napi::String>().Utf8Value();
    Napi::Function callback = info[1].As<Napi::Function>();

    // This is not a memory leak, the async worker pointer is stored
    // by Node-API and it's deleted later
    LoadStylesheetWorker *loadStylesheetWorker = new LoadStylesheetWorker(callback, stylesheetPath, m_StylesheetPtr);
    loadStylesheetWorker->Queue();

    return env.Undefined();
}

Napi::Value Computron::Apply(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    // Argument checks
    if (info.Length() < 3)
        Napi::Error::New(env, "You need to pass 3 arguments").ThrowAsJavaScriptException();

    if (!info[0].IsString())
        Napi::TypeError::New(env, "String expected as first argument").ThrowAsJavaScriptException();

    if (!info[2].IsFunction())
        Napi::TypeError::New(env, "Callback expected as third argument").ThrowAsJavaScriptException();

    if (!m_StylesheetPtr)
        Napi::Error::New(env, "You need to load a stylesheet first").ThrowAsJavaScriptException();

    std::string xmlDocPath = info[0].As<Napi::String>().Utf8Value();
    Napi::Value params = info[1];
    Napi::Function callback = info[2].As<Napi::Function>();

    // This is not a memory leak, the async worker pointer is stored
    // by Node-API and it's deleted later
    ApplyWorker *applyWorker = new ApplyWorker(callback, xmlDocPath, m_StylesheetPtr, params);
    applyWorker->Queue();

    return env.Undefined();
}

void Computron::Cleanup()
{
    if (m_StylesheetPtr)
    {
        xsltFreeStylesheet(m_StylesheetPtr);
        m_StylesheetPtr = nullptr;
    }
}

Napi::Function Computron::GetClass(Napi::Env env)
{
    return DefineClass(env, "Computron",
    {
        InstanceMethod("loadStylesheet", &Computron::LoadStylesheet),
        InstanceMethod("apply", &Computron::Apply)
    });
}
