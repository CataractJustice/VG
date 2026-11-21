#include "VoxelData.h"
#include "Math/Interpolation.h"
#include "FileSystem/loadBytesFromFile.h"
using namespace TealEngine;
#include <vector>
#include <fstream>
#include "Chunk.h"
#include "System/BlockAllocator.h"
#include "Math/IndexMath.h"
#include "System/Random.h"

Voxel VoxelData::defaultValue = {0,0};

VoxelData::VoxelData(glm::ivec3 dimensions, glm::ivec3 index, ScalarFieldWrapping wrapMode)
{
	this->neighborsCount = 0;
	this->dimensions = dimensions;
	this->datalength = dimensions.x * dimensions.y * dimensions.z;
	this->index = index;
	memset(amounts, 0, sizeof(amounts));
	memset(voxelTypeIds, 0, sizeof(voxelTypeIds));
	memset(neighbors, 0, sizeof(VoxelData*) * 27);
	memset(xSliceMass, 0, sizeof(int) * dimensions.x);
	memset(xyLineMass, 0, sizeof(int) * dimensions.x * dimensions.y);
	mass = 0;
	lastModify = 0;
}

VoxelData::~VoxelData() 
{
}

glm::ivec3 VoxelData::getDimensions()
{
	return dimensions;
}

glm::ivec3 VoxelData::getIndex() 
{
	return index;
}

unsigned int VoxelData::getDataLength() 
{
	return this->datalength * sizeof(Voxel);
}

const VoxelData* VoxelData::getNeighborConst(glm::ivec3 side) const
{
	if (side.x == 0 && side.y == 0 && side.z == 0)
		return nullptr;
	else
		return neighbors[a3to1ind(side.x + 1, side.y + 1, side.z + 1, 3, 3)];
}

VoxelData* VoxelData::getNeighbor(glm::ivec3 side) 
{
	return const_cast<VoxelData*>(getNeighborConst(side));
}

void VoxelData::setNeighbor(glm::ivec3 side, const VoxelData* data)
{
	if (side.x == 0 && side.y == 0 && side.z == 0)
		return;
	else
	{
		unsigned int flatIndex = a3to1ind(side.x + 1, side.y + 1, side.z + 1, 3, 3);
		const VoxelData* oldNeighbor = neighbors[flatIndex];
		neighbors[a3to1ind(side.x + 1, side.y + 1, side.z + 1, 3, 3)] = data;
		if(oldNeighbor == nullptr && data)neighborsCount++;
		else if(oldNeighbor && data == nullptr) neighborsCount--;
		chunk->setNeighbor(side, chunk);
	}
}

unsigned int VoxelData::getNeighborCount() const
{
	return neighborsCount;
}

const Voxel VoxelData::getVoxelConst(glm::ivec3 position) const
{
	auto index = a3to1ind(position, dimensions.y, dimensions.z);
	return {amounts[index], voxelTypeIds[index]};//&voxels[];
}

signed char VoxelData::getVoxelValue(glm::ivec3 position) const
{
	return amounts[a3to1ind(position, dimensions.y, dimensions.z)];
}

signed char VoxelData::getVoxelValue(int x, int y, int z) const
{
	return amounts[a3to1ind(x,y,z, dimensions.y, dimensions.z)];
}

//Every Voxel is filled with 0s at the initialisation
//Every interaction with voxels is done through this function
void VoxelData::quickSetVoxel(glm::ivec3 position, Voxel voxel)
{
#ifdef DEBUG
	if (position.x < 0 || position.y < 0 || position.z < 0 || position.x >= dimensions.x || position.y >= dimensions.y || position.z >= dimensions.z)
	{
		TE_DEBUG_ERROR("Index out of chunk bounds. Use modifyVoxel if you need to use wrapping mode.");
	}
#endif
	//Voxel& v = this->voxels[];
	auto index = a3to1ind(position, dimensions.y, dimensions.z);
	bool oldSolid = amounts[index] > 0;
	bool newSolid = voxel.amount > 0;
	if(oldSolid != newSolid) 
	{
		const int delta = newSolid ? 1 : -1;
		xyLineMass[a2to1ind(position.x, position.y, dimensions.y)] += delta;
		xSliceMass[position.x] += delta;
		mass += delta;
	}
	amounts[index] = voxel.amount;
	voxelTypeIds[index] = voxel.voxelTypeId;
	lastModify++;
}


const Voxel VoxelData::getVoxelWrappedConst(glm::ivec3 position) const
{
	if (
		position.x < 0 || position.x >= dimensions.x ||
		position.y < 0 || position.y >= dimensions.y ||
		position.z < 0 || position.z >= dimensions.z
		)
	{
		glm::ivec3 neighborSide = glm::ivec3(0);
		if (position.x < 0) neighborSide.x = -1;
		if (position.y < 0) neighborSide.y = -1;
		if (position.z < 0) neighborSide.z = -1;
		if (position.x >= dimensions.x) neighborSide.x = 1;
		if (position.y >= dimensions.y) neighborSide.y = 1;
		if (position.z >= dimensions.z) neighborSide.z = 1;
		if (getNeighborConst(neighborSide))
			return getNeighborConst(neighborSide)->getVoxelConst(position - neighborSide * dimensions);
		return defaultValue;
	}
	return getVoxelConst(position);
}

signed char VoxelData::getVoxelValueWrapped(int x, int y, int z) const
{
	if (
		x < 0 || x >= dimensions.x ||
		y < 0 || y >= dimensions.y ||
		z < 0 || z >= dimensions.z
		)
	{
		glm::ivec3 neighborSide = glm::ivec3(0);
		if (x < 0) neighborSide.x = -1; else
		if (x >= dimensions.x) neighborSide.x = 1;
		if (y < 0) neighborSide.y = -1; else
		if (y >= dimensions.y) neighborSide.y = 1;
		if (z < 0) neighborSide.z = -1; else
		if (z >= dimensions.z) neighborSide.z = 1;
		if (getNeighborConst(neighborSide))
			return getNeighborConst(neighborSide)->getVoxelValue(
				x - neighborSide.x * dimensions.x,
				y - neighborSide.y * dimensions.y,
				z - neighborSide.z * dimensions.z
			);
	}
	return getVoxelValue(x,y,z);
}
signed char VoxelData::getVoxelValueWrapped(glm::ivec3 p) const 
{
	return getVoxelValueWrapped(p.x, p.y, p.z);
}

float VoxelData::getInterpolatedFieldValue(glm::vec3 position) const
{
	glm::ivec3 base = vec3ToIndex(position + 0.001f);
	float v000 = (float)(getVoxelValueWrapped(base.x + 0, base.y + 0, base.z + 0)) / voxelAmountScale;
	float v100 = (float)(getVoxelValueWrapped(base.x + 1, base.y + 0, base.z + 0)) / voxelAmountScale;
	float v010 = (float)(getVoxelValueWrapped(base.x + 0, base.y + 1, base.z + 0)) / voxelAmountScale;
	float v110 = (float)(getVoxelValueWrapped(base.x + 1, base.y + 1, base.z + 0)) / voxelAmountScale;
	float v001 = (float)(getVoxelValueWrapped(base.x + 0, base.y + 0, base.z + 1)) / voxelAmountScale;
	float v101 = (float)(getVoxelValueWrapped(base.x + 1, base.y + 0, base.z + 1)) / voxelAmountScale;
	float v011 = (float)(getVoxelValueWrapped(base.x + 0, base.y + 1, base.z + 1)) / voxelAmountScale;
	float v111 = (float)(getVoxelValueWrapped(base.x + 1, base.y + 1, base.z + 1)) / voxelAmountScale;

	return linear3DGridInterpolation(glm::fract(position), 
	v000,
	v100,
	v010,
	v110,
	v001,
	v101,
	v011,
	v111
	);
}

glm::vec3 VoxelData::getNormal(glm::vec3 position) const
{
	glm::ivec3 base = vec3ToIndex(position);
	float v000 = (float)(getVoxelValueWrapped(glm::ivec3(base.x + 0, base.y + 0, base.z + 0)));
	float v100 = (float)(getVoxelValueWrapped(glm::ivec3(base.x + 1, base.y + 0, base.z + 0)));
	float v010 = (float)(getVoxelValueWrapped(glm::ivec3(base.x + 0, base.y + 1, base.z + 0)));
	float v110 = (float)(getVoxelValueWrapped(glm::ivec3(base.x + 1, base.y + 1, base.z + 0)));
	float v001 = (float)(getVoxelValueWrapped(glm::ivec3(base.x + 0, base.y + 0, base.z + 1)));
	float v101 = (float)(getVoxelValueWrapped(glm::ivec3(base.x + 1, base.y + 0, base.z + 1)));
	float v011 = (float)(getVoxelValueWrapped(glm::ivec3(base.x + 0, base.y + 1, base.z + 1)));
	float v111 = (float)(getVoxelValueWrapped(glm::ivec3(base.x + 1, base.y + 1, base.z + 1)));
	return glm::normalize(glm::vec3(
		v000-v100+v010-v110+v001-v101+v011-v111, 
		v000+v100-v010-v110+v001+v101-v011-v111, 
		v000+v100+v010+v110-v001-v101-v011-v111
	));
}

bool VoxelData::isChunkEmpty() const
{
	return mass == 0;
}

bool VoxelData::isChunkFilled() const
{
	return mass == datalength;
}

bool VoxelData::isEmpty(Voxel voxel) 
{
	return voxel.amount <= 0;
}

bool VoxelData::isVisible(Voxel voxel) 
{
	return voxel.amount > 0;
}

/*
VoxelData save headers
*/

const unsigned int VoxelDataSaveMajikV1 = 0x51ccfee7;

struct VoxelDataVersionHeader 
{
	unsigned int majik;
	unsigned int version;
};

struct VoxelDataHeaderV1 
{
	unsigned char w, h, d;
	unsigned long long lastModify;
};

const unsigned int currentVersion = 4;

std::vector<char> VoxelData::saveData() 
{
	VoxelDataVersionHeader versionHeader;
	VoxelDataHeaderV1 header;
	versionHeader.majik = VoxelDataSaveMajikV1;
	versionHeader.version = currentVersion;
	header.w = dimensions.x;
	header.h = dimensions.y;
	header.d = dimensions.z;
	header.lastModify = lastModify;
	std::vector<char> bytes;
	std::vector<unsigned short> compressed = Compress();
	bytes.insert(bytes.end(), (char*)&versionHeader, ((char*)&versionHeader) + sizeof(versionHeader));
	bytes.insert(bytes.end(), (char*)&header, ((char*)&header) + sizeof(header));
	bytes.insert(bytes.end(), (char*)compressed.data(), ((char*)compressed.data()) + compressed.size() * sizeof(short));
	return bytes;
}

void VoxelData::save(const std::filesystem::path& path)
{
	std::vector<char> bytes = saveData();
	std::ofstream chunkFile(path, std::ios::binary);
	chunkFile.write(bytes.data(), bytes.size());
}

bool VoxelData::load(const std::filesystem::path& path) 
{
	VoxelDataVersionHeader versionHeader;
	VoxelDataHeaderV1 header;
	std::vector<char> bytes = loadBytesFromFile(path);
	if(bytes.size() < sizeof(versionHeader) + sizeof(header)) 
	{
		return false;
	}
	memcpy(&versionHeader, bytes.data(), sizeof(versionHeader));

	if(versionHeader.majik != VoxelDataSaveMajikV1) return false;
	if(versionHeader.version != currentVersion) return false;

	memcpy(&header, bytes.data() + sizeof(versionHeader), sizeof(header));
	if(header.w != dimensions.x || header.h != dimensions.y || header.d != dimensions.z) return false;
	this->lastModify = header.lastModify;
	const int expectedByteSize = 
		sizeof(versionHeader) + 
		sizeof(header) + 
		sizeof(Voxel) * datalength;
	const int voxelsOffset = sizeof(versionHeader) + sizeof(header);

	Decompress((unsigned short*)((bytes.data() + voxelsOffset)));
	return true;
}

unsigned int readBits(const std::vector<unsigned char>& data, unsigned int at, unsigned int n) 
{
	unsigned int atbyte = at / 8u;
	unsigned int atbit = at % 8u;
	unsigned int bitsleft = n;
	unsigned int shift = 0u;
	unsigned int read = 0u;
	while(bitsleft) 
	{
		unsigned int readbitsn = std::min(bitsleft, 8u - atbit);
		unsigned long long mask = (1u << readbitsn) - 1u;
		read += (((unsigned int)(data.data()[atbyte]) >> atbit) & mask) << shift;
		atbit = 0u; 
		shift += readbitsn;
		bitsleft -= readbitsn;
		atbyte++;
	}
	return read;
}

std::vector<unsigned short> VoxelData::Compress() 
{
	std::vector<unsigned short> data;
	int arlep = 0;
	int irlep = 0;
	char amount = amounts[0];
	unsigned short amountCount = 1;
	unsigned short voxelTypeId = voxelTypeIds[0];
	//Так как нам поебать на то какой тип вокселя будет иметь пустое место мы считаем что воздух это тот же тип что и первый не пустой воксель шоб рле его сжимал более эффективно
	if(voxelTypeId == 0) 
	{
		for(int i = 0; i < datalength; i++) 
		{
			if(voxelTypeIds[i]) 
			{
				voxelTypeId = voxelTypeIds[i];
				break;
			}  
		}
	}
	unsigned short voxelTypeIdCount = 1;
	for(int i = 1; i < datalength; i++) 
	{
		Voxel v = {amounts[i], voxelTypeIds[i]};
		if(v.amount == amount) amountCount++;
		else 
		{
			data.push_back(amountCount);
			data.push_back(amount);
			amountCount = 1;
			amount = v.amount;
			arlep++;
		}
	}
	data.push_back(amountCount);
	data.push_back(amount);
	arlep++;
	for(int i = 1; i < datalength; i++) 
	{
		Voxel v = {amounts[i], voxelTypeIds[i]};
		if(v.voxelTypeId == voxelTypeId) voxelTypeIdCount++;
		else 
		{
			data.push_back(voxelTypeIdCount);
			data.push_back(voxelTypeId);
			voxelTypeIdCount = 1;
			voxelTypeId = v.voxelTypeId;
			irlep++;
		}
	}
	data.push_back(voxelTypeIdCount);
	data.push_back(voxelTypeId);
	irlep++;
	return data;
}

void VoxelData::Decompress(const unsigned short* data) 
{
	int amountTotalCount = 0;
	int voxelTypeIdTotalCount = 0;
	int index = 0;
	this->mass = 0;
	memset(this->xSliceMass, 0, sizeof(int) * dimensions.x);
	memset(this->xyLineMass, 0, sizeof(int) * dimensions.x * dimensions.y);
	while(amountTotalCount < datalength) 
	{
		int amountCount = data[index];
		short amount = data[index + 1];
		if(amountTotalCount+amountCount > datalength) 
		{
			throw std::runtime_error("Bad chunk, amout total is greater than data length");
		}
		if(amount > 0) 
		{
			this->mass += amountCount;
			int amountCountTemp = amountCount;
			glm::ivec3 vStartIndex = a1to3ind(amountTotalCount, dimensions.y, dimensions.z);
			glm::ivec3 vStartIndexTemp = vStartIndex;
			while(amountCountTemp) 
			{
				//То как много полных вокселей может быть в данной линии
				const int xyMassMax = dimensions.z - vStartIndexTemp.z;
				//Сколько полных вокселей в данной линии учитывая РЛЕ длинну
				const int xyMass = std::min(xyMassMax, amountCountTemp);

				xyLineMass[a2to1ind(vStartIndexTemp.x, vStartIndexTemp.y, dimensions.y)] += xyMass;
				vStartIndexTemp.z = 0;
				vStartIndexTemp.y++;
				if(vStartIndexTemp.y == 32) 
				{
					vStartIndexTemp.y = 0;
					vStartIndexTemp.x++;
				}
				amountCountTemp -= xyMass;
			}
			amountCountTemp = amountCount;
			vStartIndexTemp = vStartIndex;
			while(amountCountTemp) 
			{
				int xMassMax = dimensions.y * dimensions.z - (vStartIndexTemp.z + vStartIndexTemp.y * dimensions.z);
				int xMass = std::min(xMassMax, amountCountTemp);

				xSliceMass[vStartIndexTemp.x] += xMass;
				vStartIndexTemp.x++;
				vStartIndexTemp.y = 0;
				vStartIndexTemp.z = 0;
				amountCountTemp -= xMass;
			}
		}
		for(int i = 0; i < amountCount; i++) 
		{
			amounts[i + amountTotalCount] = amount;
		}
		amountTotalCount += amountCount;
		index += 2;
	}
	if(amountTotalCount != datalength) 
	{
		throw std::runtime_error("Bad chunk, amount total is not equal data length");
	}

	while(voxelTypeIdTotalCount < datalength) 
	{
		int voxelTypeIdCount = data[index];
		int voxelTypeId = data[index + 1];
		if(voxelTypeIdTotalCount+voxelTypeIdCount > datalength) 
		{
			throw std::runtime_error("Bad chunk, id total is greater than data length");
		}

		for(int i = 0; i < voxelTypeIdCount; i++) 
		{
			voxelTypeIds[voxelTypeIdTotalCount + i] = voxelTypeId;
			bool isSolid = amounts[voxelTypeIdTotalCount + i] > 0;
			const int v1dind = voxelTypeIdTotalCount + i;
			const glm::ivec3 v3dInd = a1to3ind(v1dind, dimensions.y, dimensions.z);
		}
		voxelTypeIdTotalCount += voxelTypeIdCount;
		index += 2;
	}
	if(voxelTypeIdTotalCount != datalength) 
	{
		throw std::runtime_error("Bad chunk, id total is not equal data length");
	}
}

unsigned long long VoxelData::getChunkFileLastModify(const std::filesystem::path& path) 
{
	if(std::filesystem::exists(path)) 
	{
		const auto headersSize = sizeof(VoxelDataVersionHeader) + sizeof(VoxelDataHeaderV1);
		std::ifstream ifs(path, std::ios::binary|std::ios::ate);
		if(!ifs) return ULONG_LONG_MAX;
		std::ifstream::pos_type pos = ifs.tellg();
		if(pos >= headersSize) 
		{
			ifs.seekg(0, std::ios::beg);
			std::vector<char> bytes(headersSize);
			ifs.read(&bytes[0], headersSize);
			return ((VoxelDataHeaderV1*)(bytes.data() + sizeof(VoxelDataVersionHeader)))->lastModify;
		}
	}
	unsigned long long noRevision;
	randomBytes((char*)&noRevision, sizeof(noRevision));
	noRevision |= 0b1111ULL << 60ULL;
	return noRevision;
}