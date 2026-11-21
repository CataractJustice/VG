#include "ChunkCompressor.h"
#include "Chunk.h"
#include "ChunkCompressImplCommon.h"

char* ChunkCompressor::compressChunk(Chunk* chunk, unsigned int& size) 
{
	clear();
	ChunkDataVersionHeader versionHeader;
	ChunkDataHeaderV2 header;
	versionHeader.majik = ChunkDataSaveMajikV2;
	versionHeader.version = currentVersion;
	header.lastModify = chunk->voxelData.lastModify;
	header.flags = 0;
	if(!chunk->tileMap.isEmpty()) header.flags |= ChunkDataHeaderV2Flags::hasTiles;
	if(chunk->fluidSim.getTotalFluid()) header.flags |= ChunkDataHeaderV2Flags::hasWater;
	write(versionHeader);
	write(header);
	unsigned short lastType = 0;
	VoxelData& voxels = chunk->voxelData;
	unsigned int flatIndex = 0;
	for(unsigned int x = 0; x < width; x++) 
	{
		for(unsigned int y = 0; y < width; y++) 
		{
			for(unsigned int z = 0; z < width; z++) 
			{
				if(getBit(voxelTypeVisited, x,y,z)) 
				{
					flatIndex++;
					continue;
				}
				unsigned short& originVoxelType = voxels.voxelTypeIds[flatIndex];
				signed char& originVoxelAmount = voxels.amounts[flatIndex];
				if(originVoxelAmount <= 0) 
				{
					originVoxelType = 0;
				}

				unsigned short recordType;
				recordType = originVoxelType;

				unsigned int w = 1;
				unsigned int h = 1;
				unsigned int l = 1;
				setBit(voxelTypeVisited, x,y,z, true);
				//Expand Z
				unsigned int checkFlatIndex = flatIndex + 1;
				for(unsigned int checkZ = z + 1; checkZ < width; checkZ++) 
				{
					if(getBit(voxelTypeVisited, x,y,checkZ) == true) break;
					unsigned short& checkedType = voxels.voxelTypeIds[checkFlatIndex];
					signed char checkedAmount = voxels.amounts[checkFlatIndex];
					if(checkedAmount <= 0) checkedType = 0;
					if(
						checkedType != recordType
					) break;
					l++;
					checkFlatIndex++;
					setBit(voxelTypeVisited, x,y,checkZ, true);
				}
				//Expand Y
				checkFlatIndex = flatIndex + width;
				for(unsigned int checkY = y + 1; checkY < width; checkY++) 
				{
					bool canExpand = true;
					for(int i = 0; i < l; i++) 
					{
						canExpand = !getBit(voxelTypeVisited, x,checkY,z+i);
						if(!canExpand) break;
						unsigned short& checkedType = voxels.voxelTypeIds[checkFlatIndex];
						signed char checkedAmount = voxels.amounts[checkFlatIndex];
						if(checkedAmount <= 0) checkedType = 0;
						if(
							checkedType != recordType
						) 
						{
							canExpand = false;
							break;
						}
						checkFlatIndex++;
					}
					checkFlatIndex += width - l;
					if(!canExpand) break;
					for(int i = 0; i < l; i++) 
					{
						setBit(voxelTypeVisited, x, checkY, z+i, true);
					}
					h++;
				}
				//Expand X
				checkFlatIndex = flatIndex + width2;
				for(unsigned int checkX = x + 1; checkX < width; checkX++) 
				{
					bool canExpand = true;
					for(int i = 0; i < h && canExpand; i++) 
					{
						for(int j = 0; j < l; j++) 
						{
							unsigned short& checkedType = voxels.voxelTypeIds[checkFlatIndex];
							signed char checkedAmount = voxels.amounts[checkFlatIndex];
							if(checkedAmount <= 0) checkedType = 0;
							if(
								checkedType != recordType
							)
							{
								canExpand = false;
								break;
							}
							checkFlatIndex++;
						}
						checkFlatIndex += width - l;
					}
					if(!canExpand) break;
					checkFlatIndex += width2 - width * h;
					for(int i = 0; i < h; i++) 
					for(int j = 0; j < l; j++) 
					{
						setBit(voxelTypeVisited, checkX, y+i, z+j, true);
					}
					w++;
				}
				unsigned short cube = packCubeDimensions(w,h,l);
				if(recordType == lastType) 
				{
					write(cube);
				}
				else 
				{
					cube |= packedCubeNewTypeFlag;
					write(cube);
					write(recordType);
				}
				lastType = recordType;
				flatIndex++;
			}	
		}
	}
	unsigned int lastAmount = INT_MAX;
	unsigned int repeatsCount = 0;
	flatIndex = 0;
	for(unsigned int x = 0; x < width; x++) 
	{
		for(unsigned int y = 0; y < width; y++) 
		{
			for(unsigned int z = 0; z < width; z++) 
			{
				signed char amount = voxels.amounts[flatIndex];
				unsigned char unsignedAmount = amount < 0 ? -amount : amount;
				if(lastAmount != unsignedAmount || repeatsCount == 256) 
				{
					//if repeats count == 256
					if(repeatsCount) compressionBuffer[writeCursor - 1] = repeatsCount - 1;
					write(unsignedAmount);
					lastAmount = unsignedAmount;
					repeatsCount = 0;
				}
				else 
				{
					if(!repeatsCount) 
					{
						compressionBuffer[writeCursor - 1] |= 0b10000000u;
						writeCursor++;
					}
					compressionBuffer[writeCursor - 1] = repeatsCount;
					repeatsCount++;
				}
				flatIndex++;
			}	
		}
	}

	TileMap& tiles = chunk->tileMap;
	memset(voxelTypeVisited, 0, sizeof(voxelTypeVisited));
	lastType = 0;
	flatIndex = 0;
	if(!tiles.isEmpty()) 
	{
		for(unsigned int x = 0; x < width; x++) 
		{
			for(unsigned int y = 0; y < width; y++) 
			{
				for(unsigned int z = 0; z < width; z++) 
				{
					if(getBit(voxelTypeVisited, x,y,z)) 
					{
						flatIndex++;
						continue;
					}
					unsigned short& originCubeType = tiles.cubeMaterials[flatIndex];

					unsigned int w = 1;
					unsigned int h = 1;
					unsigned int l = 1;
					setBit(voxelTypeVisited, x,y,z, true);
					unsigned int flatCheckIndex = flatIndex + 1;
					//Expand Z
					for(unsigned int checkZ = z + 1; checkZ < width; checkZ++) 
					{
						if(getBit(voxelTypeVisited, x,y,checkZ) == true) break;
						const unsigned short checkedType = tiles.cubeMaterials[flatCheckIndex];
						if(
							checkedType != originCubeType
						) break;
						l++;
						flatCheckIndex++;
						setBit(voxelTypeVisited, x,y,checkZ, true);
					}
					//Expand Y
					flatCheckIndex = flatIndex + width;
					for(unsigned int checkY = y + 1; checkY < width; checkY++) 
					{
						bool canExpand = true;
						for(int i = 0; i < l; i++) 
						{
							canExpand = !getBit(voxelTypeVisited, x,checkY,z+i);
							if(!canExpand) break;
							const unsigned short checkedType = tiles.cubeMaterials[flatCheckIndex];
							if(
								checkedType != originCubeType
							) 
							{
								canExpand = false;
								break;
							}
							flatCheckIndex++;
						}
						flatCheckIndex += width - l;
						if(!canExpand) break;
						for(int i = 0; i < l; i++) 
						{
							setBit(voxelTypeVisited, x, checkY, z+i, true);
						}
						h++;
					}
					//Expand X
					flatCheckIndex = flatIndex + width2;
					for(unsigned int checkX = x + 1; checkX < width; checkX++) 
					{
						bool canExpand = true;
						for(int i = 0; i < h && canExpand; i++) 
						{
							for(int j = 0; j < l; j++) 
							{	
								if(flatCheckIndex != a3to1ind(glm::ivec3(checkX, y + i, z + j), 32, 32)) throw std::logic_error("Invalid index calculated");
								unsigned short& checkedType = tiles.cubeMaterials[flatCheckIndex];
								if(
									checkedType != originCubeType
								)
								{
									canExpand = false;
									break;
								}
								flatCheckIndex++;
							}
							flatCheckIndex += width - l;
						}
						if(!canExpand) break;
						flatCheckIndex += width2 - width * h;
						for(int i = 0; i < h; i++) 
						for(int j = 0; j < l; j++) 
						{
							setBit(voxelTypeVisited, checkX, y+i, z+j, true);
						}
						w++;
					}
					unsigned short cube = packCubeDimensions(w,h,l);
					if(originCubeType == lastType) 
					{
						write(cube);
					}
					else 
					{
						cube |= packedCubeNewTypeFlag;
						write(cube);
						write(originCubeType);
					}
					lastType = originCubeType;
					flatIndex++;
				}	
			}
		}
	}

	FluidSim& fluid = chunk->fluidSim;
	const unsigned int waterId = 1;
	unsigned int lastColumnLevel = ~0u;
	unsigned int runLength = 1;
	if(fluid.totalFluid) 
	{
		for(unsigned int x = 0; x < width; x++) 
		{
			for(unsigned int z = 0; z < width; z++) 
			{
				for(unsigned int y = 0; y < width; y++) 
				{
					flatIndex = x * width2 + y * width + z;
					unsigned int currentColumnLevel = 0;
					bool hasColumn = false;
					while(y < width && canStoreWater(chunk, flatIndex)) 
					{
						hasColumn = true;
						currentColumnLevel += fluid.cells[flatIndex].type == waterId ? fluid.cells[flatIndex].value : 0;
						y++;
						flatIndex += width;
					}
					if(hasColumn) 
					{
						if(lastColumnLevel == currentColumnLevel && runLength != (1u << 11u)) 
						{
							runLength++;
							unsigned short runLengthPack = runLength - 1;
							compressionBuffer[writeCursor - 1] = runLengthPack & 0xffu;
							unsigned short& levelPack = *(unsigned short*)&compressionBuffer[writeCursor - 3];
							levelPack &= ~(0b111 << 13u);
							levelPack |= (runLengthPack & (0b111 << 8u)) << 5u;
						}
						else 
						{
							unsigned short levelPack = currentColumnLevel;
							unsigned char runLengthPack = 0;
							write(levelPack);
							write(runLengthPack);
							lastColumnLevel = currentColumnLevel;
							runLength = 1;
						}
					}
				}
			}

		}
	}
	size = writeCursor;
	return compressionBuffer;
}