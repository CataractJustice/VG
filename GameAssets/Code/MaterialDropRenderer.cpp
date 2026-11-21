#include "MaterialDropRenderer.h"
#include "Kirino/Mesh.h"
#include "Core.h"
Kirino::Material* materialDropMaterial;
Kirino::Mesh* materialDropMesh;

void MaterialDropRenderer::init(unsigned int materialAtlas, glm::vec2 atlasScale) 
{
	materialDropMaterial = const_cast<Kirino::Material*>(&Core::materialsManager.get("material_drop"));
	materialDropMesh = &Core::modelsManager.get("material_drop.obj");
	materialDropMaterial->setTexture("textureImage", &materialAtlas);
	materialDropMaterial->setUniform("atlasScale", &atlasScale, sizeof(atlasScale), 0);
	float texScale = 1.0f;
	materialDropMaterial->setUniform("texScale", &texScale, sizeof(texScale), 0);
}

void MaterialDropRenderer::render(Kirino::StateColletion& state, const FrustumPlanes& frustum, const RenderTexturesCollection& renderTextures) 
{
	const Transform& t = ((GameNode3D*)this->getParent())->getWorldTransform();
	glm::vec3 cameraPosition = Core::camera->getParentOfTypeConst<GameNode3D>()->getWorldTransform().getPosition();

	//setup transform
	TransformBlock tb = {t.getMatrix(), t.getNormalsModel()};
	Core::transformUBO.upload(tb);
	materialDropMaterial->setUniform("materialId", &materialId, sizeof(materialId), 0);
	materialDropMaterial->useProgram();
	state.shader = materialDropMaterial->getShaderProgram();
	materialDropMaterial->setupTextures(state.textureBindings);
	materialDropMaterial->setupUniformBlocks(state.uboBindings);
	materialDropMaterial->setupUniforms();
	//Draw mesh
	glBindVertexArray(*materialDropMesh->vao);
	glDrawElementsBaseVertex(GL_TRIANGLES, materialDropMesh->indexCount, GL_UNSIGNED_INT, 0, materialDropMesh->vertexOffset);
}