#include "LoadStylesheetWorker.h"


LoadStylesheetWorker::LoadStylesheetWorker(const Napi::Function &callback, const std::string &stylesheetPath, xsltStylesheetPtr &stylesheetPtr)
    : Worker(callback), m_StylesheetPath(stylesheetPath), m_StylesheetPtr(stylesheetPtr)
{
}

void LoadStylesheetWorker::Execute()
{
    // If a stylesheet has already been loaded with the current Computron
    // instance, get rid of it first
    if (m_StylesheetPtr)
    {
        xsltFreeStylesheet(m_StylesheetPtr);
        m_StylesheetPtr = nullptr;
    }

    // Ok, let's parse the stylesheet now
    m_StylesheetPtr = xsltParseStylesheetFile((const xmlChar *)m_StylesheetPath.c_str());
    if (!m_StylesheetPtr)
    {
        SetError(GetLastXmlError());
        Cleanup();
    }
}

void LoadStylesheetWorker::OnOK()
{
    const Napi::Value undefined = Env().Undefined();

    Callback().Call({ undefined, undefined });
}

void LoadStylesheetWorker::Cleanup()
{
    if (m_StylesheetPtr)
    {
        xsltFreeStylesheet(m_StylesheetPtr);
        m_StylesheetPtr = nullptr;
    }
}
