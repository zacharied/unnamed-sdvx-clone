#pragma once

#include <Shared/VectorMath.hpp>
#include <Graphics/ITexture.hpp>
#include <Graphics/IMesh.hpp>

namespace Graphics
{
	/*
		A prerendered text object, contains all the vertices and texture sheets to draw itself
	*/
	class IText
	{
	public:
		virtual ~IText() = default;

		virtual ITexture* GetTexture() = 0;
		virtual IMesh* GetMesh() = 0;
		virtual void Draw() = 0;
		virtual Vector2 GetSize() = 0;
	};
}