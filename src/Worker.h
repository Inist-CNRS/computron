#pragma once

#include <napi.h>


// Abstract worker class that groups the common functionalities between the workers.
class Worker : public Napi::AsyncWorker
{
public:
    Worker(const Napi::Function &callback);
    virtual ~Worker();

    // The event listener called on success.
    virtual void OnOK() = 0;

    // The event listener called on failure.
    void OnError(const Napi::Error &error) override;
protected:
    // Build a string from the last xmlError generated by libxml.
    static std::string GetLastXmlError();
private:
    virtual void Cleanup() = 0;
};