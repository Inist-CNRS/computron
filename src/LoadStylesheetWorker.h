#pragma once

#include <string>
#include <libxslt/xsltInternals.h>

#include "Worker.h"


// Worker that loads a stylesheet asynchronously.
class LoadStylesheetWorker : public Worker
{
public:
    LoadStylesheetWorker(const Napi::Function &callback, const std::string &stylesheetPath, xsltStylesheetPtr &stylesheetPtr);

    // The long task to execute.
    void Execute() override;

    // The event listener called on success.
    void OnOK() override;
private:
    // Free the memory allocated by libxslt.
    void Cleanup() override;
private:
    // Absolute path to the stylesheet file.
    std::string m_StylesheetPath;

    // Reference to a pointer to the stylesheet.
    xsltStylesheetPtr &m_StylesheetPtr;
};
