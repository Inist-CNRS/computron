#include "ApplyWorker.h"

#include <libxslt/xsltutils.h>
#include <libxslt/transform.h>


ApplyWorker::ApplyWorker(const Napi::Function &callback, const std::string &xmlDocPath, xsltStylesheetPtr &stylesheetPtr, const Napi::Value &params)
    : Worker(callback), m_XmlDocPath(xmlDocPath), m_StylesheetPtr(stylesheetPtr), m_InputXmlDocPtr(nullptr), m_OutputXmlDocPtr(nullptr)
{
    // If parameters were passed, build them and save them in m_Params
    if (params.IsObject())
        m_Params = BuildParams(params.As<Napi::Object>());
}

void ApplyWorker::Execute()
{
    // If an XML document has already been parsed with the current Computron
    // instance, get rid of it first
    if (m_InputXmlDocPtr)
    {
        xmlFreeDoc(m_InputXmlDocPtr);
        m_InputXmlDocPtr = nullptr;
    }

    // The errors and warnings are not completely ignored, they're just not
    // printed to the standard output
    m_InputXmlDocPtr = xmlReadFile(m_XmlDocPath.c_str(), NULL, XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
    if (!m_InputXmlDocPtr)
    {
        SetError(GetLastXmlError());
        Cleanup();

        return;
    }

    // If an XML document has already been created with the current Computron
    // instance, get rid of it first
    if (m_OutputXmlDocPtr)
    {
        xmlFreeDoc(m_OutputXmlDocPtr);
        m_OutputXmlDocPtr = nullptr;
    }

    if (!m_Params.empty())
    {
        std::vector<const char *> params;

        // The vector of const char * needs to contain the amount of elements
        // in the vector of strings + 1 because it needs to be NULL-terminated
        params.reserve(m_Params.size() + 1);

        // Build the vector of const char * from the vector of strings
        for (const auto &param : m_Params)
            params.emplace_back(param.c_str());

        params.emplace_back(reinterpret_cast<const char *>(NULL));

        m_OutputXmlDocPtr = xsltApplyStylesheet(m_StylesheetPtr, m_InputXmlDocPtr, params.data());
    }
    else
        m_OutputXmlDocPtr = xsltApplyStylesheet(m_StylesheetPtr, m_InputXmlDocPtr, nullptr);

    if (!m_OutputXmlDocPtr)
    {
        SetError(GetLastXmlError());
        Cleanup();

        return;
    }

    // Save the result to a string for the JS environment
    xmlChar *result = nullptr;
    int resultLength = 0;

    xsltSaveResultToString(&result, &resultLength, m_OutputXmlDocPtr, m_StylesheetPtr);
    if (!result || !resultLength)
    {
        SetError(GetLastXmlError());
        Cleanup();

        return;
    }

    m_Result = reinterpret_cast<const char *>(result);
}

void ApplyWorker::OnOK()
{
    Callback().Call({ Env().Undefined(), Napi::String::New(Env(), m_Result) });
}

void ApplyWorker::Cleanup()
{
    if (m_InputXmlDocPtr)
    {
        xmlFreeDoc(m_InputXmlDocPtr);
        m_InputXmlDocPtr = nullptr;
    }

    if (m_OutputXmlDocPtr)
    {
        xmlFreeDoc(m_OutputXmlDocPtr);
        m_OutputXmlDocPtr = nullptr;
    }
}

std::vector<std::string> ApplyWorker::BuildParams(const Napi::Object &paramsObj)
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
