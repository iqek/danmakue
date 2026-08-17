#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

#include "engine/render/Texture.h"

namespace Engine {

struct Glyph {
	glm::vec4 uvRect{ 0.0f };  // (uMin, vMin, uMax, vMax) within the atlas
	glm::vec2 size{ 0.0f };    // glyph quad size, in pixels
	glm::vec2 offset{ 0.0f };  // offset from the cursor to the glyph's top-left
	float advance = 0.0f;      // how far to move the cursor after drawing this glyph
};

// A bitmap font atlas baked from a .ttf via stb_truetype, ASCII only
class Font {
private:
	std::unique_ptr<Texture> atlas;
	std::unordered_map<char, Glyph> glyphs;
	float lineHeight = 0.0f;

public:
	Font(const std::string& ttfPath, float pixelHeight);

	const Texture& GetAtlas() const { return *atlas; }
	const Glyph& GetGlyph(char c) const;
	float GetLineHeight() const { return lineHeight; }
};

}
