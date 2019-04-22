#pragma once
#include <Graphics/ResourceTypes.hpp>

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

	class ImageRes;

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

	// OpenGL texture wrapper, can be created from an Image object or raw data
	class Texture : public ITexture
	{
	public:
		~Texture() override = default;
		static auto Create() -> pair<unique_ptr<Texture>, bool>;
		static auto Create(const class ImageRes& image) -> pair<unique_ptr<Texture>, bool>;
		static auto Create(const Vector2i& resolution) -> pair<unique_ptr<Texture>, bool>;

		void Init(Vector2i size, TextureFormat format) override;
		void SetData(Vector2i size, const void* pData) override;
		void SetFromFrameBuffer(Vector2i pos) override;
		void SetMipmaps(bool enabled) override;
		void SetFilter(bool enabled, bool mipFiltering, float anisotropic) override;
		const Vector2i& GetSize() const override;

		// Binds the texture to a given texture unit
		void Bind(uint32 index) override;
		uint32 Handle() override;
		void SetWrap(TextureWrap u, TextureWrap v) override;
		TextureFormat GetFormat() const override;

	private:
		Texture() = default;
		bool Init();
		void UpdateWrap();
		void UpdateFilterState();

		uint32 m_texture = 0;
		array<TextureWrap, 2> m_wmode = { TextureWrap::Repeat };
		TextureFormat m_format = TextureFormat::Invalid;
		Vector2i m_size{};
		bool m_filter = true;
		bool m_mipFilter = true;
		bool m_mipmaps = false;
		float m_anisotropic = 1.0f;
		const void* m_data = nullptr;
	};
}