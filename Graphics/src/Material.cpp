#include "stdafx.h"
#include "Material.hpp"
#include "OpenGL.hpp"
#include "RenderQueue.hpp"

namespace Graphics
{
	const char* builtInShaderVariableNames[] =
	{
		"world",
		"proj",
		"camera",
		"billboard",
		"viewport",
		"aspectRatio",
		"time",
	};
	class BuiltInShaderVariableMap : public Map<String, BuiltInShaderVariable>
	{
	public:
		BuiltInShaderVariableMap()
		{
			for(int32 i = 0; i < SV__BuiltInEnd; i++)
			{
				Add(builtInShaderVariableNames[i], (BuiltInShaderVariable)i);
			}
		}
	};
	BuiltInShaderVariableMap builtInShaderVariableMap;

	// Defined in Shader.cpp
	extern uint32 shaderStageMap[];

	Material::Material()
	{
		glGenProgramPipelines(1, &m_pipeline);
	}

	Material::~Material()
	{
		glDeleteProgramPipelines(1, &m_pipeline);
	}

	void Material::AssignShader(unique_ptr<IShader> shader)
	{
		auto handle = shader->Handle();
		auto shader_type = shader->GetType();
		auto original_name = shader->GetOriginalName();

		m_shaders[(size_t)shader_type] = std::move(shader);

#ifdef _DEBUG
		Logf("Listing shader uniforms for %s", Logger::Info, original_name);
#endif // _DEBUG

		int32 numUniforms;
		glGetProgramiv(handle, GL_ACTIVE_UNIFORMS, &numUniforms);
		for(int32 i = 0; i < numUniforms; i++)
		{
			char name[64];
			int32 nameLen, size;
			uint32 type;
			glGetActiveUniform(handle, i, sizeof(name), &nameLen, &size, &type, name);
			uint32 loc = glGetUniformLocation(handle, name);

			// Select type
			uint32 textureID = 0;
			String typeName = "Unknown";
			if(type == GL_SAMPLER_2D)
			{
				typeName = "Sampler2D";
				if(!m_textureIDs.Contains(name))
					m_textureIDs.Add(name, m_textureID++);
			}
			else if(type == GL_FLOAT_MAT4)
			{
				typeName = "Transform";
			}
			else if(type == GL_FLOAT_VEC4)
			{
				typeName = "Vector4";
			}
			else if(type == GL_FLOAT_VEC3)
			{
				typeName = "Vector3";
			}
			else if(type == GL_FLOAT_VEC2)
			{
				typeName = "Vector2";
			}
			else if(type == GL_FLOAT)
			{
				typeName = "Float";
			}

			// Built in variable?
			uint32 targetID = 0;
			if(builtInShaderVariableMap.Contains(name))
			{
				targetID = builtInShaderVariableMap[name];
			}
			else
			{
				if(m_mappedParameters.Contains(name))
					targetID = m_mappedParameters[name];
				else
					targetID = m_mappedParameters.Add(name, m_userID++);
			}

			BoundParameterInfo& param = m_boundParameters.FindOrAdd(targetID).Add(BoundParameterInfo(shader_type, type, loc));

#ifdef _DEBUG
			Logf("Uniform [%d, loc=%d, %s] = %s", Logger::Info,
					i, loc, Utility::Sprintf("Unknown [%d]", type), name);
#endif // _DEBUG
		}

		glUseProgramStages(m_pipeline, shaderStageMap[(size_t)shader_type], handle);
	}

	// Bind render state and params and shaders to context
	void Material::Bind(const RenderState& rs, const MaterialParameterSet& params)
	{
#if _DEBUG
		bool reloadedShaders = false;
			for(uint32 i = 0; i < 3; i++)
			{
				if(m_shaders[i] && m_shaders[i]->UpdateHotReload())
				{
					reloadedShaders = true;
				}
			}

			// Regenerate parameter map
			if(reloadedShaders)
			{
				Log("Reloading material", Logger::Info);
				m_boundParameters.clear();
				m_textureIDs.clear();
				m_mappedParameters.clear();
				m_userID = SV_User;
				m_textureID = 0;
				for(uint32 i = 0; i < 3; i++)
				{
					if(m_shaders[i])
						AssignShader(std::move(m_shaders[i]));
				}
			}
#endif

		// Bind renderstate variables
		BindAll(SV_Proj, rs.projectionTransform);
		BindAll(SV_Camera, rs.cameraTransform);
		BindAll(SV_Viewport, rs.viewportSize);
		BindAll(SV_AspectRatio, rs.aspectRatio);
		Transform billboard = CameraMatrix::BillboardMatrix(rs.cameraTransform);
		BindAll(SV_BillboardMatrix, billboard);
		BindAll(SV_Time, rs.time);

		// Bind parameters
		BindParameters(params, rs.worldTransform);

		BindToContext();
	}

	void Material::BindParameters(const MaterialParameterSet& params, const Transform& worldTransform)
	{
		BindAll(SV_World, worldTransform);
		for(auto p : params)
		{
			switch(p.second.parameterType)
			{
			case GL_INT:
				BindAll(p.first, p.second.Get<int>());
				break;
			case GL_FLOAT:
				BindAll(p.first, p.second.Get<float>());
				break;
			case GL_INT_VEC2:
				BindAll(p.first, p.second.Get<Vector2i>());
				break;
			case GL_INT_VEC3:
				BindAll(p.first, p.second.Get<Vector3i>());
				break;
			case GL_INT_VEC4:
				BindAll(p.first, p.second.Get<Vector4i>());
				break;
			case GL_FLOAT_VEC2:
				BindAll(p.first, p.second.Get<Vector2>());
				break;
			case GL_FLOAT_VEC3:
				BindAll(p.first, p.second.Get<Vector3>());
				break;
			case GL_FLOAT_VEC4:
				BindAll(p.first, p.second.Get<Vector4>());
				break;
			case GL_FLOAT_MAT4:
				BindAll(p.first, p.second.Get<Transform>());
				break;
			case GL_SAMPLER_2D:
			{
				uint32* textureUnit = m_textureIDs.Find(p.first);
				if(!textureUnit)
				{
					/// TODO: Add print once mechanism for these kind of errors
					//Logf("Texture not found \"%s\"", Logger::Warning, p.first);
					break;
				}
				auto tex = p.second.Get<ITexture*>();

				// Bind the texture
				tex->Bind(*textureUnit);

				// Bind sampler
				BindAll<int32>(p.first, *textureUnit);
				break;
			}
			default:
				assert(false);
			}
		}
	}

	void Material::BindToContext()
	{
		// Bind pipeline to context
		glBindProgramPipeline(m_pipeline);
	}

	BoundParameterInfo* Material::GetBoundParameters(const String& name, uint32& count)
	{
		uint32* mappedID = m_mappedParameters.Find(name);
		if(!mappedID)
			return nullptr;
		return GetBoundParameters((BuiltInShaderVariable)*mappedID, count);
	}

	BoundParameterInfo* Material::GetBoundParameters(BuiltInShaderVariable bsv, uint32& count)
	{
		BoundParameterList* l = m_boundParameters.Find(bsv);
		if(!l)
			return nullptr;
		else
		{
			count = (uint32)l->size();
			return l->data();
		}
	}

	auto Material::Create() -> optional<unique_ptr<Material>>
	{
		struct EnableMaker : public Material { using Material::Material; };
		return make_unique<EnableMaker>();
	}

	auto Material::Create(const String& vsPath, const String& fsPath) -> optional<unique_ptr<Material>>
	{
		struct EnableMaker : public Material { using Material::Material; };
		auto mat = make_unique<EnableMaker>();
		auto vShader = Shader::Create(ShaderType::Vertex, vsPath);
		assert(vShader); // TODO: (factory)
		auto fShader = Shader::Create(ShaderType::Fragment, fsPath);
		assert(fShader); // TODO: (factory)
		mat->AssignShader(std::move(*vShader));
		mat->AssignShader(std::move(*fShader));
#if _DEBUG
		mat->m_debugNames[(size_t)ShaderType::Vertex] = vsPath;
		mat->m_debugNames[(size_t)ShaderType::Fragment] = fsPath;
#endif

		if(!mat->m_shaders[(size_t)ShaderType::Vertex])
		{
			Logf("Failed to load vertex shader for material from %s", Logger::Error, vsPath);
			return {};
		}

		if(!mat->m_shaders[(size_t)ShaderType::Fragment])
		{
			Logf("Failed to load fragment shader for material from %s", Logger::Error, fsPath);
			return {};
		}

		return std::move(mat);
	}
}
