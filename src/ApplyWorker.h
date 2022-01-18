#pragma once

#include <string>
#include <vector>
#include <libxslt/xsltInternals.h>

#include "Worker.h"


// Worker that applies a stylesheet asynchronously.
class ApplyWorker : public Worker
{
public:
    ApplyWorker(const Napi::Function &callback, const std::string &xmlDocPath, xsltStylesheet *stylesheetPtr, const Napi::Value &params);

    // The long task to execute.
    void Execute() override;

    // The event listener called on success.
    void OnOK() override;
private:
    // Free the memory allocated by libxslt.
    void Cleanup() override;

    // Build the vector of strings holding the params for the stylesheet.
    std::vector<std::string> BuildParams(const Napi::Object &paramsObj);
private:
    // Absolute path to the XML document.
    std::string m_XmlDocPath;

    // Pointer to the stylesheet.
    xsltStylesheet *m_StylesheetPtr;

    // Pointer to the XML input document.
    xmlDoc *m_InputXmlDocPtr;

    // Pointer to the XML document formatted with the stylesheet.
    xmlDoc *m_OutputXmlDocPtr;

    // The content of the XML document formatted with the stylesheet as a string.
    std::string m_Result;

    // Parameters passed to the stylesheet as a vector of strings.
    std::vector<std::string> m_Params;
};
