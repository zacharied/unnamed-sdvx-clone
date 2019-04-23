#pragma once
#include "ITexture.hpp"
#include "IImage.hpp"

namespace Graphics
{
	// OpenGL texture wrapper, can be created from an Image object or raw data
	class Texture : public ITexture
	{
	public:
		~Texture() override = default;
		static auto Create() -> pair<unique_ptr<Texture>, bool>;
		static auto Create(const IImage& image) -> pair<unique_ptr<Texture>, bool>;
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