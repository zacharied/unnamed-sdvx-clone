#pragma once
#include "IMesh.hpp"

namespace Graphics
{
	class Mesh : public IMesh
	{
	public:
		~Mesh() override;
		static auto Create() -> optional<unique_ptr<Mesh>>;

		// Sets how the point data is interpreted and drawn
		// must be set before drawing
		void SetPrimitiveType(PrimitiveType pt) override;
		PrimitiveType GetPrimitiveType() const override;
		// Draws the mesh
		void Draw() override;
		// Draws the mesh after if has already been drawn once, reuse of bound objects
		void Redraw() override;

	private:
		uint32 m_buffer = 0;
		uint32 m_vao = 0;
		PrimitiveType m_type;
		uint32 m_glType;
		size_t m_vertexCount;
		bool m_bDynamic = true;

		Mesh() = default;
		bool Init();
		void SetData(const void* pData, size_t vertexCount, const VertexFormatList& desc) override;
	};
}