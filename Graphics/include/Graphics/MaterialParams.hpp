#pragma once
#include <Shared/Shared.hpp>
#include <Graphics/ITexture.hpp>

namespace Graphics
{
	/* A single parameter that is set for a material */
	struct MaterialParameter
	{
		CopyableBuffer parameterData;
		uint32 parameterType;
		shared_ptr<ITexture> textureBidning;

		template<typename T>
		static MaterialParameter Create(const T& obj, uint32 type)
		{
			MaterialParameter r;
			r.Bind(obj);
			r.parameterType = type;
			return r;
		}
		template<typename T>
		void Bind(const T& obj)
		{
			parameterData.resize(sizeof(T));
			memcpy(parameterData.data(), &obj, sizeof(T));
		}
		template<typename T>
		const T& Get()
		{
			assert(sizeof(T) == parameterData.size());
			return *(T*)parameterData.data();
		}

		bool operator==(const MaterialParameter& other) const
		{
			if(parameterType != other.parameterType)
				return false;
			if(parameterData.size() != other.parameterData.size())
				return false;
			return memcmp(parameterData.data(), other.parameterData.data(), parameterData.size()) == 0;
		}
	};

	/*
		A list of parameters that is set for a material
		use SetParameter(name, param) to set any parameter by name
	*/
	class MaterialParameterSet : public Map<String, MaterialParameter>
	{
	public:
		using Map<String, MaterialParameter>::Map;
		void SetParameter(const String& name, int sc);
		void SetParameter(const String& name, float sc);
		void SetParameter(const String& name, const Vector4& vec);
		void SetParameter(const String& name, const Colori& color);
		void SetParameter(const String& name, const Vector2& vec2);
		void SetParameter(const String& name, const Vector3& vec3);
		void SetParameter(const String& name, const Vector2i& vec2);
		void SetParameter(const String& name, const Transform& tf);
		void SetParameter(const String& name, shared_ptr<ITexture> tex);
	};

	enum class MaterialBlendMode
	{
		Normal,
		Additive,
		Multiply,
	};
}