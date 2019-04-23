#pragma once

namespace Graphics
{
	enum class TextureWrap
	{
		Repeat,
		Mirror,
		Clamp,
	};

	enum class TextureFormat
	{
		RGBA8,
		D32,
		Invalid
	};

	class ITexture
	{
	public:
		virtual ~ITexture() = default;

		virtual void Init(Vector2i size, TextureFormat format) = 0;
		virtual void SetData(Vector2i size, const void* pData) = 0;
		virtual void SetFromFrameBuffer(Vector2i pos) = 0;
		virtual void SetMipmaps(bool enabled) = 0;
		virtual void SetFilter(bool enabled, bool mipFiltering, float anisotropic) = 0;
		virtual const Vector2i& GetSize() const = 0;

		// Gives the aspect ratio correct height for a given width
		float CalculateHeight(float width);
		// Gives the aspect ratio correct width for a given height
		float CalculateWidth(float height);

		// Binds the texture to a given texture unit (default = 0)
		virtual void Bind(uint32 index) = 0;
		virtual uint32 Handle() = 0;
		virtual void SetWrap(TextureWrap u, TextureWrap v) = 0;
		virtual TextureFormat GetFormat() const = 0;
	};
}