#include "SceneObject.h"

#include "glm/gtc/matrix_transform.hpp"

#include "core/GfxCore.h"

#include "util/ModelLoading.h"

#include "scene/Mesh.h"
#include "scene/Material.h"

namespace GP
{
	inline Mat4 SceneObject::GetTransformationMatrix(const Transform& transform)
	{
		// TODO : Rotation
		Mat4 model = glm::translate(MAT4_IDENTITY, transform.position);
		model = glm::scale(model, Vec3(transform.scale));
		return model;
	}

	SceneObject::SceneObject(const ModelLoading::ObjectData& data)
	{
		m_Mesh = new Mesh(*data.mesh);
		m_Material = new Material(data.material);
		m_InstanceBuffer = new GfxConstantBuffer<CBInstanceData>();
		UpdateInstance();
	}

	SceneObject::~SceneObject()
	{
		delete m_Mesh;
		delete m_Material;
		delete m_InstanceBuffer;
	}

	void SceneObject::UpdateInstance()
	{
		Mat4 model = GetTransformationMatrix(m_Transform);
		static CBInstanceData instanceData = {};
		instanceData.model = model;
		m_InstanceBuffer->Upload(instanceData);
	}
}