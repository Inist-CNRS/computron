#include "transformer.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  return Transformer::Init(env, exports);
}

NODE_API_MODULE(addon, InitAll)
