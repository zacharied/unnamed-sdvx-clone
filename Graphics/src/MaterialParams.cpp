#include "stdafx.h"
#include "MaterialParams.hpp"

namespace Graphics
{
	void MaterialParameterSet::SetParameter(const String& name, int sc)
	{
		Add(name, MaterialParameter::Create(sc, GL_INT));
	}

	void MaterialParameterSet::SetParameter(const String& name, float sc)
	{
		Add(name, MaterialParameter::Create(sc, GL_FLOAT));
	}

	void MaterialParameterSet::SetParameter(const String& name, const Vector4& vec)
	{
		Add(name, MaterialParameter::Create(vec, GL_FLOAT_VEC4));
	}

	void MaterialParameterSet::SetParameter(const String& name, const Colori& color)
	{
		Add(name, MaterialParameter::Create(Color(color), GL_FLOAT_VEC4));
	}

	void MaterialParameterSet::SetParameter(const String& name, const Vector2& vec2)
	{
		Add(name, MaterialParameter::Create(vec2, GL_FLOAT_VEC2));
	}

	void MaterialParameterSet::SetParameter(const String& name, const Vector3& vec3)
	{
		Add(name, MaterialParameter::Create(vec3, GL_FLOAT_VEC3));
	}

	void MaterialParameterSet::SetParameter(const String& name, const Transform& tf)
	{
		Add(name, MaterialParameter::Create(tf, GL_FLOAT_MAT4));
	}

	void MaterialParameterSet::SetParameter(const String& name, shared_ptr<ITexture> tex)
	{
		MaterialParameter mp;
		mp.parameterType = GL_SAMPLER_2D;
		mp.textureBidning = tex;
		Add(name, mp);
	}

	void MaterialParameterSet::SetParameter(const String& name, const Vector2i& vec2)
	{
		Add(name, MaterialParameter::Create(vec2, GL_INT_VEC2));
	}
}