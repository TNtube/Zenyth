#pragma once

enum class ShaderPass
{
	Normal,
	Transparent,
	Water,
	Size
};

#define BLOCKS(F) \
F( EMPTY,				-1 ) \
F( STONE,				1 ) \
F( DIRT,				2 ) \
F( GRASS,				3, 0, 2 ) \
F( WOOD,				4 ) \
F( SLAB,				5, 6, 6 ) \
F( BRICK,				7 ) \
F( STONE_BRICK,			54 ) \
F( BOOKSHELF,			35, 4, 4 ) \
F( DUNGEON_STONE,		36 ) \
F( TNT,					8, 9, 10 ) \
F( COBBLESTONE,			16 ) \
F( BEDROCK,				17 ) \
F( SAND,				18 ) \
F( GRAVEL,				19 ) \
F( LOG,					20, 21, 21 ) \
F( SPONGE,				48 ) \
F( WOOL,				64 ) \
/* ORE */ \
F( COAL,				34 ) \
F( IRON_ORE,			33 ) \
F( IRON_BLOCK,			22 ) \
F( GOLD_ORE,			32 ) \
F( GOLD_BLOCK,			23 ) \
F( DIAMOND_ORE,			50 ) \
F( DIAMOND_BLOCK,		24 ) \
F( EMERALD_BLOCK,		25 ) \
F( REDSTONE_ORE,		51 ) \
F( OBSIDIAN,			37 ) \
\
/* OBJECTS */ \
F( CRAFTING_TABLE,		59, 43, 4 ) /* there is a side variation at index 60 */ \
F( FURNACE,				44, 62, 62 ) /* need an orientation & on/off flag */ \
F( DISPENSER,			46, 62, 62 ) /* need an orientation flag */ \
/* TRANSPARENT STUFF */ \
F( GLASS,				49, ShaderPass::Transparent ) \
F( WATER,				205, ShaderPass::Water ) \
/* 38, 39 & 40 contains greyscale grass for biome variation */
/* as an exercice you can try to implement that by adding back some vertex color informations to the pipeline */
/* 52, 53 contains greyscale leaves */

#define EXTRACT_BLOCK_ID( v ) v,
enum BlockId: uint8_t {
	BLOCKS(EXTRACT_BLOCK_ID)
};

class BlockData {
public:
	BlockId id;

	int texIdSide;
	int texIdTop;
	int texIdBottom;

	ShaderPass pass;
public:
	BlockData(const BlockId id, const int texId, const ShaderPass pass = ShaderPass::Normal) :
		id(id),
		texIdSide(texId),
		texIdTop(texId),
		texIdBottom(texId),
		pass(pass) {}

	BlockData(const BlockId id, const int texIdSide, const int texIdTop, const int texIdBottom, const ShaderPass pass = ShaderPass::Normal) :
		id(id),
		texIdSide(texIdSide),
		texIdTop(texIdTop),
		texIdBottom(texIdBottom),
		pass(pass) {}

	static const BlockData& Get(BlockId id);
};