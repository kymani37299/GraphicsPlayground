#pragma once

#include <d3d11_1.h>

#include "core/Core.h"

#define DX_CALL(X) {HRESULT hr = X; ASSERT(SUCCEEDED(hr), "DX ERROR " __FILE__);}
#define SAFE_RELEASE(X) if(X) { X->Release(); X = nullptr; }