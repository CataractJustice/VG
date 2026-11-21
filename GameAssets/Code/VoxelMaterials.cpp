#include "VoxelMaterials.h"
#include <filesystem>
#include "GameNode/GameNode.h"
#include "GameNode/GameNode3D.h"
#include "Core.h"
#include "Graphics/Mesh/MeshRenderer.h"
#include "MaterialDropRenderer.h"
#include "Inventory/InventoryItem.h"
#include "Entity/Entities/ItemDrop.h"
#include "Entity/EntityTemplates.h"
#include "World.h"
VoxelMaterials* VoxelMaterials::instance = nullptr;
unsigned char* voxelMaterialItemTextures = nullptr;
const ItemDrop* itemDropEntityTemplate = nullptr;

class MaterialItemDropBreakHandler : public BreakEventHandler
{
	private:

	public:
	void invoke(glm::vec3 position, VoxelMaterial material) override 
	{
		if(itemDropEntityTemplate == nullptr) 
		{
			itemDropEntityTemplate = (const ItemDrop*)EntityTemplates::getInstance()->getTemplate("item");
		}
		InventoryItem item;
		item.init("material."+material.name, 1);
		GameNode3D* itemNode = nullptr;
		Entity* itemEntity = nullptr;
		itemDropEntityTemplate->create(item, position, glm::vec3(0.0f), itemEntity, itemNode);
		World::currentWorld->entityManager->registerEntity(itemEntity);
		TealEngine::Core::getRoot()->addChild(itemNode);
	}
};
MaterialItemDropBreakHandler materialDropHandler;
VoxelMaterials::VoxelMaterials() 
{	
	addMaterial("toxic_sludge", true, false, DecorationType::None, nullptr);
	addMaterial("short_grass", true, false, DecorationType::Grass, &materialDropHandler);
	addMaterial("slate", true, false, DecorationType::None, &materialDropHandler);
	addMaterial("cobble", true, false, DecorationType::None, &materialDropHandler);
	addMaterial("wood", true, false, DecorationType::None, &materialDropHandler);
	addMaterial("dirt", true, false, DecorationType::None, &materialDropHandler);
	addMaterial("tiles", true, false, DecorationType::None, &materialDropHandler);
	addMaterial("mana_crystal", false, false, DecorationType::None, &materialDropHandler);
	addMaterial("copper_ore", true, false, DecorationType::None, &materialDropHandler);
	addMaterial("iron_ore", true, false, DecorationType::None, &materialDropHandler);
	addMaterial("stone_bricks", false, false, DecorationType::None, &materialDropHandler);
	addMaterial("sand", true, false, DecorationType::None, &materialDropHandler);
	addMaterial("coal", true, false, DecorationType::None, &materialDropHandler);
	addMaterial("aztec0", false, false, DecorationType::None, &materialDropHandler);
	addMaterial("aztec1", false, false, DecorationType::None, &materialDropHandler);
	build();
}

void pushTextureToMaterialsTextureBuffer(const std::string& name, Texture& fallback, int& textureWriteOffset, int singleTextureBufferSize, std::vector<unsigned char>& buffer) 
{
	if(Core::textureManager.exists(name)) 
	{
		Texture& t = Core::textureManager.get(name);
		memcpy(buffer.data() + textureWriteOffset, t.getPixels(), singleTextureBufferSize);
	}
	else 
	{
		memcpy(buffer.data() + textureWriteOffset, fallback.getPixels(), singleTextureBufferSize);
	}
	textureWriteOffset += singleTextureBufferSize;
}

void VoxelMaterials::build() 
{
	const int textureWidth = 64;
	const int nrChannels = 4;
	const int nrTextures = 6;
	const int nrTexturesRepeat = 2;
	const int textureBufferSize = vmats.size() * textureWidth * textureWidth * nrChannels * nrTextures * nrTexturesRepeat;
	const int singleTextureBufferSize = textureWidth * textureWidth * nrChannels;
	int textureWriteOffset = 0;
	textureBuffer.reserve(textureBufferSize);
	textureBuffer.resize(textureBufferSize);
	Texture& defaultAlbedo = Core::textureManager.get("terrain.placeholder.png");
	Texture& defaultNormal = Core::textureManager.get("terrain.placeholder.normal.png");
	Texture& defaultUnlit = Core::textureManager.get("terrain.placeholder.unlit.png");
	Texture& defaultSpecular = Core::textureManager.get("terrain.placeholder.specular.png");

	for(const VoxelMaterial& material : vmats) 
	{

		const std::string& name = material.name;
		
		const std::string& sideTextureName = "terrain.side." + name + ".png";
		const std::string& topTextureName = "terrain.top." + name + ".png";
		const std::string& sideNormalTextureName = "terrain.side.normal." + name + ".png";
		const std::string& topNormalTextureName = "terrain.top.normal." + name + ".png";
		const std::string& unlitTexture = "terrain.side.unlit." + name + ".png";
		const std::string& specularTextureName = "terrain.side.specular." + name + ".png";
		
		for(int i = 0; i < nrTexturesRepeat; i++) pushTextureToMaterialsTextureBuffer(sideTextureName, defaultAlbedo, textureWriteOffset, singleTextureBufferSize, textureBuffer);
		for(int i = 0; i < nrTexturesRepeat; i++) pushTextureToMaterialsTextureBuffer(topTextureName, defaultAlbedo, textureWriteOffset, singleTextureBufferSize, textureBuffer);
		for(int i = 0; i < nrTexturesRepeat; i++) pushTextureToMaterialsTextureBuffer(sideNormalTextureName, defaultNormal, textureWriteOffset, singleTextureBufferSize, textureBuffer);
		for(int i = 0; i < nrTexturesRepeat; i++) pushTextureToMaterialsTextureBuffer(topNormalTextureName, defaultNormal, textureWriteOffset, singleTextureBufferSize, textureBuffer);
		for(int i = 0; i < nrTexturesRepeat; i++) pushTextureToMaterialsTextureBuffer(unlitTexture, defaultUnlit, textureWriteOffset, singleTextureBufferSize, textureBuffer);
		for(int i = 0; i < nrTexturesRepeat; i++) pushTextureToMaterialsTextureBuffer(specularTextureName, defaultSpecular, textureWriteOffset, singleTextureBufferSize, textureBuffer);
	}
	this->materialsTexture = Texture(GL_TEXTURE_2D, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	this->materialsTexture.create(textureWidth, textureWidth * vmats.size() * nrTextures * nrTexturesRepeat, textureBuffer.data());
	for(int i = 0; i < vmats.size(); i++) 
	{
		vmats[i].texture = i;
	}
	
	materialsTexture.setParameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	materialsTexture.setParameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	materialsTexture.setParameteri(GL_TEXTURE_WRAP_S, GL_REPEAT);
	materialsTexture.setParameteri(GL_TEXTURE_WRAP_T, GL_REPEAT);
	materialsTexture.generateMipmap();

	const unsigned int itemTextureSize = 64;
	const unsigned int itemTexturePadding = 0;
	voxelMaterialItemTextures = new unsigned char[itemTextureSize*itemTextureSize*4*vmats.size()];
	memset(voxelMaterialItemTextures, 0, itemTextureSize*itemTextureSize*4);

	Texture& pt = Core::textureManager.get("ItemCubePosition.png");
	Texture& nt = Core::textureManager.get("ItemCubeNormal.png");
	unsigned char* ptPixels = (unsigned char*)pt.getPixels();
	unsigned char* ntPixels = (unsigned char*)nt.getPixels();
	for(int matId = 0; matId < vmats.size(); matId++) 
	{
		unsigned int sideTextureBase = 	matId * nrTexturesRepeat * singleTextureBufferSize * nrTextures + singleTextureBufferSize * nrTexturesRepeat * 0;
		unsigned int topTextureBase  = 	matId * nrTexturesRepeat * singleTextureBufferSize * nrTextures + singleTextureBufferSize * nrTexturesRepeat * 1;
		unsigned int sideNormalBase  = 	matId * nrTexturesRepeat * singleTextureBufferSize * nrTextures + singleTextureBufferSize * nrTexturesRepeat * 2;
		unsigned int topNormalBase   = 	matId * nrTexturesRepeat * singleTextureBufferSize * nrTextures + singleTextureBufferSize * nrTexturesRepeat * 3;
		for(unsigned int i = itemTexturePadding; i < itemTextureSize - itemTexturePadding; i++) 
		{
			for(unsigned int j = itemTexturePadding; j < itemTextureSize - itemTexturePadding; j++) 
			{
				unsigned int flatIndex = a2to1ind(j, i, itemTextureSize);
				if(ptPixels[flatIndex * 4 + 3] == 0) continue;
				glm::vec3 position = glm::vec3(
					ptPixels[flatIndex * 4 + 0],
					ptPixels[flatIndex * 4 + 1],
					ptPixels[flatIndex * 4 + 2]
				) / 255.0f;
				glm::vec3 normal = glm::vec3(
					ntPixels[flatIndex * 4 + 0],
					ntPixels[flatIndex * 4 + 1],
					ntPixels[flatIndex * 4 + 2]
				) / 255.0f * 2.0f - 1.0f;

				glm::ivec2 uvx = glm::ivec2(position.z * textureWidth, position.y * textureWidth);
				glm::ivec2 uvy = glm::ivec2(position.x * textureWidth, position.z * textureWidth);
				glm::ivec2 uvz = glm::ivec2(position.x * textureWidth, position.y * textureWidth);

				unsigned int colXIndex = sideTextureBase + a2to1ind(uvx.y, uvx.x, textureWidth) * 4;
				unsigned int colYIndex = topTextureBase + a2to1ind(uvy.y, uvy.x, textureWidth) * 4;
				unsigned int colZIndex = sideTextureBase + a2to1ind(uvz.y, uvz.x, textureWidth) * 4;
				
				unsigned int normXIndex = sideNormalBase + a2to1ind(uvx.y, uvx.x, textureWidth) * 4;
				unsigned int normYIndex = topNormalBase + a2to1ind(uvy.y, uvy.x, textureWidth) * 4;
				unsigned int normZIndex = sideNormalBase + a2to1ind(uvz.y, uvz.x, textureWidth) * 4;

				glm::vec3 weights = glm::abs(normal*normal);
				//float wswap = weights.y;
				//weights.y = weights.z;
				//weights.z = wswap;

				glm::vec3 n0 = 
				(glm::vec3(
					textureBuffer[normXIndex + 0], 
					textureBuffer[normXIndex + 1], 
					textureBuffer[normXIndex + 2]
				) / 255.0f * 2.0f - 1.0f);
				glm::vec3 n1 = 
				(glm::vec3(
					textureBuffer[normYIndex + 0], 
					textureBuffer[normYIndex + 1], 
					textureBuffer[normYIndex + 2]
				) / 255.0f * 2.0f - 1.0f);
				glm::vec3 n2 = 
				(glm::vec3(
					textureBuffer[normZIndex + 0], 
					textureBuffer[normZIndex + 1], 
					textureBuffer[normZIndex + 2]
				) / 255.0f * 2.0f - 1.0f);
				
				n0 = glm::vec3(
				glm::vec2(n0.x, n0.y) + glm::vec2(normal.z, normal.y),
				glm::abs(n0.z) * normal.x
				);	

				n1 = glm::vec3(
				glm::vec2(n1.x, n1.y) + glm::vec2(normal.x, normal.z),
				glm::abs(n1.z) * normal.y
				);

				n2 = glm::vec3(
				glm::vec2(n2.x, n2.y) + glm::vec2(normal.x, normal.y),
				glm::abs(n2.z) * normal.z
				);

				glm::vec3 outNormal = glm::normalize(
				glm::vec3(n0.z, n0.y, n0.x) * weights.x +
				glm::vec3(n1.x, n1.z, n1.y) * weights.y +
				n2 * weights.z);

				unsigned int colR = 
				textureBuffer[colXIndex + 0] * weights.x + 
				textureBuffer[colYIndex + 0] * weights.y + 
				textureBuffer[colZIndex + 0] * weights.z;

				unsigned int colG = 
				textureBuffer[colXIndex + 1] * weights.x + 
				textureBuffer[colYIndex + 1] * weights.y + 
				textureBuffer[colZIndex + 1] * weights.z;

				unsigned int colB = 
				textureBuffer[colXIndex + 2] * weights.x + 
				textureBuffer[colYIndex + 2] * weights.y + 
				textureBuffer[colZIndex + 2] * weights.z;

				glm::vec3 sun = glm::normalize(glm::vec3(1.0f, 2.0f, 3.0f));
				float light = (glm::dot(outNormal, sun) * 0.8f + 0.2f);

				light = glm::clamp(light, 0.0f, 1.0f);

				unsigned int itemPixelIndex = flatIndex + matId * itemTextureSize * itemTextureSize;
				voxelMaterialItemTextures[itemPixelIndex * 4 + 0] = glm::min(255.0f, colR * light);
				voxelMaterialItemTextures[itemPixelIndex * 4 + 1] = glm::min(255.0f, colG * light);
				voxelMaterialItemTextures[itemPixelIndex * 4 + 2] = glm::min(255.0f, colB * light);
				voxelMaterialItemTextures[itemPixelIndex * 4 + 3] = ptPixels[flatIndex * 4 + 3];
			}
		}
		glGenTextures(1, &vmats[matId].itemTexture);
		glBindTexture(GL_TEXTURE_2D, vmats[matId].itemTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		unsigned int itemTextureBase = matId * itemTextureSize * itemTextureSize * nrChannels;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, itemTextureSize, itemTextureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, &voxelMaterialItemTextures[itemTextureBase]);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	MaterialDropRenderer::init(materialsTexture.id(), glm::vec2(1.0f, vmats.size() * 12.0f));
}

const VoxelMaterial& VoxelMaterials::getById(unsigned short id) const
{
	return this->vmats[id];
}

void VoxelMaterials::addMaterial(const std::string& name, bool smooth, bool cube, DecorationType decoration, BreakEventHandler* breakEventHandler) 
{
	vmats.push_back({});
	VoxelMaterial& vm = vmats[vmats.size() - 1];
	vm.name = name;
	namemap[vm.name] = vmats.size() - 1;
	vm.breakEvent = breakEventHandler;
	vm.cube = cube;
	vm.decorations = decoration;
	vm.smooth = smooth;
	vm.id = vmats.size() - 1;
}

unsigned short VoxelMaterials::getIdByName(const std::string& name) const
{
	return namemap.at(name);
}

VoxelMaterials& VoxelMaterials::getInstance() 
{
	if(instance == nullptr) 
	{
		instance = new VoxelMaterials();
	}
	return *instance;
}


std::string VoxelMaterials::getMaterialItemName(unsigned short id) 
{
	return materialItemPrefix + vmats[id].name;
}

std::string VoxelMaterials::parseMaterialNameFromMaterialItemId(const std::string& itemId) 
{
	if(itemId.starts_with(materialItemPrefix)) 
	{
		return itemId.substr(materialItemPrefix.length());
	}
	return "";
}