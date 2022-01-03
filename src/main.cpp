#include "Computron.h"


Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    Napi::Function func = Computron::GetClass(env);

    // This is not a memory leak, the function reference pointer is stored
    // by Node-API and the memory is freed later
    Napi::FunctionReference *constructor = new Napi::FunctionReference();
    *constructor = Napi::Persistent(func);
    exports.Set("Computron", func);

    env.SetInstanceData<Napi::FunctionReference>(constructor);

    return exports;
}

NODE_API_MODULE(addon, Init)
