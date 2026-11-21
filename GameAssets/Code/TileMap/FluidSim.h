#pragma once
#include <queue>
#include "libs/glm/glm.hpp"
#include "Kirino/Material.h"
#include "Kirino/VertexArrayObject.h"
#include <mutex>
#include "Graphics/FrameBuffer/FrameBuffer.h"
#include "Graphics/Texture.h"
#include "Kirino/StateColletion.h"
#include "GameNode/Component.h"
using namespace TealEngine;
class Chunk;
const unsigned int fluidSimWidth = 32;
const unsigned char fluidSimMaxValue = 255;
const unsigned char fluidSimSolidType = 255;
const unsigned char fluidSimAirType = 0;
const unsigned int textureSegmentSize = 4;

#include <compare>
#include <limits>
#include <stdexcept>

class SafeUInt {
public:
    using value_type = unsigned int;

    constexpr SafeUInt() noexcept : value_(0) {}
    constexpr SafeUInt(value_type v) noexcept : value_(v) {}

    constexpr operator value_type() const noexcept {
        return value_;
    }

    // Prefix ++
    SafeUInt& operator++() {
        if (value_ == std::numeric_limits<value_type>::max())
            throw std::overflow_error("SafeUInt overflow");
        ++value_;
        return *this;
    }

    // Postfix ++
    SafeUInt operator++(int) {
        SafeUInt tmp = *this;
        ++(*this);
        return tmp;
    }

    // Prefix --
    SafeUInt& operator--() {
        if (value_ == 0)
            throw std::underflow_error("SafeUInt underflow");
        --value_;
        return *this;
    }

    // Postfix --
    SafeUInt operator--(int) {
        SafeUInt tmp = *this;
        --(*this);
        return tmp;
    }

    // +=
    SafeUInt& operator+=(const SafeUInt& rhs) {
#if defined(__GNUC__) || defined(__clang__)
        value_type result;
        if (__builtin_add_overflow(value_, rhs.value_, &result))
            throw std::overflow_error("SafeUInt overflow");
        value_ = result;
#else
        if (value_ > std::numeric_limits<value_type>::max() - rhs.value_)
            throw std::overflow_error("SafeUInt overflow");
        value_ += rhs.value_;
#endif
        return *this;
    }

    // -=
    SafeUInt& operator-=(const SafeUInt& rhs) {
        if (value_ < rhs.value_)
            throw std::underflow_error("SafeUInt underflow");
        value_ -= rhs.value_;
        return *this;
    }

    // *=
    SafeUInt& operator*=(const SafeUInt& rhs) {
#if defined(__GNUC__) || defined(__clang__)
        value_type result;
        if (__builtin_mul_overflow(value_, rhs.value_, &result))
            throw std::overflow_error("SafeUInt overflow");
        value_ = result;
#else
        if (rhs.value_ != 0 &&
            value_ > std::numeric_limits<value_type>::max() / rhs.value_)
            throw std::overflow_error("SafeUInt overflow");
        value_ *= rhs.value_;
#endif
        return *this;
    }

    // /=
    SafeUInt& operator/=(const SafeUInt& rhs) {
        if (rhs.value_ == 0)
            throw std::domain_error("division by zero");
        value_ /= rhs.value_;
        return *this;
    }

    // %=
    SafeUInt& operator%=(const SafeUInt& rhs) {
        if (rhs.value_ == 0)
            throw std::domain_error("modulo by zero");
        value_ %= rhs.value_;
        return *this;
    }

    friend SafeUInt operator+(SafeUInt lhs, const SafeUInt& rhs) {
        lhs += rhs;
        return lhs;
    }

    friend SafeUInt operator-(SafeUInt lhs, const SafeUInt& rhs) {
        lhs -= rhs;
        return lhs;
    }

    friend SafeUInt operator*(SafeUInt lhs, const SafeUInt& rhs) {
        lhs *= rhs;
        return lhs;
    }

    friend SafeUInt operator/(SafeUInt lhs, const SafeUInt& rhs) {
        lhs /= rhs;
        return lhs;
    }

    friend SafeUInt operator%(SafeUInt lhs, const SafeUInt& rhs) {
        lhs %= rhs;
        return lhs;
    }

    // Comparisons
    constexpr auto operator<=>(const SafeUInt&) const = default;

private:
    value_type value_;
};

class FluidSim 
{
	public:
	#pragma pack(push, 1)
	struct FluidCell 
	{
		unsigned char value;
		unsigned char type;
	};
	#pragma pack(pop)
	protected:
	bool wasCompressed, wasDecompressed;
	SafeUInt totalFluid;
	FluidCell cells[fluidSimWidth*fluidSimWidth*fluidSimWidth];
    unsigned int networkLinesUpdateMask[32];
	unsigned int updateMask[fluidSimWidth*fluidSimWidth];
	unsigned int updateMaskParallel[fluidSimWidth*fluidSimWidth];
	unsigned int textureSegmentUploadMask[fluidSimWidth*fluidSimWidth*fluidSimWidth / 32 / textureSegmentSize/textureSegmentSize/textureSegmentSize + 1];
	std::queue<unsigned short> textureUploadQueue;
	std::queue<unsigned short> updateQueue;
    std::queue<unsigned short> parallelUpdateQueue;
	bool textureDirty;
	private:
	unsigned int random;
	void setWaterLevelAt(glm::ivec3 index, unsigned char value);
	public:
    void clear();
	void queueTextureUpload(glm::ivec3 fluidIndex);
	unsigned int dequeueTextureUpload();
	unsigned int fluidSampler;
	Chunk* parentChunk = nullptr;
	void update(int updateCount);
	void updateFlow(glm::ivec3 index);
	void queueUpdateFlow(glm::ivec3 index);
	void queueUpdateFlowParallel(glm::ivec3 index);
	FluidCell& getCell(glm::ivec3 index);
	FluidCell& getCell(glm::ivec3 index, FluidSim*& simPtr);
	FluidSim* getFluidSimAt(glm::ivec3 index);
	FluidSim* getFluidSimAtMutIndex(glm::ivec3& index);
	void setCell(glm::ivec3 index, unsigned char value, unsigned char type, bool updateFlag = true);
	static FluidCell outOfBoundsCell;
	inline unsigned int getTotalFluid() { return totalFluid; }
	FluidSim();
	void render(glm::vec3 offset, Kirino::StateColletion& stateCollection, const FrustumPlanes& frustum, const RenderTexturesCollection& renderTextures);
	void uploadTexture();
	inline bool isTextureDirty() { return textureDirty; }
	inline void setTextureDirty() { this->textureDirty = true; }
	const unsigned short* compressedData(int& size);
	void decodeCompressedData(const unsigned short* data, int size);
    //returned buffer is valid untill the next call
    const char* getNetworkSyncBuffer(unsigned int& sizeOut);
    void markLineForSync(unsigned int x, unsigned int y);
    void stageUpdates();
	friend class ChunkCompressor;
};