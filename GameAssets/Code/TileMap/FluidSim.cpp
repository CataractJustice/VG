#include "FluidSim.h"
#include "Chunk.h"
#include <math.h>
#include "Math/IndexMath.h"

FluidSim::FluidCell FluidSim::outOfBoundsCell = {0, 0};

FluidSim::FluidSim() : updateQueue()
{
	wasCompressed = wasDecompressed = false;
	random = rand();
	memset(cells, 0, sizeof(cells));
	memset(updateMask, 0, sizeof(updateMask));
	totalFluid = 0;
	
	memset(textureSegmentUploadMask, 0, sizeof(textureSegmentUploadMask));
	glGenTextures(1, &fluidSampler);
	glBindTexture(GL_TEXTURE_3D, fluidSampler);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	GLuint borderColor[] = {0, 0, 0, 0};
	glTexParameterIuiv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, borderColor);
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
		cells
	);
}

void FluidSim::markLineForSync(unsigned int x, unsigned int y) 
{
	networkLinesUpdateMask[x] |= 1U << y;
}


char networkFluidSyncDataBuffer[fluidSimWidth*fluidSimWidth*fluidSimWidth*8];

const char* FluidSim::getNetworkSyncBuffer(unsigned int& sizeOut) 
{
	memcpy(networkFluidSyncDataBuffer, networkLinesUpdateMask, sizeof(networkLinesUpdateMask));
	sizeOut = sizeof(networkLinesUpdateMask);
	unsigned int lineStart = 0;
	for(int i = 0; i < fluidSimWidth; i++) 
	{
		if(!networkLinesUpdateMask[i] == 0) continue;
		for(int j = 0; j < fluidSimWidth; j++) 
		{
			if((networkLinesUpdateMask[i] & 1U << j) == 0) 
			{
				lineStart += fluidSimWidth;
				continue;
			}
			unsigned int flatIndex = lineStart;
			for(int k = 0; k < fluidSimWidth; k++) 
			{
				networkFluidSyncDataBuffer[sizeOut] = cells[flatIndex].value;
				sizeOut++;
				flatIndex++;
			}
			lineStart += fluidSimWidth;
		}
	}
	memset(networkLinesUpdateMask, 0, sizeof(networkLinesUpdateMask));
	return networkFluidSyncDataBuffer;
}

bool isFluid(FluidSim::FluidCell& cell) 
{
	return cell.type != fluidSimAirType && cell.type != fluidSimSolidType && cell.value != 0;
}

void FluidSim::update(int updateCount) 
{
	auto qTemp = updateQueue;
	unsigned int processed = 0;
	while(updateCount && qTemp.size()) 
	{
		updateCount--;
		unsigned int updateCell = qTemp.front();
		qTemp.pop();
		updateFlow(a1to3ind(updateCell, fluidSimWidth, fluidSimWidth));
		processed++;
	}
	for(int i = 0; i < processed; i++) updateQueue.pop();
}
void FluidSim::updateFlow(glm::ivec3 index) 
{
	queueTextureUpload(index);
	//unset update mask
	updateMask[index.x * fluidSimWidth + index.y] &= ~(1u << index.z);
	//should neigbors be updated
	bool updateFlag = false;
	FluidCell& thisCell = getCell(index);
	FluidCell originalValue = thisCell;
	//check if cell is fluid
	if(thisCell.type == fluidSimSolidType || thisCell.type == fluidSimAirType || thisCell.value == 0) return;
	//flow down
	FluidSim* bottomSim;
	FluidCell& bottom = getCell(index - glm::ivec3(0, 1, 0), bottomSim);
	if(bottomSim && bottom.type != fluidSimSolidType && (bottom.type == fluidSimAirType || (bottom.type == thisCell.type && bottom.value < fluidSimMaxValue))) 
	{
		bottom.type = thisCell.type;
		const unsigned int maxColumnHeight = 32;
		unsigned char bottomIntake = fluidSimMaxValue - bottom.value;
		FluidCell* cellColumn[maxColumnHeight];
		FluidSim* cellColumnSims[maxColumnHeight];
		unsigned int columnHeight = 1;
		cellColumn[0] = &thisCell;
		cellColumnSims[0] = this;
		for("What's up"; columnHeight < maxColumnHeight; columnHeight++) 
		{
			cellColumn[columnHeight] = &getCell(index + glm::ivec3(0, columnHeight, 0), cellColumnSims[columnHeight]);
			if(cellColumn[columnHeight]->type != thisCell.type || cellColumn[columnHeight]->value == 0) 
			{
				break;
			}
		}

		unsigned int columnIterator = columnHeight;
		while(columnIterator && bottomIntake) 
		{
			columnIterator--;
			const unsigned char flowDelta = std::min(cellColumn[columnIterator]->value, bottomIntake);
			bottom.value += flowDelta;
			bottomSim->totalFluid += flowDelta;
			cellColumnSims[columnIterator]->totalFluid -= flowDelta;
			cellColumn[columnIterator]->value -= flowDelta;
			bottomIntake -= flowDelta;


			queueUpdateFlow(index + glm::ivec3(1, columnIterator, 0));
			queueUpdateFlow(index - glm::ivec3(1, columnIterator, 0));
			queueUpdateFlow(index + glm::ivec3(0, columnIterator, 1));
			queueUpdateFlow(index - glm::ivec3(0, columnIterator, 1));
		}

		queueUpdateFlow(index + glm::ivec3(0, columnHeight, 0));
		queueUpdateFlow(index - glm::ivec3(0, 1, 0));
		//glBindTexture(GL_TEXTURE_3D, this->fluidSampler);
		//glTexSubImage3D(GL_TEXTURE_3D, 0, index.x, index.y, index.z, 1, 1, 1, GL_RG, GL_UNSIGNED_BYTE, &thisCell);
		//glBindTexture(GL_TEXTURE_3D, bottomSim->fluidSampler);
		//glTexSubImage3D(GL_TEXTURE_3D, 0, index.x, realIMod(index.y, fluidSimWidth), index.z, 1, 1, 1, GL_RG, GL_UNSIGNED_BYTE, &bottom);
		
	}
	skipFlowDown:

	glm::ivec3 nIndex[] = 
	{
		glm::ivec3( 1,  0,  0),
		glm::ivec3( 0,  0,  1),
		glm::ivec3(-1,  0,  0),
		glm::ivec3( 0,  0, -1),
		glm::ivec3( 0,  0,  0)
	};

	FluidSim* fSims[5];
	FluidCell* fCells[5];
	
	unsigned int originalValues[5];
	bool fluidCellMask[5];

	unsigned int fluid = 0;
	unsigned int flowCellCount = 0;
	unsigned int minFluid = 256;
	unsigned int maxFluid = 0;

	for(int i = 0; i < 5; i++) 
	{
		FluidSim* sim = nullptr;
		FluidCell& cell = getCell(index + nIndex[i], sim);
		fSims[i] = sim;
		fCells[i] = &cell;
		if(sim == nullptr) 
		{
			fluidCellMask[i] = false;
			continue;
		}
		if(cell.type == fluidSimAirType) cell.value = 0;
		if(cell.type != fluidSimSolidType) 
		{
			fluid += cell.value;
			flowCellCount++;
			fluidCellMask[i] = true;
			originalValues[i] = cell.value;
			minFluid = (minFluid < cell.value) ? minFluid : cell.value;
			maxFluid = (maxFluid > cell.value) ? maxFluid : cell.value;
		}
		else 
		{
			fluidCellMask[i] = false;
		}
	}

	if((maxFluid - minFluid) > 4 && thisCell.value > 0)
	{
		unsigned int average = fluid / flowCellCount;
		unsigned int rest = fluid - (average * flowCellCount);

		for(int i = 0; i < 5; i++) 
		{
			if(fluidCellMask[i] == false) continue;
			FluidSim* sim = fSims[i];
			FluidCell& cell = *fCells[i];
			sim->totalFluid -= cell.value;
			cell.value = average;
			cell.type = 1;
			sim->totalFluid += average;
		}
		unsigned int ind = random % 5;
		random++;
		while(rest) 
		{
			ind++;
			ind %= 5;
			if(fluidCellMask[ind] == false) 
			{
				continue;
			}
			FluidSim* sim = fSims[ind];
			FluidCell& cell = *fCells[ind];
			cell.value++;
			rest--;
			sim->totalFluid++;
		}

		for(int i = 0; i < 5; i++) 
		{
			if(fluidCellMask[i] == false) continue;
			FluidSim* sim = fSims[i];
			FluidCell& cell = *fCells[i];
			glm::ivec3 updFlowIndex = index + nIndex[i];
			if(sim != this) updFlowIndex -= nIndex[i] * glm::ivec3(fluidSimWidth);
			if(cell.value != originalValues[i]) sim->queueUpdateFlow(updFlowIndex);
		}
	}

	/*sif(originalValue.type != thisCell.type || originalValue.value != thisCell.value) 
	{
		queueTextureUpload(index);
	}*/
	/*
	glm::ivec3 horNeighborIndexes[] = 
	{
		index + glm::ivec3(1, 0, 0) ,
		index + glm::ivec3(-1, 0, 0),
		index + glm::ivec3(0, 0, 1) ,
		index + glm::ivec3(0, 0, -1)
	};

	FluidCell* horNeighbors[] = 
	{
		&getCell(horNeighborIndexes[0]),
		&getCell(horNeighborIndexes[1]),
		&getCell(horNeighborIndexes[2]),
		&getCell(horNeighborIndexes[3])
	};

	FluidSim* horNeighborFluidSims[] = 
	{
		getFluidSimAt(horNeighborIndexes[0]),
		getFluidSimAt(horNeighborIndexes[1]),
		getFluidSimAt(horNeighborIndexes[2]),
		getFluidSimAt(horNeighborIndexes[3])
	};
	
	unsigned char neighborValues[] = 
	{
		horNeighbors[0]->value,
		horNeighbors[1]->value,
		horNeighbors[2]->value,
		horNeighbors[3]->value
	};
	unsigned char thisValue = thisCell.value;

	unsigned int fluidLevel = thisCell.value;
	unsigned int fluidLevelCellCount = isFluid(thisCell) ? 1 : 0;
	bool horisontalFlow = false;
	for(int n = 0; n < 4; n++) 
	{
		if(horNeighbors[n]->type == fluidSimAirType) horisontalFlow = true;
		if(horNeighbors[n]->type == thisCell.type) 
		{
			fluidLevel += horNeighbors[n]->value;
			fluidLevelCellCount++;
			if(int(thisCell.value) > int(horNeighbors[n]->value) + 1) 
			{
				horisontalFlow = true;
			}
		}
	}	
	//horisontalFlow = false;
	
	if(horisontalFlow) 
	{
		unsigned int random = randUInt();
		unsigned int fluidLevelSplit = fluidLevel / fluidLevelCellCount;
		unsigned int fluidLevelRest = fluidLevel - fluidLevelSplit;
		thisValue = fluidLevelSplit;
		if(fluidLevelRest && random & 4) 
		{
			thisValue++;
			fluidLevelRest--;
		}
		for(int n = 0; n < 4; n++) 
		{
			FluidCell& neighbor = *horNeighbors[(n + random) & 3];
			if(neighbor.type != thisCell.type) continue;
			neighborValues[(n + random) & 3] = fluidLevelSplit;
			if(fluidLevelRest) 
			{
				neighborValues[(n + random) & 3]++;
			}
		}
		if(fluidLevelRest) thisValue++;
		setCell(index, thisValue, thisCell.type);
		//glBindTexture(GL_TEXTURE_3D, this->fluidSampler);
		//glTexSubImage3D(GL_TEXTURE_3D, 0, index.x, index.y, index.z, 1, 1, 1, GL_RG, GL_UNSIGNED_BYTE, &thisCell);
		for(int n = 0; n < 4; n++) 
		{
			if(!horNeighborFluidSims[n]) continue;
			setCell(horNeighborIndexes[n], neighborValues[n], horNeighbors[n]->type);
			//glBindTexture(GL_TEXTURE_3D, horNeighborFluidSims[n]->fluidSampler);
			//glm::ivec3 nmodIndex = ivec3RealMod(horNeighborIndexes[n], glm::ivec3(fluidSimWidth));
			//glTexSubImage3D(GL_TEXTURE_3D, 0, nmodIndex.x, nmodIndex.y, nmodIndex.z, 1, 1, 1, GL_RG, GL_UNSIGNED_BYTE, horNeighbors[n]);
		}
	}*/
}

FluidSim* FluidSim::getFluidSimAtMutIndex(glm::ivec3& index) 
{
	glm::ivec3 neighborIndex = glm::ivec3(0);
	if(index.x < 0) 
	{
		neighborIndex.x = -1;
		index.x += fluidSimWidth;
	}
	if(index.y < 0) 
	{
		neighborIndex.y = -1;
		index.y += fluidSimWidth;
	}
	if(index.z < 0) 
	{
		neighborIndex.z = -1;
		index.z += fluidSimWidth;
	}
	if(index.x >= fluidSimWidth) 
	{
		neighborIndex.x = 1;
		index.x -= fluidSimWidth;
	}
	if(index.y >= fluidSimWidth) 
	{
		neighborIndex.y = 1;
		index.y -= fluidSimWidth;
	}
	if(index.z >= fluidSimWidth) 
	{
		neighborIndex.z = 1;
		index.z -= fluidSimWidth;
	}
	if(neighborIndex.x || neighborIndex.y || neighborIndex.z) 
	{
		Chunk* neighborChunk = parentChunk->getNeighbor(neighborIndex);
		if(!neighborChunk) return nullptr;
		return neighborChunk->fluidSim.getFluidSimAtMutIndex(index);
	}
	return this;
}

FluidSim* FluidSim::getFluidSimAt(glm::ivec3 index) 
{
	return getFluidSimAtMutIndex(index);
}

void FluidSim::queueUpdateFlow(glm::ivec3 index) 
{
	FluidSim* sim = getFluidSimAtMutIndex(index);
	if(!sim) return;
	if(sim->updateMask[index.x * fluidSimWidth + index.y] & 1u << index.z) return;
	sim->updateMask[index.x * fluidSimWidth + index.y] |= 1u << index.z;
	unsigned int flatIndex = a3to1ind(index, fluidSimWidth, fluidSimWidth);	
	if(flatIndex > sizeof(cells) / sizeof(FluidCell)) throw std::range_error("Update mark out of bounds");
	sim->updateQueue.emplace(flatIndex);
}

void FluidSim::queueUpdateFlowParallel(glm::ivec3 index) 
{
	FluidSim* sim = getFluidSimAtMutIndex(index);
	if(!sim) return;
	if(sim->updateMaskParallel[index.x * fluidSimWidth + index.y] & 1u << index.z) return;
	sim->updateMaskParallel[index.x * fluidSimWidth + index.y] |= 1u << index.z;
	unsigned int flatIndex = a3to1ind(index, fluidSimWidth, fluidSimWidth);	
	if(flatIndex > sizeof(cells) / sizeof(FluidCell)) throw std::range_error("Update mark out of bounds");
	sim->parallelUpdateQueue.emplace(flatIndex);
}


FluidSim::FluidCell& FluidSim::getCell(glm::ivec3 index) 
{
	FluidSim* sim = getFluidSimAtMutIndex(index);
	if(!sim) return outOfBoundsCell;
	unsigned char type = sim->cells[a3to1ind(index, fluidSimWidth, fluidSimWidth)].type;
	if(type != fluidSimAirType && type != fluidSimSolidType && type != 1) 
	{
		throw std::logic_error("Fuck");
	}
	return sim->cells[a3to1ind(index, fluidSimWidth, fluidSimWidth)];
}

FluidSim::FluidCell& FluidSim::getCell(glm::ivec3 index, FluidSim*& simPtr) 
{
	simPtr = getFluidSimAtMutIndex(index);
	if(!simPtr) return outOfBoundsCell;
	return simPtr->cells[a3to1ind(index, fluidSimWidth, fluidSimWidth)];
}

void FluidSim::setCell(glm::ivec3 index, unsigned char value, unsigned char type, bool updateFlag) 
{
	networkLinesUpdateMask[index.x] |= 1U << index.y;
	if(type != fluidSimAirType && type != fluidSimSolidType && type != 1) 
	{
		throw std::logic_error("Invalid type");
	}
	if(value == 0 && type != fluidSimSolidType) type = fluidSimAirType;
	FluidSim* sim = getFluidSimAtMutIndex(index);
	if(sim == nullptr) return;
	FluidCell& cell = sim->cells[a3to1ind(index, fluidSimWidth, fluidSimWidth)];
	bool cellWasFluid = cell.type != fluidSimSolidType && cell.type != fluidSimAirType;
	bool cellWillBeFluid = type != fluidSimSolidType && type != fluidSimAirType && value != 0;
	sim->totalFluid += (cellWillBeFluid ? value : 0);
	sim->totalFluid -= (cellWasFluid ? cell.value : 0);
	cell.type = type;
	cell.value = value;

	if(updateFlag) 
	{
		if(type == fluidSimSolidType) 
		{
			//sim->queueUpdateFlow(index + glm::ivec3(0,1,0));
			//sim->queueUpdateFlow(index + glm::ivec3(1,0,0));
			//sim->queueUpdateFlow(index - glm::ivec3(1,0,0));
			//sim->queueUpdateFlow(index + glm::ivec3(0,0,1));
			//sim->queueUpdateFlow(index - glm::ivec3(0,0,1));
		}
		else
		if(type == fluidSimAirType) 
		{
			sim->queueUpdateFlowParallel(index + glm::ivec3(0,1,0));
			sim->queueUpdateFlowParallel(index + glm::ivec3(1,0,0));
			sim->queueUpdateFlowParallel(index - glm::ivec3(1,0,0));
			sim->queueUpdateFlowParallel(index + glm::ivec3(0,0,1));
			sim->queueUpdateFlowParallel(index - glm::ivec3(0,0,1));
		}
		else
		if(isFluid(cell)) 
		{
			sim->queueUpdateFlowParallel(index + glm::ivec3(0,0,0));
		}
	}
}

void FluidSim::render(glm::vec3 offset, Kirino::StateColletion& stateCollection, const FrustumPlanes& frustum, const RenderTexturesCollection& renderTextures) 
{
	
}

void FluidSim::uploadTexture() 
{
	if(!textureDirty && textureUploadQueue.size() == 0) return;
	glBindTexture(GL_TEXTURE_3D, fluidSampler);
	if(textureDirty) 
	{
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
			cells
		);
		textureDirty = false;
	}
	else 
	{
		const unsigned int segmentsWidth = fluidSimWidth / textureSegmentSize;
		if(textureUploadQueue.size() == 0) return;
		memset(textureSegmentUploadMask, 0, sizeof(textureSegmentUploadMask));
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, fluidSimWidth);
		glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, fluidSimWidth);
		while(textureUploadQueue.size()) 
		{
			const unsigned int segmentIndex = dequeueTextureUpload();
			const glm::ivec3 segmentOffset = glm::ivec3(textureSegmentSize) * a1to3ind(segmentIndex, segmentsWidth, segmentsWidth);

			glTexSubImage3D(
				GL_TEXTURE_3D,
				0,
				segmentOffset.z,
				segmentOffset.y,
				segmentOffset.x,
				textureSegmentSize,
				textureSegmentSize,
				textureSegmentSize,
				GL_RED_INTEGER,
				GL_UNSIGNED_SHORT,
				cells + a3to1ind(segmentOffset, fluidSimWidth, fluidSimWidth)
			);
		}
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
	}
}

void FluidSim::queueTextureUpload(glm::ivec3 fluidIndex) 
{
	const unsigned int segmentsWidth = fluidSimWidth / textureSegmentSize;
	const glm::ivec3 segmentIndex = fluidIndex / glm::ivec3(textureSegmentSize);
	const unsigned int segmentFlatIndex = a3to1ind(segmentIndex, segmentsWidth, segmentsWidth);
	const unsigned int segmentMaskIndex = segmentFlatIndex / 32u;
	const unsigned int segmentBitIndex = segmentFlatIndex % 32u;
	if(textureSegmentUploadMask[segmentMaskIndex] & (1u << segmentBitIndex)) return;
	textureSegmentUploadMask[segmentMaskIndex] |= 1u << segmentBitIndex;
	textureUploadQueue.emplace(segmentFlatIndex);
}
unsigned int FluidSim::dequeueTextureUpload() 
{
	unsigned int index = textureUploadQueue.front();
	textureUploadQueue.pop();
	return index;
}

unsigned short compressedFluidDataBuffer[fluidSimWidth * fluidSimWidth * fluidSimWidth];
//unsigned long compressedDataBitmask[fluidSimWidth * fluidSimWidth * fluidSimWidth / 64 + 1];

void FluidSim::clear() 
{
	memset(cells, 0, sizeof(cells));
	totalFluid = 0;
	while(!updateQueue.empty()) updateQueue.pop();
}

const unsigned short* FluidSim::compressedData(int& size) 
{
	wasCompressed = true;
	size = 0;
	const unsigned char fluidType = 1;
	for(unsigned int x = 0; x < fluidSimWidth; x++)
	for(unsigned int z = 0; z < fluidSimWidth; z++) 
	{
		unsigned short fluidColumnValue = 0;
		unsigned short fluidColumnIndex = 0;
		bool columnStarted = false;
		for(unsigned int y = 0; y < fluidSimWidth; y++) 
		{
			unsigned int flatIndex = a3to1ind(x, y, z, fluidSimWidth, fluidSimWidth);
			const FluidCell& cell = cells[flatIndex];
			if(columnStarted) 
			{
				if(cell.type == fluidType) 
				{
					fluidColumnValue += cell.value;
				}
				else if(cell.type != fluidSimAirType) 
				{
					if(fluidColumnValue) 
					{
						compressedFluidDataBuffer[size] = fluidColumnIndex;
						compressedFluidDataBuffer[size + 1] = fluidColumnValue;
						fluidColumnValue = 0;
						size += 2;
					}
					columnStarted = false;
				}
			}
			else 
			{
				if(cell.type == fluidSimAirType || cell.type == fluidType) 
				{
					fluidColumnIndex = a3to1ind(x, y, z, fluidSimWidth, fluidSimWidth);
					if(cell.type == fluidType) fluidColumnValue = cell.value;
					columnStarted = true;
				}
			}
		}

		if(fluidColumnValue) 
		{
			compressedFluidDataBuffer[size] = fluidColumnIndex;
			compressedFluidDataBuffer[size + 1] = fluidColumnValue;
			size += 2;
		}
	}
	if(size == 0) throw std::logic_error("Should not try to compress an empty chunk");
	return compressedFluidDataBuffer;
}

void FluidSim::decodeCompressedData(const unsigned short* data, int size) 
{
	wasDecompressed = true;
	totalFluid = 0;
	const unsigned char fluidType = 1;
	const int entries = size / 2;
	for(int i = 0; i < entries; i++)
	{
		unsigned short fluidColumnAmount = data[i * 2 + 1];
		unsigned short fluidColumnFlatIndex = data[i * 2];
		glm::ivec3 index = a1to3ind(fluidColumnFlatIndex, fluidSimWidth, fluidSimWidth);
		if(fluidColumnAmount == 0) continue;
		while(fluidColumnAmount) 
		{
			const unsigned int flatIndex = a3to1ind(index, fluidSimWidth, fluidSimWidth);
			const unsigned short delta = fluidColumnAmount < fluidSimMaxValue ? fluidColumnAmount : fluidSimMaxValue;
			cells[flatIndex].type = fluidType;
			cells[flatIndex].value = delta;
			fluidColumnAmount -= delta;
			index.y += 1;
			totalFluid += delta;
		}
	}
}

void FluidSim::stageUpdates()
{
	if(parallelUpdateQueue.size())
	{
		memset(updateMaskParallel, 0, sizeof(updateMaskParallel));
	}
	while(parallelUpdateQueue.size()) 
	{
		queueUpdateFlow(a1to3ind(parallelUpdateQueue.back(), fluidSimWidth, fluidSimWidth));
		parallelUpdateQueue.pop();
	}
}

/*
const unsigned int* FluidSim::compressedData(int& size) 
{
	const unsigned char fluidType = 1;
	memset(compressedDataBitmask, 0, sizeof(compressedDataBitmask));
	for(unsigned int x = 0; x < fluidSimWidth; x++)
	for(unsigned int y = 0; y < fluidSimWidth; y++) 
	for(unsigned int z = 0; z < fluidSimWidth; z++) 
	{
		const unsigned int flatIndex = a3to1ind(x, y, z, fluidSimWidth, fluidSimWidth);
		//skip visited
		if(compressedDataBitmask[flatIndex / 64ul] & (1ul << (flatIndex % 64ul))) continue;
		std::vector<glm::ivec3> fillStack;
		fillStack.push_back(glm::ivec3(x,y,z));

		unsigned int fluidAmount = 0;
		glm::ivec3 lowestPoint = glm::ivec3(fluidSimWidth + 8);
		
		while(fillStack.size()) 
		{
			glm::ivec3 index = fillStack.back();
			fillStack.pop_back();
			bool leftPushed = false;
			bool rightPushed = false;
			bool frontPushed = false;
			bool backPushed = false;
			//find bottom
			while(index.y > 0) 
			{
				FluidCell& checkCell = cells[a3to1ind(index - glm::ivec3(0, 1, 0), fluidSimWidth, fluidSimWidth)];
				if(checkCell.type == fluidSimAirType || checkCell.type == fluidType) 
				{
					index.y--;
					if(lowestPoint.y > index.y) 
					{
						lowestPoint = index;
					}
				}
				else 
				{
					break;
				}
			}
			while(index.y < fluidSimWidth) 
			{
				if(index.x > 0) 
				{
					const glm::ivec3 checkIndex = index - glm::ivec3(1, 0, 0);
					const unsigned int flatCheckIndex = a3to1ind(checkIndex, fluidSimWidth, fluidSimWidth);
					const FluidCell& checkCell = cells[a3to1ind(checkIndex, fluidSimWidth, fluidSimWidth)];
					const bool isExpandable = 
					(checkCell.type == fluidSimAirType || checkCell.type == fluidType) && 
					((compressedDataBitmask[flatCheckIndex / 64ul] & (1ul << (flatCheckIndex % 64ul))) == 0);
					if(leftPushed) 
					{
						leftPushed = isExpandable;
					}
					else if(isExpandable)
					{
						leftPushed = true;
						fillStack.push_back(checkIndex);
					}
				}

				if(index.x < fluidSimWidth - 1) 
				{
					const glm::ivec3 checkIndex = index + glm::ivec3(1, 0, 0);
					const unsigned int flatCheckIndex = a3to1ind(checkIndex, fluidSimWidth, fluidSimWidth);
					const FluidCell& checkCell = cells[flatCheckIndex];
					const bool isExpandable = 
					(checkCell.type == fluidSimAirType || checkCell.type == fluidType) && 
					((compressedDataBitmask[flatCheckIndex / 64ul] & (1ul << (flatCheckIndex % 64ul))) == 0);
					
					if(rightPushed) 
					{
						rightPushed = isExpandable;
					}
					else if(isExpandable)
					{
						rightPushed = true;
						fillStack.push_back(checkIndex);
					}
				}

				if(index.z > 0) 
				{
					const glm::ivec3 checkIndex = index - glm::ivec3(0, 0, 1);
					const unsigned int flatCheckIndex = a3to1ind(checkIndex, fluidSimWidth, fluidSimWidth);
					const FluidCell& checkCell = cells[a3to1ind(checkIndex, fluidSimWidth, fluidSimWidth)];
					const bool isExpandable = 
					(checkCell.type == fluidSimAirType || checkCell.type == fluidType) && 
					((compressedDataBitmask[flatCheckIndex / 64ul] & (1ul << (flatCheckIndex % 64ul))) == 0);
					if(backPushed) 
					{
						backPushed = isExpandable;
					}
					else if(isExpandable)
					{
						backPushed = true;
						fillStack.push_back(checkIndex);
					}
				}

				if(index.z < fluidSimWidth - 1) 
				{
					const glm::ivec3 checkIndex = index + glm::ivec3(0, 0, 1);
					const unsigned int flatCheckIndex = a3to1ind(checkIndex, fluidSimWidth, fluidSimWidth);
					const FluidCell& checkCell = cells[a3to1ind(checkIndex, fluidSimWidth, fluidSimWidth)];
					const bool isExpandable = 
					(checkCell.type == fluidSimAirType || checkCell.type == fluidType) && 
					((compressedDataBitmask[flatCheckIndex / 64ul] & (1ul << (flatCheckIndex % 64ul))) == 0);
					if(frontPushed) 
					{
						frontPushed = isExpandable;
					}
					else if(isExpandable)
					{
						frontPushed = true;
						fillStack.push_back(checkIndex);
					}
				}
				const FluidCell& checkCell = cells[a3to1ind(index, fluidSimWidth, fluidSimWidth)];
				if(checkCell.type == fluidType) 
				{
					fluidAmount += checkCell.value;
				}
				else if(checkCell.type != fluidSimAirType) 
				{
					break;
				}
				const unsigned int cellFlatIndex = a3to1ind(index, fluidSimWidth, fluidSimWidth);
				compressedDataBitmask[cellFlatIndex / 64ul] |= (1ul << (cellFlatIndex % 64ul));
				index.y++;
			}
		}
		compressedFluidDataBuffer[size] = fluidAmount;
		compressedFluidDataBuffer[size + 1] = a3to1ind(lowestPoint, fluidSimWidth, fluidSimWidth);
		size += 2;
	}
}*/

/*
void FluidSim::decodeCompressedData(const unsigned int* data, int size) 
{
	const int entries = size / 2;
	for(int i = 0; i < entries; i++) 
	{
		const unsigned int fluidAmount = data[i * 2];
		const unsigned int fluidLowestFlatIndex = data[i * 2 + 1];
		if(fluidAmount == 0) continue;

	}
}*/