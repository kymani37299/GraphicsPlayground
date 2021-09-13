#pragma once

#include <Common.h>

namespace GP
{
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
	public:
		Camera();

		void SetPosition(const Vec3 position);
		void SetRotation(const Vec3 rotation);
		void LookAt(const Vec3& point);

		inline const CBCamera& GetData() const { return m_Data; }
		inline Vec3 GetPosition() const { return m_Position; }
		inline Vec3 GetRotation() const { return m_Rotation; }

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
		void UpdateView();

	private:
		CBCamera m_Data;
		Vec3 m_Position = VEC3_ZERO;
		Vec3 m_Rotation = VEC3_ZERO; // (pitch,yaw,roll)

		Vec3 m_Forward = { 0.0f,0.0f, -1.0f };
		Vec3 m_Right = { 1.0f,0.0f, 0.0f };
		Vec3 m_Up = { 0.0f,1.0f,0.0f };
	};
}
