#pragma once

#include "Config.h"

#include <glm/glm.hpp>

#define LOG(X) ::GP::Logger::Get()->ConsoleLog(X)
#define POPUP(X) ::GP::Logger::Get()->PopupLog(X)

#define SAFE_DELETE(X) if((X)) { delete (X); (X) = nullptr; }
#define UNUSED(x) (void)(x)

#ifdef DEBUG
#define ASSERT(X,msg) if(!(X)) { POPUP("ASSERT: " msg); __debugbreak(); }
#define NOT_IMPLEMENTED ASSERT(0, "NOT IMPLEMENTED")
#else
#define ASSERT(X, MSG)
#define NOT_IMPLEMENTED
#endif // DEBUG

#define MIN(X,Y) ((X) < (Y)) ? (X) : (Y)
#define MAX(X,Y) ((X) > (Y)) ? (X) : (Y)
#define CLAMP(X,min,max) (MIN(MAX(X,min),max));

#define JOIN(X,Y) X##Y

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;

using Mat3 = glm::mat3;
using Mat4 = glm::mat4;

struct ColorUNORM
{
	ColorUNORM(): r(0), g(0), b(0), a(0) {}
	ColorUNORM(unsigned char r, unsigned char g, unsigned char b, unsigned char a): r(r), g(g), b(b), a(a) {}
	ColorUNORM(Vec4 colorFloat): r(toU8(colorFloat.x)), g(toU8(colorFloat.y)), b(toU8(colorFloat.z)), a(toU8(colorFloat.w)) {}

	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;

private:
	static unsigned char toU8(float x) { return (unsigned char)(255.0f * x); }
};

static_assert(sizeof(Vec2) == sizeof(float) * 2);
static_assert(sizeof(Vec3) == sizeof(float) * 3);
static_assert(sizeof(Vec4) == sizeof(float) * 4);

static_assert(sizeof(Mat3) == sizeof(float) * 3 * 3);
static_assert(sizeof(Mat4) == sizeof(float) * 4 * 4);

static_assert(sizeof(ColorUNORM) == 4);

#define VEC2_ZERO Vec2(0.0f,0.0f)
#define VEC2_ONE Vec2(1.0f,1.0f)

#define VEC3_ZERO Vec3(0.0f,0.0f,0.0f)
#define VEC3_ONE Vec3(1.0f,1.0f,1.0f)

#define VEC4_ZERO Vec4(0.0f,0.0f,0.0f,0.0f)

#define MAT4_IDENTITY Mat4(1.0)
#define MAT3_IDENTITY Mat3(1.0)

#define DELETE_COPY_CONSTRUCTOR(X) \
X(X const&) = delete; \
X& operator=(X const&) = delete;

#ifdef _GP
#define GP_DLL __declspec(dllexport)
#else
#define GP_DLL __declspec(dllimport)
#endif // _GP

#include "debug/Logger.h"