#include "Worker.h"

#include <sstream>
#include <libxml/xmlerror.h>


Worker::Worker(const Napi::Function &callback)
    : Napi::AsyncWorker(callback)
{
}

Worker::~Worker()
{
}

void Worker::OnError(const Napi::Error &error)
{
    Callback().Call({ error.Value() });
}

std::string Worker::GetLastXmlError()
{
    std::string result;

    xmlError *err = xmlGetLastError();
    if (!err)
        return result;

    std::stringstream stream;

    if (err->message)
        stream << err->message;

    if (err->file)
        stream << "file: " << err->file << '\n';

    if (err->line)
        stream << "line: " << err->line << '\n';

    result = stream.str();

    return result;
}
