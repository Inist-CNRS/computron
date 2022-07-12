#include "Computron.h"

#include "Utils.h"

#include <libxslt/xsltutils.h>
#include <libexslt/exslt.h>
#include <libxslt/transform.h>

#include <iostream>


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
    // Free the stylesheet
    if (m_StylesheetPtr)
        xsltFreeStylesheet(m_StylesheetPtr);

    // Cleanup libxml and libxslt globals
    xsltCleanupGlobals();
    xmlCleanupParser();
}

Napi::Value Computron::LoadStylesheet(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::Value undefined = env.Undefined();

    // Argument checks
    if (info.Length() < 1 || !info[0].IsString())
    {
        Napi::TypeError::New(env, "String expected as first argument").ThrowAsJavaScriptException();
        return undefined;
    }

    std::string stylesheetPath = info[0].As<Napi::String>().Utf8Value();

    // If a stylesheet has already been loaded with the current Computron
    // instance, get rid of it first
    if (m_StylesheetPtr)
    {
        xsltFreeStylesheet(m_StylesheetPtr);
        m_StylesheetPtr = nullptr;
    }

    // Ok, let's parse the stylesheet now
    m_StylesheetPtr = xsltParseStylesheetFile(reinterpret_cast<const xmlChar *>(stylesheetPath.c_str()));
    if (!m_StylesheetPtr)
        Napi::Error::New(env, Utils::GetLastXmlError()).ThrowAsJavaScriptException();

    return undefined;
}

Napi::Value Computron::Apply(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::Value null = env.Null();

    // Argument checks
    if (info.Length() < 1 || !info[0].IsString())
    {
        Napi::TypeError::New(env, "String expected as first argument").ThrowAsJavaScriptException();
        return null;
    }

    if (!m_StylesheetPtr)
    {
        Napi::Error::New(env, "You need to load a stylesheet first").ThrowAsJavaScriptException();
        return null;
    }

    std::string xmlDocPath = info[0].As<Napi::String>().Utf8Value();
    Napi::Value params = info[1];

    // The errors and warnings are not completely ignored, they're just not
    // printed to the standard output
    xmlDoc *inputXmlDocPtr = xmlReadFile(xmlDocPath.c_str(), NULL, XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
    if (!inputXmlDocPtr)
    {
        Napi::Error::New(env, Utils::GetLastXmlError()).ThrowAsJavaScriptException();
        return null;
    }

    xmlDoc *outputXmlDocPtr = nullptr;

    // If parameters were passed, build them for libxml
    if (params.IsObject())
    {
        std::vector<std::string> builtParams = BuildParams(params.As<Napi::Object>());

        if (!builtParams.empty())
        {
            std::vector<const char *> paramsForLibxml;

            // The vector of const char * needs to contain the amount of elements
            // in the vector of strings + 1 because it needs to be NULL-terminated
            paramsForLibxml.reserve(builtParams.size() + 1);

            // Build the vector of const char * from the vector of strings
            for (const auto &param : builtParams)
                paramsForLibxml.emplace_back(param.c_str());

            paramsForLibxml.emplace_back(static_cast<const char *>(NULL));

            outputXmlDocPtr = xsltApplyStylesheet(m_StylesheetPtr, inputXmlDocPtr, paramsForLibxml.data());
        }
    }
    else
        outputXmlDocPtr = xsltApplyStylesheet(m_StylesheetPtr, inputXmlDocPtr, nullptr);

    if (!outputXmlDocPtr)
    {
        Napi::Error::New(env, Utils::GetLastXmlError()).ThrowAsJavaScriptException();

        xmlFreeDoc(inputXmlDocPtr);

        return null;
    }

    // Save the result to a string for the JS environment
    xmlChar *tranformResult = nullptr;
    int transformResultLength = 0;

    xsltSaveResultToString(&tranformResult, &transformResultLength, outputXmlDocPtr, m_StylesheetPtr);
    if (!tranformResult || !transformResultLength)
    {
        Napi::Error::New(env, Utils::GetLastXmlError()).ThrowAsJavaScriptException();

        xmlFreeDoc(inputXmlDocPtr);
        xmlFreeDoc(outputXmlDocPtr);

        return null;
    }

    std::string resultForJSEnvironment = reinterpret_cast<const char *>(tranformResult);

    xmlFreeDoc(inputXmlDocPtr);
    xmlFreeDoc(outputXmlDocPtr);

    return Napi::String::New(env, resultForJSEnvironment);
}

std::vector<std::string> Computron::BuildParams(const Napi::Object &paramsObj)
{
    std::vector<std::string> params;
    Napi::Array keys = paramsObj.GetPropertyNames();

    // Preallocate enough memory for the params, each key has a value
    // so the vector will contain the number of keys * 2
    params.reserve(keys.Length() * 2);

    // Build the vector of strings
    for (uint32_t i = 0; i < keys.Length(); i++)
    {
        std::string key = keys.Get(i).As<Napi::String>().Utf8Value();
        params.emplace_back(key);

        // Apparently the values need to be between double-quotes, I don't know why...
        std::string value = "\"" + paramsObj.Get(key).As<Napi::String>().Utf8Value() + "\"";
        params.emplace_back(value);
    }

    return params;
}

Napi::Function Computron::GetClass(Napi::Env env)
{
    return DefineClass(env, "Computron",
    {
        InstanceMethod("loadStylesheet", &Computron::LoadStylesheet),
        InstanceMethod("apply", &Computron::Apply)
    });
}
