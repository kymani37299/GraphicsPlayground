#pragma once

#include "Common.h"

#define DX_CALL(X) {HRESULT hr = X; ASSERT(SUCCEEDED(hr), "DX ERROR " __FILE__);}
#define SAFE_RELEASE(X) if(X) { X->Release(); X = nullptr; }