#pragma once

#include "gfx/GfxCommon.h"
#include "gfx/GfxBuffers.h"

namespace GP
{
	template<typename T> class GfxConstantBuffer;
	class GfxContext;

	struct CBCamera
	{
		Mat4 view;
		Mat4 projection;
		Vec3 position;
		Mat4 viewInv;
		Mat4 projectionInv;
	};

	class Camera
	{
		DELETE_COPY_CONSTRUCTOR(Camera);
	public:
		Camera()
		{
			SetProjectionPerspective(45.0f, 18.0f / 10.0f, 0.1f, 10000.0f);
			SetPosition(Vec3(0.0f, 0.0f, 0.0f));
			LookAt(Vec3(0.0f, 1.0f, 0.0f));
		}

		GP_DLL void SetProjectionPerspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane);

		GP_DLL void SetPosition(const Vec3 position);
		GP_DLL void SetRotation(const Vec3 rotation);
		GP_DLL void LookAt(const Vec3& point);

		inline const CBCamera& GetData() const { return m_Data; }
		inline Vec3 GetPosition() const { return m_Position; }
		inline Vec3 GetRotation() const { return m_Rotation; }

		inline GfxConstantBuffer<CBCamera>* GetBuffer(GfxContext* context) 
		{
			if (m_Dirty)
			{
				UpdateBuffer(context);
				m_Dirty = false;
			}
			return &m_Buffer; 
		}

		inline Vec3 RelativeToView(const Vec3 direction) const
		{
			Vec4 dir = Vec4(direction.x, direction.y, direction.z, 0.0f);
			Vec4 relativeDir = dir * m_Data.view;
			return Vec3(relativeDir.x, relativeDir.y, relativeDir.z);
		}

	private:
		void PositionChanged();
		void RotationChanged();
		void AxisChanged();
		GP_DLL void UpdateBuffer(GfxContext* context);

	private:
		bool m_Dirty = true;

		CBCamera m_Data;
		GfxConstantBuffer<CBCamera> m_Buffer;

		Vec3 m_Position = VEC3_ZERO;
		Vec3 m_Rotation = VEC3_ZERO; // (pitch,yaw,roll)

		Vec3 m_Forward = { 0.0f,0.0f, -1.0f };
		Vec3 m_Right = { 1.0f,0.0f, 0.0f };
		Vec3 m_Up = { 0.0f,1.0f,0.0f };
	};

	class ModelTransform
	{
		DELETE_COPY_CONSTRUCTOR(ModelTransform);
	public:
		ModelTransform() = default;

	private:
		GP_DLL void UpdateBuffer(GfxContext* context);

	public:
		inline Vec3 GetPosition() const { return m_Position; }
		inline Vec3 GetRotation() const { return m_Rotation; }
		inline Vec3 GetScale() const { return m_Scale; }

		inline void SetPosition(Vec3 position) { m_Position = position; m_Dirty = true; }
		inline void SetRotation(Vec3 rotation) { m_Rotation = rotation;  m_Dirty = true; }
		inline void SetScale(Vec3 scale) { m_Scale = scale;  m_Dirty = true; }

		inline GfxConstantBuffer<Mat4>* GetBuffer(GfxContext* context)
		{
			if (m_Dirty)
			{
				UpdateBuffer(context);
				m_Dirty = false;
			}
			return &m_Buffer; 
		}

	private:
		bool m_Dirty = true;

		Mat4 m_Data = MAT4_IDENTITY;
		GfxConstantBuffer<Mat4> m_Buffer;

		Vec3 m_Position = VEC3_ZERO;
		Vec3 m_Rotation =  VEC3_ZERO; // (pitch, yaw, roll)
		Vec3 m_Scale = VEC3_ONE;
	};
}
