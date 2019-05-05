#pragma once

#include <Shared/VectorMath.hpp>
#include <Graphics/ITexture.hpp>
#include <Graphics/Mesh.hpp>


namespace Graphics
{
	/*
		A prerendered text object, contains all the vertices and texture sheets to draw itself
	*/
	class IText
	{
	public:
		virtual ~IText() = default;

		virtual shared_ptr<ITexture> GetTexture() = 0;
		virtual shared_ptr<IMesh> GetMesh() = 0;
		virtual void Draw() = 0;
		virtual Vector2 GetSize() = 0;
	};
}