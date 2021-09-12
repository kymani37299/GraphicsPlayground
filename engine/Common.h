#pragma once

#include "Config.h"

#include <glm/glm.hpp>

#ifdef DEBUG
#include "debug/Logger.h"
#else
#define LOG(X)
#define POPUP(X)
#endif

#define SAFE_DELETE(X) if((X)) { delete (X); (X) = nullptr; }

#ifdef DEBUG
#define ASSERT(X,msg) if(!(X)) { POPUP("ASSERT: " msg); __debugbreak(); }
#define NOT_IMPLEMENTED ASSERT(0, "NOT IMPLEMENTED")
#else
#define ASSERT(X, MSG)
#define NOT_IMPLEMENTED
#endif // DEBUG

#define MIN(X,Y) ((X) < (Y)) ? (X) : (Y)
#define MAX(X,Y) ((X) > (Y)) ? (X) : (Y)

#define JOIN(X,Y) X##Y

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;

using Mat3 = glm::mat3;
using Mat4 = glm::mat4;

#define VEC2_ZERO Vec2(0.0f,0.0f)
#define VEC2_ONE Vec2(1.0f,1.0f)

#define VEC3_ZERO Vec3(0.0f,0.0f,0.0f)
#define VEC3_ONE Vec3(1.0f,1.0f,1.0f)

#define VEC4_ZERO Vec4(0.0f,0.0f,0.0f,0.0f)

#define RMAT4_IDENTITY RMat4(Mat4(1.0))
#define MAT4_IDENTITY Mat4(1.0)
#define MAT3_IDENTITY Mat3(1.0)

#define DELETE_COPY_CONSTRUCTOR(X) \
X(X const&) = delete; \
X& operator=(X const&) = delete;

#ifdef ENGINE
#define ENGINE_DLL __declspec(dllexport)
#else
#define ENGINE_DLL __declspec(dllimport)
#endif // ENGINE