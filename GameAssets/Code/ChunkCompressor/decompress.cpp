#include "ChunkCompressor.h"
#include "Chunk.h"
#include "ChunkCompressImplCommon.h"
#include <exception>
template <class T>
T read(const char* data, unsigned int& cursor, unsigned int maxSize) 
{
	T readResult;
	if(cursor + sizeof(T) > maxSize) 
	{
		throw std::range_error("Ran out of bounds");
	}
	memcpy(&readResult, data + cursor, sizeof(T));
	cursor += sizeof(T);
	return readResult;
}

void ChunkCompressor::decompressChunk(Chunk* chunk, const char* data, unsigned int size) 
{
	unsigned int readCursor = 0;
	int voxelTypesLeft = 32*32*32;
	clear();

	ChunkDataVersionHeader versionHeader = read<ChunkDataVersionHeader>(data, readCursor, size);
	ChunkDataHeaderV2 header = read<ChunkDataHeaderV2>(data, readCursor, size);
	if(
		versionHeader.majik != ChunkDataSaveMajikV2 ||
		versionHeader.version != currentVersion
	) 
	{
		throw std::runtime_error("Invalid chunk format");
	}
	chunk->voxelData.lastModify = header.lastModify;
	
	bool hasTiles = header.flags & ChunkDataHeaderV2Flags::hasTiles;
	bool hasFluid = header.flags & ChunkDataHeaderV2Flags::hasWater;

	VoxelData& voxels = chunk->voxelData;
	memset(voxels.xSliceMass, 0, sizeof(int) * width);
	memset(voxels.xyLineMass, 0, sizeof(int) * width * width);
	unsigned short currentType = 0;
	for(unsigned int x = 0; x < width; x++) 
	{
		for(unsigned int y = 0; y < width; y++) 
		{
			for(unsigned int z = 0; z < width; z++) 
			{
				if(voxelTypesLeft == 0) 
				{
					x = y = z = width;
					break;
				}
				if(getBit(voxelTypeVisited, x,y,z)) continue;
				unsigned short cube = read<unsigned short>(data, readCursor, size);
				if(cube & packedCubeNewTypeFlag) 
				{
					currentType = read<unsigned short>(data, readCursor, size);
				}
				unsigned int w, h, l;
				unpackCubeDimensions(cube, w, h, l);
				if(w > 32 || h > 32 || l > 32) 
				{
					throw std::runtime_error("Shouldn't be possible");
				}
				voxelTypesLeft -= w * h * l;
				if(voxelTypesLeft < 0) 
				{
					throw std::runtime_error("Too many voxel types");
				}
				for(int i = 0; i < w; i++)
				{
					if(currentType) voxels.xSliceMass[x + i] += h * l;
					for(int j = 0; j < h; j++) 
					{
						if(currentType) voxels.xyLineMass[y + j + (x + i) * width] += l;
						for(int k = 0; k < l; k++) 
						{
							if(getBit(voxelTypeVisited, x+i,y+j,z+k) == true) 
							{
								throw std::runtime_error("Overlaping cubes");
							}
							setBit(voxelTypeVisited, x+i,y+j,z+k, true);
							chunk->voxelData.voxelTypeIds[a3to1ind(x+i,y+j,z+k, width, width)] = currentType;
						}
					}
				}
			}
		}
	}

	signed char amount;
	unsigned int amountsLeft = 0;
	unsigned int writes = 0;
	for(unsigned int x = 0; x < width; x++) 
	{
		for(unsigned int y = 0; y < width; y++) 
		{
			for(unsigned int z = 0; z < width; z++) 
			{
				unsigned int flatIndex = a3to1ind(x,y,z, width, width);
				writes++;
				if(amountsLeft) 
				{
					voxels.amounts[flatIndex] = voxels.voxelTypeIds[flatIndex] ? amount : -amount;
					amountsLeft--;
				}
				else 
				{
					unsigned char packedAmount = read<unsigned char>(data, readCursor, size);
					amount = packedAmount & 0b01111111u;
					if(packedAmount & 0b10000000) 
					{
						amountsLeft = read<unsigned char>(data, readCursor, size);
						amountsLeft += 1;
					}
					voxels.amounts[flatIndex] = voxels.voxelTypeIds[flatIndex] ? amount : -amount;
				}
			}	
		}
	}
	TileMap& tiles = chunk->tileMap;
	if(hasTiles) 
	{
		tiles.cubeTileCount = 0;
		currentType = 0;
		memset(voxelTypeVisited, 0, sizeof(voxelTypeVisited));
		voxelTypesLeft = 32*32*32;
		for(unsigned int x = 0; x < width; x++) 
		{
			for(unsigned int y = 0; y < width; y++) 
			{
				for(unsigned int z = 0; z < width; z++) 
				{
					if(voxelTypesLeft == 0) 
					{
						x = y = z = width;
						break;
					}
					if(getBit(voxelTypeVisited, x,y,z)) continue;
					unsigned short cube = read<unsigned short>(data, readCursor, size);
					if(cube & packedCubeNewTypeFlag) 
					{
						currentType = read<unsigned short>(data, readCursor, size);
					}
					unsigned int w, h, l;
					unpackCubeDimensions(cube, w, h, l);
					if(w > 32 || h > 32 || l > 32) 
					{
						throw std::runtime_error("Shouldn't be possible");
					}
					unsigned int cubeVolume = w * h * l;
					voxelTypesLeft -= cubeVolume;
					if(currentType != 0) 
					{
						tiles.cubeTileCount += cubeVolume;
					}
					if(voxelTypesLeft < 0) 
					{
						throw std::runtime_error("Too many voxel types");
					}
					for(int i = 0; i < w; i++)
					{
						for(int j = 0; j < h; j++) 
						{
							for(int k = 0; k < l; k++) 
							{
								if(getBit(voxelTypeVisited, x+i,y+j,z+k) == true) 
								{
									throw std::runtime_error("Overlaping cubes");
								}
								setBit(voxelTypeVisited, x+i,y+j,z+k, true);
								tiles.cubeMaterials[a3to1ind(x+i,y+j,z+k, width, width)] = currentType;
							}
						}
					}
				}
			}
		}
	}

	if(hasFluid) 
	{
		const unsigned int waterId = 1;
		unsigned int columnsLevel = 0;
		unsigned int columnsLeft = 0;
		FluidSim& fluid = chunk->fluidSim;
		fluid.totalFluid = 0;
		for(unsigned int x = 0; x < width; x++) 
		{
			for(unsigned int z = 0; z < width; z++) 
			{
				for(unsigned int y = 0; y < width; y++) 
				{
					unsigned int flatIndex = a3to1ind(x,y,z,width,width);
					fluid.cells[flatIndex].type = canStoreWater(chunk, flatIndex) ? fluidSimAirType : fluidSimSolidType;
					fluid.cells[flatIndex].value = 0;
					if(canStoreWater(chunk, flatIndex)) 
					{
						if(columnsLeft == 0) 
						{
							unsigned int firstPart = read<unsigned short>(data, readCursor, size);
							unsigned int secondPart = read<unsigned char>(data, readCursor, size);
							columnsLevel = firstPart & ~(0b111u << 13u);
							columnsLeft = (secondPart) + (((firstPart & (0b111u << 13u)) >> 13u) << 8u);
							columnsLeft++;
						}
						unsigned int columnLevelLeft = columnsLevel;
						fluid.totalFluid += columnsLevel;
						while(y < width && canStoreWater(chunk, flatIndex)) 
						{
							fluid.cells[flatIndex].value = columnLevelLeft > 255u ? 255u : columnLevelLeft;
							fluid.cells[flatIndex].type = columnLevelLeft ? waterId : 0;
							columnLevelLeft -= fluid.cells[flatIndex].value;
							y++;
							flatIndex += width;
						}
						columnsLeft--;
						if(columnLevelLeft) throw std::runtime_error("Fluid not consumed");
					}
 				}
			}
		}
	}
}