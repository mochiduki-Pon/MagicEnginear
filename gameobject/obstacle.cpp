#include	"obstacle.h"	
#include    "../system/CDirectInput.h"
#include	"../system/meshmanager.h"
#include	"../system/collision.h"

void obstacle::init() 
{
	m_mesh = MeshManager::getMesh<CStaticMesh>("obstaclebox");
	m_shader = MeshManager::getShader<CShader>("unlightshader");
	m_meshrenderer = MeshManager::getRenderer<CStaticMeshRenderer>("obstaclebox");

	std::vector<Vector3> verts{};

	for (auto& v : m_mesh->GetVertices()) {
		verts.push_back(v.Position);
	}

	Call::Collision::BoundingBoxAABB aabb = Call::Collision::calcAABB(verts, SRT{});

	Vector3 size =  aabb.max - aabb.min;

	m_width = fabs(size.x);
	m_height = fabs(size.y);
	m_depth = fabs(size.z);
}

void obstacle::update(uint64_t dt) {


}

void obstacle::draw(uint64_t dt) {


	Matrix4x4 mtx = m_srt.GetMatrix();

	Renderer::SetWorldMatrix(&mtx);

	m_shader->SetGPU();
	m_meshrenderer->Draw();

}

void obstacle::dispose() {

}
