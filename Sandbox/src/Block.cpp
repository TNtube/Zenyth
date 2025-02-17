#include "Block.hpp"

#define CREATE_BLOCK_DATA( ... ) BlockData(__VA_ARGS__),
const BlockData blocksData[] = {
	BLOCKS(CREATE_BLOCK_DATA)
};

const BlockData& BlockData::Get(const BlockId id) {
	return blocksData[id];
}