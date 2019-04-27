#include "stdafx.h"
#include "Image.hpp"
#include "OpenGL.hpp"

#ifdef __APPLE__
#include "libpng16/png.h"
#else
#include "png.h"
#endif

namespace Graphics
{
	Image::~Image()
	{
		Clear();
	}

	auto Image::Create(const String& assetPath) -> optional<unique_ptr<Image>>
	{
		struct EnableMaker : public Image { using Image::Image; };
		auto img = make_unique<EnableMaker>();
		if (!img->Load(assetPath))
			return {};
		return std::move(img);
	}

	auto Image::Create(Vector2i size) -> optional<unique_ptr<Image>>
	{
		struct EnableMaker : public Image { using Image::Image; };
		auto img = make_unique<EnableMaker>();
		img->SetSize(size);
		return std::move(img);
	}

	auto Image::CraeteScreenshot(Vector2i size, Vector2i pos) -> optional<unique_ptr<Image>>
	{
		struct EnableMaker : public Image { using Image::Image; };
		auto img = make_unique<EnableMaker>();
		img->SetSize(size);
		if(img->Screenshot(pos))
			return {};
		return std::move(img);
	}

	bool Image::Screenshot(Vector2i pos)
	{
		GLuint texture;
		//Create texture
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_size.x, m_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFinish();

		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			Logf("OpenGL Error: 0x%p", Logger::Severity::Error, err);
			return false;
		}

		//Set texture from buffer
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glReadBuffer(GL_BACK);
		glBindTexture(GL_TEXTURE_2D, texture);
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pos.x, pos.y, m_size.x, m_size.y);

		while ((err = glGetError()) != GL_NO_ERROR)
		{
			Logf("OpenGL Error: 0x%p", Logger::Severity::Error, err);
			return false;
		}
		glFinish();


		//Copy texture contents to image pData
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pData);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFinish();

		while ((err = glGetError()) != GL_NO_ERROR)
		{
			Logf("OpenGL Error: 0x%p", Logger::Severity::Error, err);
			return false;
		}

		//Delete texture
		glDeleteTextures(1, &texture);
		return true;
	}

	void Image::pngfile_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
	{
		File* pngFile = (File*)png_get_io_ptr(png_ptr);
		pngFile->Write(data, length);
	}

	void Image::SetSize(Vector2i size)
	{
		m_size = size;
		Clear();
		Allocate();
	}

	Vector2i Image::GetSize() const
	{
		return m_size;
	}

	void Image::Clear()
	{
		delete[] m_pData;
		m_pData = nullptr;
	}

	void Image::Allocate()
	{
		m_nDataLength = m_size.x * m_size.y;
		if(m_nDataLength == 0)
			return;
		m_pData = new Colori[m_nDataLength];
	}

	void Image::ReSize(Vector2i size)
	{
		size_t new_DataLength = size.x * size.y;
		if (new_DataLength == 0){
			return;
		}
		auto* new_pData = new Colori[new_DataLength];

		for (int32 ix = 0; ix < size.x; ++ix){
			for (int32 iy = 0; iy < size.y; ++iy){
				int32 sampledX = ix * ((double)m_size.x / (double)size.x);
				int32 sampledY = iy * ((double)m_size.y / (double)size.y);
				new_pData[size.x * iy + ix] = m_pData[m_size.x * sampledY + sampledX];
			}
		}

		delete[] m_pData;
		m_pData = new_pData;
		m_size = size;
		m_nDataLength = m_size.x * m_size.y;
	}

	Colori* Image::GetBits()
	{
		return m_pData;
	}

	const Colori* Image::GetBits() const
	{
		return m_pData;
	}

	void Image::SavePNG(const String& file)
	{
		// TODO: Use shared/File.hpp instead?
		File pngfile;
		pngfile.OpenWrite(Path::Normalize(file));

		png_structp png_ptr = nullptr;
		png_infop info_ptr = nullptr;
		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		info_ptr = png_create_info_struct(png_ptr);
		if (setjmp(png_jmpbuf(png_ptr)))
		{
			/* If we get here, we had a problem writing the file */
			pngfile.Close();
			png_destroy_write_struct(&png_ptr, &info_ptr);
			assert(false);
		}
		png_set_IHDR(png_ptr, info_ptr, m_size.x, m_size.y,
					 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
					 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

		auto* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * m_size.y);
		for (size_t i = 0; i < m_size.y; ++i) {
			row_pointers[m_size.y - i - 1] = (png_bytep)(m_pData + i * m_size.x);
		}

		png_set_write_fn(png_ptr, &pngfile, pngfile_write_data, pngfile_flush);
		png_set_rows(png_ptr, info_ptr, row_pointers);
		png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
		pngfile.Close();
		free(row_pointers);
	}

	void Image::pngfile_flush(png_structp png_ptr)
	{}

	bool Image::Load(const String& fullPath)
	{
		File f;
		if(!f.OpenRead(fullPath))
			return false;

		Buffer b(f.GetSize());
		f.Read(b.data(), b.size());
		if(b.size() < 4)
			return false;

		// Check for PNG based on first 4 bytes
		if(*(uint32*)b.data() == (uint32&)"�PNG")
			return LoadPNG(b);
		else // jay-PEG ?
			return LoadJPEG(b);
	}

	bool Image::LoadPNG(Buffer& in)
	{
		png_image image;
		memset(&image, 0, (sizeof image));
		image.version = PNG_IMAGE_VERSION;

		if(png_image_begin_read_from_memory(&image, in.data(), in.size()) == 0)
			return false;

		image.format = PNG_FORMAT_RGBA;

		SetSize(Vector2i(image.width, image.height));
		Colori* pBuffer = GetBits();
		if(!pBuffer)
			return false;

		if((image.width * image.height * 4) != PNG_IMAGE_SIZE(image))
			return false;

		if(png_image_finish_read(&image, nullptr, pBuffer, 0, nullptr) == 0)
			return false;

		png_image_free(&image);
		return true;
	}

	bool Image::LoadJPEG(Buffer& in)
	{
		/* This struct contains the JPEG decompression parameters and pointers to
			* working space (which is allocated as needed by the JPEG library).
			*/
		jpeg_decompress_struct cinfo;
		jpegErrorMgr jerr = {};
		jerr.reset_error_mgr = &jpegErrorReset;
		jerr.error_exit = &jpegErrorExit;
		jerr.emit_message = &jpegEmitMessage;
		jerr.format_message = &jpegFormatMessage;
		jerr.output_message = &jpegOutputMessage;
		cinfo.err = &jerr;

		// Return point for long jump
		if(setjmp(jerr.jmpBuf) == 0)
		{
			jpeg_create_decompress(&cinfo);
			jpeg_mem_src(&cinfo, in.data(), (uint32)in.size());
			int res = jpeg_read_header(&cinfo, TRUE);

			jpeg_start_decompress(&cinfo);
			int row_stride = cinfo.output_width * cinfo.output_components;
			JSAMPARRAY sample = (*cinfo.mem->alloc_sarray)
					((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

			Vector2i size = Vector2i(cinfo.output_width, cinfo.output_height);
			SetSize(size);
			Colori* pBits = GetBits();

			size_t pixelSize = cinfo.out_color_components;
			cinfo.out_color_space = JCS_RGB;

			while(cinfo.output_scanline < cinfo.output_height)
			{
				jpeg_read_scanlines(&cinfo, sample, 1);
				for(size_t i = 0; i < cinfo.output_width; i++)
				{
					memcpy(pBits + i, sample[0] + i * pixelSize, pixelSize);
					pBits[i].w = 0xFF;
				}

				pBits += size.x;
			}

			jpeg_finish_decompress(&cinfo);
			jpeg_destroy_decompress(&cinfo);
			return true;
		}

		// If we get here, the loading of the jpeg failed
		return false;
	}
}
