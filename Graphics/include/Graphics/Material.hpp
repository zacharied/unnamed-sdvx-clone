#pragma once
#include <Graphics/IMaterial.hpp>
#include <Graphics/ResourceTypes.hpp>
#include <Graphics/Shader.hpp>
#include <Graphics/RenderState.hpp>

namespace Graphics
{
	// Defines build in shader variables
	enum BuiltInShaderVariable
	{
		SV_World = 0,
		SV_Proj,
		SV_Camera,
		SV_BillboardMatrix,
		SV_Viewport,
		SV_AspectRatio,
		SV_Time,
		SV__BuiltInEnd,
		SV_User = 0x100, // Start defining user variables here
	};

	struct BoundParameterInfo
	{
		BoundParameterInfo(ShaderType shaderType, uint32 paramType, uint32 location)
				:shaderType(shaderType), paramType(paramType), location(location)
		{}

		ShaderType shaderType;
		uint32 location;
		uint32 paramType;
	};

	struct BoundParameterList : public Vector<BoundParameterInfo>
	{};

	/*
		Abstracts the use of shaders/uniforms/pipelines into a single interface class
	*/
	class Material : public IMaterial
	{
	public:
		~Material() override;
		// Create a default material
		static auto Create() -> optional<unique_ptr<Material>>;
		// Create a material that has both a vertex and fragment shader
		static auto Create(const String& vsPath, const String& fsPath) -> optional<unique_ptr<Material>>;

		bool opaque = true;
		MaterialBlendMode blendMode = MaterialBlendMode::Normal;

		void AssignShader(ShaderType t, IShader shader) override;
		void Bind(const RenderState& rs, const MaterialParameterSet& params) override;

		// Only binds parameters to the current shader
		void BindParameters(const MaterialParameterSet& params, const Transform& worldTransform) override;

		// Bind only shaders/pipeline to context
		void BindToContext() override;

	private:
		array<IShader, 3> m_shaders;
#if _DEBUG
		String m_debugNames[3];
#endif
		uint32 m_pipeline;
		Map<uint32, BoundParameterList> m_boundParameters;
		Map<String, uint32> m_mappedParameters;
		Map<String, uint32> m_textureIDs;
		uint32 m_userID = SV_User;
		uint32 m_textureID = 0;

		Material();
		BoundParameterInfo* GetBoundParameters(const String& name, uint32& count);
		BoundParameterInfo* GetBoundParameters(BuiltInShaderVariable bsv, uint32& count);
		template<typename T> void BindAll(const String& name, const T& obj);
		template<typename T> void BindAll(BuiltInShaderVariable bsv, const T& obj);
		template<typename T> void BindShaderVar(uint32 shader, uint32 loc, const T& obj);
	};

	template<typename T>
	void Material::BindAll(const String& name, const T& obj)
	{
		uint32 num = 0;
		BoundParameterInfo* bp = GetBoundParameters(name, num);
		for(uint32 i = 0; bp && i < num; i++)
			BindShaderVar<T>(m_shaders[(size_t)bp[i].shaderType]->Handle(), bp[i].location, obj);
	}

	template<typename T>
	void Material::BindAll(BuiltInShaderVariable bsv, const T& obj)
	{
		uint32 num = 0;
		BoundParameterInfo* bp = GetBoundParameters(bsv, num);
		for(uint32 i = 0; bp && i < num; i++)
			BindShaderVar<T>(m_shaders[(size_t)bp[i].shaderType]->Handle(), bp[i].location, obj);
	}

	template<typename T>
	void Material::BindShaderVar(uint32 shader, uint32 loc, const T& obj)
	{
		static_assert(sizeof(T) != 0, "Incompatible shader uniform type");
	}

	template<> void Material::BindShaderVar<Vector4>(uint32 shader, uint32 loc, const Vector4& obj)
	{
		glProgramUniform4fv(shader, loc, 1, &obj.x);
	}
	template<> void Material::BindShaderVar<Vector3>(uint32 shader, uint32 loc, const Vector3& obj)
	{
		glProgramUniform3fv(shader, loc, 1, &obj.x);
	}
	template<> void Material::BindShaderVar<Vector2>(uint32 shader, uint32 loc, const Vector2& obj)
	{
		glProgramUniform2fv(shader, loc, 1, &obj.x);
	}
	template<> void Material::BindShaderVar<float>(uint32 shader, uint32 loc, const float& obj)
	{
		glProgramUniform1fv(shader, loc, 1, &obj);
	}
	template<> void Material::BindShaderVar<Colori>(uint32 shader, uint32 loc, const Colori& obj)
	{
		Color c = obj;
		glProgramUniform4fv(shader, loc, 1, &c.x);
	}
	template<> void Material::BindShaderVar<Vector4i>(uint32 shader, uint32 loc, const Vector4i& obj)
	{
		glProgramUniform4iv(shader, loc, 1, &obj.x);
	}
	template<> void Material::BindShaderVar<Vector3i>(uint32 shader, uint32 loc, const Vector3i& obj)
	{
		glProgramUniform3iv(shader, loc, 1, &obj.x);
	}
	template<> void Material::BindShaderVar<Vector2i>(uint32 shader, uint32 loc, const Vector2i& obj)
	{
		glProgramUniform2iv(shader, loc, 1, &obj.x);
	}
	template<> void Material::BindShaderVar<int32>(uint32 shader, uint32 loc, const int32& obj)
	{
		glProgramUniform1iv(shader, loc, 1, &obj);
	}
	template<> void Material::BindShaderVar<Transform>(uint32 shader, uint32 loc, const Transform& obj)
	{
		glProgramUniformMatrix4fv(shader, loc, 1, GL_FALSE, obj.mat);
	}
}