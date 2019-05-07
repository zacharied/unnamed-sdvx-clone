#include "stdafx.h"
#include "Mesh.hpp"

namespace Graphics
{
	uint32 primitiveTypeMap[] = {GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_LINES, GL_LINE_STRIP, GL_POINTS,};

	auto Mesh::Create() -> optional<unique_ptr<Mesh>>
	{
		struct EnableMaker : public Mesh { using Mesh::Mesh; };
		auto mesh = make_unique<EnableMaker>();
		if(!mesh->Init())
			return {};
		return std::move(mesh);
	}

	Mesh::~Mesh()
	{
		if (m_buffer)
			glDeleteBuffers(1, &m_buffer);
		if (m_vao)
			glDeleteVertexArrays(1, &m_vao);
	}

	void Mesh::SetPrimitiveType(PrimitiveType pt)
	{
		m_type = pt;
		m_glType = primitiveTypeMap[(size_t) pt];
	}

	PrimitiveType Mesh::GetPrimitiveType() const
	{
		return m_type;
	}

	void Mesh::Draw()
	{
		glBindVertexArray(m_vao);
		glDrawArrays(m_glType, 0, (int) m_vertexCount);
	}

	void Mesh::Redraw()
	{
		glDrawArrays(m_glType, 0, (int) m_vertexCount);
	}

	bool Mesh::Init()
	{
		glGenBuffers(1, &m_buffer);
		glGenVertexArrays(1, &m_vao);
		return m_buffer != 0 && m_vao != 0;
	}

	void Mesh::SetData(const void* pData, size_t vertexCount, const VertexFormatList& desc)
	{
		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer);

		m_vertexCount = vertexCount;
		size_t totalVertexSize = 0;
		for (auto e : desc)
		{
			totalVertexSize += e.componentSize * e.components;
		}
		size_t index = 0;
		size_t offset = 0;
		for (auto e : desc)
		{
			uint32 type = -1;
			if (!e.isFloat)
			{
				if (e.componentSize == 4)
					type = e.isSigned ? GL_INT : GL_UNSIGNED_INT;
				else if (e.componentSize == 2)
					type = e.isSigned ? GL_SHORT : GL_UNSIGNED_SHORT;
				else if (e.componentSize == 1)
					type = e.isSigned ? GL_BYTE : GL_UNSIGNED_BYTE;
			}
			else
			{
				if (e.componentSize == 4)
					type = GL_FLOAT;
				else if (e.componentSize == 8)
					type = GL_DOUBLE;
			}
			assert(type != -1);
			glVertexAttribPointer((int) index, (int) e.components, type, GL_TRUE, (int) totalVertexSize,
								  (void*) offset);
			glEnableVertexAttribArray((int) index);
			offset += e.componentSize * e.components;
			index++;
		}
		glBufferData(GL_ARRAY_BUFFER, totalVertexSize * vertexCount, pData,
					 m_bDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	unique_ptr<Mesh> Mesh::GenerateQuad(Vector2 pos, Vector2 size)
	{
		Vector<SimpleVertex> verts =
				{
						{ { 0.0f,  size.y, 0.0f }, { 0.0f, 0.0f } },
						{ { size.x, 0.0f,  0.0f }, { 1.0f, 1.0f } },
						{ { size.x, size.y, 0.0f }, { 1.0f, 0.0f } },

						{ { 0.0f,  size.y, 0.0f }, { 0.0f, 0.0f } },
						{ { 0.0f,  0.0f,  0.0f }, { 0.0f, 1.0f } },
						{ { size.x, 0.0f,  0.0f }, { 1.0f, 1.0f } },
				};

		for(auto& v : verts)
		{
			v.pos += pos;
		}

		auto mesh = Mesh::Create();
		assert(mesh); // TODO: factory
		((IMesh*)(*mesh).get())->SetData(verts);
		(*mesh)->SetPrimitiveType(PrimitiveType::TriangleList);
		return std::move(*mesh);
	}


	void IMesh::GenerateSimpleXYQuad(Rect3D r, Rect uv, Vector<SimpleVertex>& out)
	{
		Vector<SimpleVertex> verts = { {{r.Left(),  r.Top(),    0.0f}, {uv.Left(),  uv.Top()}},
									  {{r.Right(), r.Bottom(), 0.0f}, {uv.Right(), uv.Bottom()}},
									  {{r.Right(), r.Top(),    0.0f}, {uv.Right(), uv.Top()}},

									  {{r.Left(),  r.Top(),    0.0f}, {uv.Left(),  uv.Top()}},
									  {{r.Left(),  r.Bottom(), 0.0f}, {uv.Left(),  uv.Bottom()}},
									  {{r.Right(), r.Bottom(), 0.0f}, {uv.Right(), uv.Bottom()}}, };

		for (auto& v : verts)
		{
			out.Add(v);
		}
	}

	void IMesh::GenerateSimpleXZQuad(Rect3D r, Rect uv, Vector<SimpleVertex>& out)
	{
		Vector<SimpleVertex> verts = { {{r.Left(),  0.0f, r.Top(),},    {uv.Left(),  uv.Top()}},
									  {{r.Right(), 0.0f, r.Bottom(),}, {uv.Right(), uv.Bottom()}},
									  {{r.Right(), 0.0f, r.Top(),},    {uv.Right(), uv.Top()}},

									  {{r.Left(),  0.0f, r.Top(),},    {uv.Left(),  uv.Top()}},
									  {{r.Left(),  0.0f, r.Bottom(),}, {uv.Left(),  uv.Bottom()}},
									  {{r.Right(), 0.0f, r.Bottom(),}, {uv.Right(), uv.Bottom()}}, };

		for (auto& v : verts)
		{
			out.Add(v);
		}
	}
}

