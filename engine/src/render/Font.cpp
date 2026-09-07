#include "engine/render/Font.h"
#include "engine/core/Log.h"
#include "engine/core/Utf8Path.h"

#include <stb_truetype.h>

#include <cstddef>
#include <fstream>
#include <vector>

namespace Engine {

namespace {

constexpr int atlasSize = 512;
constexpr int firstChar = 32;
constexpr int numChars = 96; // printable ASCII: 32..127

std::vector<unsigned char> ReadFile(const std::string& path){
	std::ifstream file(PathFromUtf8(path), std::ios::binary | std::ios::ate);
	if(!file){
		return {};
	}

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<unsigned char> buffer(static_cast<std::size_t>(size));
	file.read(reinterpret_cast<char*>(buffer.data()), size);
	return buffer;
}

}

Font::Font(const std::string& ttfPath, float pixelHeight){
	std::vector<unsigned char> fontData = ReadFile(ttfPath);
	if(fontData.empty()){
		ENGINE_CORE_ERROR("Failed to read font file: {}", ttfPath);
		// atlas must stay non-null - GetAtlas() has no failure mode of its own
		unsigned char blankPixel = 0;
		atlas = std::make_unique<Texture>(&blankPixel, 1, 1, 1);
		return;
	}

	std::vector<unsigned char> bitmap(atlasSize * atlasSize, 0);
	std::vector<stbtt_bakedchar> bakedChars(numChars);

	int result = stbtt_BakeFontBitmap(fontData.data(), 0, pixelHeight, bitmap.data(), atlasSize, atlasSize, firstChar, numChars, bakedChars.data());
	if(result <= 0){
		ENGINE_CORE_ERROR("Font atlas too small to bake: {}", ttfPath);
	}

	atlas = std::make_unique<Texture>(bitmap.data(), atlasSize, atlasSize, 1);

	for(int i = 0; i < numChars; i++){
		const stbtt_bakedchar& bakedChar = bakedChars[i];

		Glyph glyph;
		glyph.uvRect = glm::vec4(
			static_cast<float>(bakedChar.x0) / atlasSize,
			static_cast<float>(bakedChar.y0) / atlasSize,
			static_cast<float>(bakedChar.x1) / atlasSize,
			static_cast<float>(bakedChar.y1) / atlasSize
		);
		glyph.size = glm::vec2(bakedChar.x1 - bakedChar.x0, bakedChar.y1 - bakedChar.y0);
		glyph.offset = glm::vec2(bakedChar.xoff, bakedChar.yoff);
		glyph.advance = bakedChar.xadvance;

		glyphs[static_cast<char>(firstChar + i)] = glyph;
	}

	stbtt_fontinfo fontInfo;
	stbtt_InitFont(&fontInfo, fontData.data(), 0);
	int ascent = 0;
	int descent = 0;
	int lineGap = 0;
	stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
	float scale = stbtt_ScaleForPixelHeight(&fontInfo, pixelHeight);
	lineHeight = static_cast<float>(ascent - descent + lineGap) * scale;

	ENGINE_CORE_INFO("Loaded font: {} ({}px)", ttfPath, pixelHeight);
}

const Glyph& Font::GetGlyph(char c) const{
	auto it = glyphs.find(c);
	if(it != glyphs.end()){
		return it->second;
	}

	static Glyph fallback;
	return fallback;
}

}
