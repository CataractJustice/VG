#include "TileMap.h"
#include "GameNode/ComponentFactory.h"
#include "Math/IndexConvert.h"
#include "TileTypes.h"
#include "GameNode/GameNode3D.h"
#include "Chunk.h"
#include "TileMeshPart.h"
#include <fstream>
#include "System/BlockAllocator.h"
const unsigned char defaultOrientation = 0b00010001;

TealEngine::BlockAllocator tilesAllocator(sizeof(TileMap::TileData) * 32*32*32 + sizeof(unsigned short)*32*32*32, 15*15*15);

unsigned char getZCross(unsigned char x, unsigned char y) 
{
	bool signx = (((x & 0b100) ? 1 : 0) + ((x & 0b10) ? 1 : 0) + ((x & 0b1) ? 1 : 0)) > 1;
	bool signy = (((y & 0b100) ? 1 : 0) + ((y & 0b10) ? 1 : 0) + ((y & 0b1) ? 1 : 0)) > 1;
	bool signz = signx != signy;
	unsigned char absx = signx ? ((~x)&0b111) : x;
	unsigned char absy = signy ? ((~y)&0b111) : y;

	const unsigned char posX = 0b001;
	const unsigned char negX = 0b110;
	const unsigned char posY = 0b010;
	const unsigned char negY = 0b101;
	const unsigned char posZ = 0b100;
	const unsigned char negZ = 0b011;

	if(absx == posX && absy == posY) 
	{
		//+
		return signz ? negZ : posZ;
	}
	if(absx == posX && absy == posZ) 
	{
		//-
		return signz ? posY : negY;
	}
	if(absx == posY && absy == posX) 
	{
		//-
		return signz ? posZ : negZ;
	}
	if(absx == posY && absy == posZ) 
	{
		//+
		return signz ? negX : posX;
	}
	if(absx == posZ && absy == posX) 
	{
		//+
		return signz ? negY : posY;
	}
	if(absx == posZ && absy == posY) 
	{
		//-
		return signz ? posX : negX;
	}
	return 0b000;
}

void decomposeOrientation(unsigned char orientation, unsigned char& x, unsigned char& y, unsigned char& z) 
{
	x = (orientation & 0b111);
	y = (orientation & 0b111000) >> 3;
	z = getZCross(x, y);
}

void decomposeXY(unsigned char orientation, unsigned char& x, unsigned char& y) 
{
	x = (orientation & 0b111);
	y = (orientation & 0b111000) >> 3;
}

unsigned char composeOrientation(unsigned char x, unsigned char y) 
{
	return x + (y << 3);
}

unsigned char rotXC(unsigned char orientation) 
{
	unsigned char x, y, z;
	decomposeOrientation(orientation, x, y, z);
	y = z;
	return composeOrientation(x, y);
}

unsigned char rotXCC(unsigned char orientation) 
{
	unsigned char x, y, z;
	decomposeOrientation(orientation, x, y, z);
	y = ((~z)&0b111);
	return composeOrientation(x, y);
}

unsigned char rotYC(unsigned char orientation) 
{
	unsigned char x, y, z;
	decomposeOrientation(orientation, x, y, z);
	x = ((~z)&0b111);
	return composeOrientation(x, y);
}

unsigned char rotYCC(unsigned char orientation) 
{
	unsigned char x, y, z;
	decomposeOrientation(orientation, x, y, z);
	x = z;
	return composeOrientation(x, y);
}

unsigned char rotZC(unsigned char orientation) 
{
	unsigned char x, y;
	decomposeXY(orientation, x, y);
	unsigned char t = x;
	x = y;
	y = ((~t)&0b111);
	return composeOrientation(x, y);
}

unsigned char rotZCC(unsigned char orientation) 
{
	unsigned char x, y;
	decomposeXY(orientation, x, y);
	unsigned char t = x;
	x = ((~y)&0b111);
	y = t;
	return composeOrientation(x, y);
}

void getAxes(unsigned char orientation, glm::vec3& xAxis, glm::vec3& yAxis, glm::vec3 zAxis) 
{
	unsigned char x = orientation & 0b111;
	unsigned char y = (orientation >> 3) & 0b111;
	unsigned char xBitX = (x & 0b001) ? 1 : 0;
	unsigned char xBitY = (x & 0b010) ? 1 : 0;
	unsigned char xBitZ = (x & 0b100) ? 1 : 0;
	unsigned char xbitcount = xBitX + xBitY + xBitZ;
	if(xbitcount > 1) 
	{
		x = ((~x)&0b111);	
		xBitX = xBitX ? 0 : 1; 
		xBitY = xBitY ? 0 : 1; 
		xBitZ = xBitZ ? 0 : 1; 
	}
	xAxis = glm::vec3(1.0f * (x & 0b001), 1.0f * (x & 0b010), 1.0f * (0b100));
	if(xbitcount > 1) xAxis = -xAxis;
	
}

const float gridSize = 1.0f;
const int tileDataWidth = 32;
const int tileDataHeight = 32;
const int tileDataLength = 32;
const glm::ivec3 tileDataDimensions = glm::ivec3(tileDataWidth, tileDataHeight, tileDataLength);
const int tileDataSize = tileDataWidth * tileDataHeight * tileDataLength;

TileMap::TileMap() 
{
	memset(tileIds, 0, sizeof(tileIds));
	memset(cubeMaterials, 0, sizeof(cubeMaterials));
	meshParts[0] = new TileMeshPart();
	meshParts[1] = new TileMeshPart();
	meshParts[2] = new TileMeshPart();
	meshParts[3] = new TileMeshPart();
	meshParts[4] = new TileMeshPart();
	meshParts[5] = new TileMeshPart();
	parentChunk = nullptr;
	
	glGenTextures(1, &volumeSampler);
	glBindTexture(GL_TEXTURE_3D, volumeSampler);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexImage3D(
		GL_TEXTURE_3D,
		0,
		GL_R16UI,
		32,
		32,
		32,
		0,
		GL_RED_INTEGER,
		GL_UNSIGNED_SHORT,
		cubeMaterials
	);
	cubeTileCount = 0;
}

void TileMap::onAttach() 
{
	//const Transform& t = this->getParentOfType<GameNode3D>()->getWorldTransform();
	meshParts[0]->setTileMap(this, TileMeshPart::Back);
	meshParts[1]->setTileMap(this, TileMeshPart::Front);
	meshParts[2]->setTileMap(this, TileMeshPart::Bottom);
	meshParts[3]->setTileMap(this, TileMeshPart::Top);
	meshParts[4]->setTileMap(this, TileMeshPart::Left);
	meshParts[5]->setTileMap(this, TileMeshPart::Right);
}

void TileMap::updateMesh() 
{
	for(int i = 0; i < 6; i++) 
	{
		meshParts[i]->updateData();
	}
}
void TileMap::uploadMesh() const 
{
	glBindTexture(GL_TEXTURE_3D, volumeSampler);
	glTexImage3D(
		GL_TEXTURE_3D,
		0,
		GL_R16UI,
		32,
		32,
		32,
		0,
		GL_RED_INTEGER,
		GL_UNSIGNED_SHORT,
		cubeMaterials
	);
	for(int i = 0; i < 6; i++) 
	{
		meshParts[i]->uploadData();
	}
}

TileMap::TileData TileMap::tileAtWrapped(const glm::ivec3& index) const
{
	glm::ivec3 neighborIndex = glm::ivec3(0,0,0);
	if(index.x < 0) neighborIndex.x = -1; else
	if(index.x >= tileDataDimensions.x) neighborIndex.x = 1;
	if(index.y < 0) neighborIndex.y = -1; else
	if(index.y >= tileDataDimensions.y) neighborIndex.y = 1;
	if(index.z < 0) neighborIndex.z = -1; else
	if(index.z >= tileDataDimensions.z) neighborIndex.z = 1;
	const TileMap* tileMap = this;
	if(neighborIndex.x || neighborIndex.y ||  neighborIndex.z) 
	{
		tileMap = &parentChunk->getNeighbor(neighborIndex)->tileMap;
	}
	glm::ivec3 relativeIndex = index - neighborIndex * tileDataDimensions;
	unsigned int flatIndex = a3to1ind(relativeIndex, tileDataDimensions.y, tileDataDimensions.z);
	return {
		tileMap->cubeMaterials[flatIndex],
		tileMap->tileIds[flatIndex],
		0
	};
}

unsigned short TileMap::cubeAtWrapped(const glm::ivec3& index) const 
{
	glm::ivec3 neighborIndex = glm::ivec3(0,0,0);
	if(index.x < 0) neighborIndex.x = -1; else
	if(index.x >= tileDataDimensions.x) neighborIndex.x = 1;
	if(index.y < 0) neighborIndex.y = -1; else
	if(index.y >= tileDataDimensions.y) neighborIndex.y = 1;
	if(index.z < 0) neighborIndex.z = -1; else
	if(index.z >= tileDataDimensions.z) neighborIndex.z = 1;
	const TileMap* tileMap = this;
	if(neighborIndex.x || neighborIndex.y ||  neighborIndex.z) 
	{
		Chunk* chunk = parentChunk->getNeighbor(neighborIndex);
		if(chunk == nullptr) return 0;
		tileMap = &chunk->tileMap;
	}
	glm::ivec3 relativeIndex = index - neighborIndex * tileDataDimensions;
	unsigned int flatIndex = a3to1ind(relativeIndex, tileDataDimensions.y, tileDataDimensions.z);
	return tileMap->cubeMaterials[flatIndex];
}

void TileMap::setTileAtIndex(glm::ivec3 index, const TileMap::TileData& newTile) 
{
	unsigned int flatIndex = a3to1ind(index, tileDataDimensions.y, tileDataDimensions.z);
	if(newTile.tileId) 
	{
		const TileType& newTileType = TileTypes::getInstance().getById(newTile.tileId);
		if(newTileType.isNode) 
		{
			//
		}
	}
	if(newTile.cubeMaterialId && cubeMaterials[flatIndex] == 0) 
	{
		cubeTileCount++;
	}
	else if(newTile.cubeMaterialId == 0 && cubeMaterials[flatIndex])
	{
		cubeTileCount--;
	}
	cubeMaterials[flatIndex] = newTile.cubeMaterialId;
}

TileMap::~TileMap() 
{
}

static void decomposeXYZWHL(unsigned int xyzwhl, int& x, int& y, int& z, int& w, int& h, int& l) 
{
	x = xyzwhl & 0b11111 << 0;
	y = xyzwhl & 0b11111 << 1;
	z = xyzwhl & 0b11111 << 2;
	w = (xyzwhl & 0b11111 << 3) + 1;
	h = (xyzwhl & 0b11111 << 4) + 1;
	l = (xyzwhl & 0b11111 << 5) + 1;
}
static int composeXYZWHL(int x, int y, int z, int w, int h, int l) 
{
	return x + (y << 5) + (z << 10) + ((w - 1) << 15) + ((h - 1) << 20) + ((l - 1) << 25);
}

std::vector<unsigned short> TileMap::compress() 
{
	std::vector<unsigned short> data;
	bool marked[32*32*32];
	memset(marked, 0, sizeof(marked));
	for(int x = 0; x < 32; x++) 
	{
		for(int y = 0; y < 32; y++) 
		{
			for(int z = 0; z < 32; z++) 
			{
				if(marked[a3to1ind(x,y,z,32,32)]) continue;
				marked[a3to1ind(x,y,z,32,32)] = true;
				int w = 1, h = 1, l = 1;
				unsigned short baseMaterial = getTileAtIndex(glm::ivec3(x,y,z)).cubeMaterialId;

				//extend Z
				glm::ivec3 testIndex;
				while(z + l < 32) 
				{
					testIndex = glm::ivec3(x, y, z+l);
					if(marked[a3to1ind(testIndex, 32, 32)]) 
					{
						break;
					}
					if(baseMaterial != getTileAtIndex(testIndex).cubeMaterialId) 
					{
						break;
					}
					marked[a3to1ind(testIndex, 32, 32)] = true;
					l++;
				}
				//extend Y
				bool canExtendY = true;
				while (y + h < 32 && canExtendY)
				{
					for (int dz = 0; dz < l; dz++)
					{
						testIndex = glm::ivec3(x, y + h, z + dz);

						if (marked[a3to1ind(testIndex, 32, 32)] ||
							getTileAtIndex(testIndex).cubeMaterialId != baseMaterial)
						{
							canExtendY = false;
							break;
						}
					}

					if (canExtendY)
					{
						for (int dz = 0; dz < l; dz++)
						{
							marked[a3to1ind(x, y + h, z + dz, 32, 32)] = true;
						}
						h++;
					}
				}
				//extend X
				bool canExtendX = true;
				while (x + w < 32 && canExtendX)
				{
					for (int dy = 0; dy < h; dy++)
					{
						for (int dz = 0; dz < l; dz++)
						{
							testIndex = glm::ivec3(x + w, y + dy, z + dz);

							if (marked[a3to1ind(testIndex, 32, 32)] ||
								getTileAtIndex(testIndex).cubeMaterialId != baseMaterial)
							{
								canExtendX = false;
								break;
							}
						}
						if (!canExtendX) break;
					}

					if (canExtendX)
					{
						for (int dy = 0; dy < h; dy++)
						{
							for (int dz = 0; dz < l; dz++)
							{
								marked[a3to1ind(x + w, y + dy, z + dz, 32, 32)] = true;
							}
						}
						w++;
					}
				}
				data.push_back(baseMaterial);
				data.push_back(composeWHL(w,h,l));
			}	
		}	
	}
	if(data.size() % 2) throw;
	return data;
}
unsigned short TileMap::composeWHL(int w, int h, int l) 
{
	return (w - 1) | ((h - 1) << 5) | ((l - 1) << 10);
}
void TileMap::decomposeWHL(unsigned short whl, int& w, int& h, int& l) 
{
	w = (whl & 0b11111u) + 1;
	h = ((whl & 0b1111100000u) >> 5) + 1;
	l = ((whl & 0b111110000000000u) >> 10) + 1;
}
void TileMap::decompress(const std::vector<unsigned short>& data) 
{
	if(data.size() % 2) throw;
	int readIndex = 0;
	int xyz = 0;
	const int chunkWidth = 32;
	unsigned int mask[chunkWidth*chunkWidth];
	memset(mask, 0, sizeof(mask)	);
	#define setbit(x, y, z) (mask[x*chunkWidth+y] |= (1 << z))
	#define getbit(x, y, z) (mask[x*chunkWidth+y] & (1 << z))
	while(readIndex < data.size() && xyz < 32*32*32) 
	{
		unsigned int z = xyz & 0b11111;
		unsigned int y = (xyz & 0b1111100000) >> 5;
		unsigned int x = (xyz & 0b111110000000000) >> 10;
		while(getbit(x,y,z)) 
		{
			xyz++;
			z = xyz & 0b11111;
			y = (xyz & 0b1111100000) >> 5;
			x = (xyz & 0b111110000000000) >> 10;
		}

		unsigned int materialType = data[readIndex];
		unsigned short whl = data[readIndex+1];
		int widthSpan;
		int heightSpan;
		int lengthSpan;
		decomposeWHL(whl, widthSpan, heightSpan, lengthSpan);
		for(int i = x; i < x + widthSpan;  i++)
		for(int j = y; j < y + heightSpan; j++)
		for(int k = z; k < z + lengthSpan; k++) 
		{
			if(i >= 32 || j >= 32 || k >= 32) 
			{
				throw;
			}
			TileData newTile;
			newTile.cubeMaterialId = materialType;
			newTile.orientation = 0;
			newTile.tileId = 0;
			this->setTileAtIndex(glm::ivec3(i,j,k), newTile);
			setbit(i,j,k);
		}
		readIndex += 2;
		xyz++;
	}
}

void TileMap::save(const std::filesystem::path& path) 
{
	const std::vector<unsigned short> data = compress();
	std::ofstream file(path, std::ios::binary);
	file.write((char*)data.data(), data.size() * sizeof(unsigned short));
}
TileMap::TileData TileMap::getTileAtIndex(glm::ivec3 index) const 
{
	auto flat = a3to1ind(index, tileDataHeight, tileDataLength);
	return {cubeMaterials[flat], tileIds[flat], 0};
}

void TileMap::clear() 
{
	memset(tileIds, 0, sizeof(tileIds));
	memset(cubeMaterials, 0, sizeof(cubeMaterials));
	cubeTileCount = 0;
}

TileMap::TileData TileMap::defaultTile = {0,0,0};