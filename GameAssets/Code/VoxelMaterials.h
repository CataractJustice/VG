#include "libs/glm/glm.hpp"
#include <vector>
#include <string>
#include <map>
#include "Graphics/Texture.h"

static const std::string& materialItemPrefix = "material.";

enum class DecorationType 
{
	None,
	Grass,
	Leaves
};

class BreakEventHandler;

struct VoxelMaterial
{
public:
	unsigned int texture;
	unsigned int itemTexture;
	std::string name;
	DecorationType decorations;
	BreakEventHandler* breakEvent;
	bool smooth;
	bool cube;
	unsigned short id;
};

class BreakEventHandler 
{
	public:
	virtual void invoke(glm::vec3 position, VoxelMaterial material) = 0;
};

class VoxelMaterials 
{
	private:
	std::vector<VoxelMaterial> vmats;
	std::map<std::string, unsigned short> namemap;
	std::vector<unsigned char> textureBuffer;
	static VoxelMaterials* instance;
	void addMaterial(const std::string& name, bool smooth, bool cube, DecorationType decoration, BreakEventHandler* breakEventHandler);
	void build();
	public:
	TealEngine::Texture materialsTexture;
	VoxelMaterials();
	const VoxelMaterial& getById(unsigned short id) const;
	unsigned short getIdByName(const std::string& name) const;
	inline const std::vector<VoxelMaterial> getMaterials() const { return vmats; };
	static VoxelMaterials& getInstance();
	std::string getMaterialItemName(unsigned short id);
	std::string parseMaterialNameFromMaterialItemId(const std::string& itemId);
};