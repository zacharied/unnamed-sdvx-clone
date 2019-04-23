#pragma once

#include <Graphics/MaterialParams.hpp>
#include <Graphics/IShader.hpp>
#include <Graphics/RenderState.hpp>

namespace Graphics
{/*
		Abstracts the use of shaders/uniforms/pipelines into a single interface class
	*/
	class IMaterial
	{
	public:
		virtual ~IMaterial() = default;

		bool opaque = true;
		MaterialBlendMode blendMode = MaterialBlendMode::Normal;

		virtual void AssignShader(ShaderType t, IShader shader) = 0;

		virtual void Bind(const RenderState& rs, const MaterialParameterSet& params) = 0;

		// Only binds parameters to the current shader
		virtual void BindParameters(const MaterialParameterSet& params, const Transform& worldTransform) = 0;

		// Bind only shaders/pipeline to context
		virtual void BindToContext() = 0;
	};
}