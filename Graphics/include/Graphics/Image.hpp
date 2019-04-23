#pragma once

#include <png.h>
#include <jpeglib.h>
#include "IImage.hpp"

namespace Graphics
{
	/*
		RGBA8 image class
		The bits have the same layout as the Colori class
	*/
	class Image : public IImage
	{
	public:
		~Image() override;
		static auto Create(const String& assetPath) -> optional<unique_ptr<Image>>;
		static auto Create(Vector2i size = Vector2i{}) -> optional<unique_ptr<Image>>;
		static auto CraeteScreenshot(Vector2i size = Vector2i(), Vector2i pos = Vector2i()) -> optional<unique_ptr<Image>>;

		void SetSize(Vector2i size) override;
		void ReSize(Vector2i size) override;
		Vector2i GetSize() const override;
		Colori* GetBits() override;
		const Colori* GetBits() const override;
		void SavePNG(const String& file) override;

	private:
		Vector2i m_size;
		Colori* m_pData = nullptr;
		size_t m_nDataLength{};

		void Clear();
		void Allocate();
		bool Screenshot(Vector2i pos);
		bool Load(const String& fullPath);
		bool LoadPNG(Buffer& in);
		bool LoadJPEG(Buffer& in);
		static void pngfile_write_data(png_structp png_ptr, png_bytep data, png_size_t length);
		static void pngfile_flush(png_structp png_ptr);

		// error handling
		struct jpegErrorMgr : public jpeg_error_mgr
		{
			jmp_buf jmpBuf;
		};

		static void jpegErrorExit(jpeg_common_struct* cinfo)
		{
			longjmp(((jpegErrorMgr*)cinfo->err)->jmpBuf, 1);
		}

		static void jpegErrorReset(jpeg_common_struct* cinfo)
		{}

		static void jpegEmitMessage(jpeg_common_struct* cinfo, int msgLvl)
		{}

		static void jpegOutputMessage(jpeg_common_struct* cinfo)
		{}

		static void jpegFormatMessage(jpeg_common_struct* cinfo, char * buffer)
		{}
	};
}