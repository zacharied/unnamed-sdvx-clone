#include "stdafx.h"
#include "Window.hpp"
#include "KeyMap.hpp"
#include "Image.hpp"

namespace Graphics
{
	/* SDL Instance singleton */
	class SDL
	{
	protected:
		SDL()
		{
			SDL_SetMainReady();
			int r = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
			if(r != 0)
			{
                Logf("SDL_Init Failed: %s", Logger::Error, SDL_GetError());
                assert(false);
			}
		}
	public:
		~SDL()
		{
			SDL_Quit();
		}
		static SDL& Main()
		{
			static SDL sdl;
			return sdl;
		}
	};

	Window::Window(Vector2i size, uint8 samplecount)
	{
		SDL::Main();

		m_clntSize = size;
		m_caption = L"Window";
		String titleUtf8 = Utility::ConvertToUTF8(m_caption);

		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, samplecount);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 2);

		m_window = SDL_CreateWindow(*titleUtf8, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
									m_clntSize.x, m_clntSize.y, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		assert(m_window);

		uint32 numJoysticks = SDL_NumJoysticks();
		if(numJoysticks == 0)
		{
			Logf("No joysticks found", Logger::Warning);
		}
		else
		{
			Logf("Listing %d Joysticks:", Logger::Info, numJoysticks);
			for(uint32 i = 0; i < numJoysticks; i++)
			{
				SDL_Joystick* joystick = SDL_JoystickOpen(i);
				if(!joystick)
				{
					Logf("[%d] <failed to open>", Logger::Warning, i);
					continue;
				}
				String deviceName = SDL_JoystickName(joystick);

				Logf("[%d] \"%s\" (%d buttons, %d axes, %d hats)", Logger::Info,
					 i, deviceName, SDL_JoystickNumButtons(joystick), SDL_JoystickNumAxes(joystick), SDL_JoystickNumHats(joystick));

				SDL_JoystickClose(joystick);
			}
		}
	}

	Window::~Window()
	{
		SDL_DestroyWindow(m_window);
	}

	void Window::Show()
	{
		SDL_ShowWindow(m_window);
	}

	void Window::Hide()
	{
		SDL_HideWindow(m_window);
	}

	void Window::Close()
	{
		m_closed = true;
	}

	void* Window::Handle()
	{
		return m_window;
	}

	void Window::SetCaption(const WString& cap)
	{
		m_caption = L"Window";
		String titleUtf8 = Utility::ConvertToUTF8(m_caption);
		SDL_SetWindowTitle(m_window, *titleUtf8);
	}

	Vector2i Window::GetMousePos()
	{
		Vector2i res;
		SDL_GetMouseState(&res.x, &res.y);
		return res;
	}

	void Window::SetCursor(const IImage& image, Vector2i hotspot /*= Vector2i(0,0)*/)
	{
#ifdef _WIN32
		if(currentCursor)
		{
			SDL_FreeCursor(currentCursor);
			currentCursor = nullptr;
		}

		Vector2i size = image.GetSize();
		void* bits = (void*)image.GetBits();
		SDL_Surface* surf = SDL_CreateRGBSurfaceFrom(bits, size.x, size.y, 32, size.x * 4,
					0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
		if(surf)
			currentCursor = SDL_CreateColorCursor(surf, hotspot.x, hotspot.y);

		SDL_SetCursor(currentCursor);
#endif
		/// NOTE: Cursor transparency is broken on linux
	}

	void Window::SetCursorVisible(bool visible)
	{
		SDL_ShowCursor(visible);
	}

	void Window::SetWindowStyle(WindowStyle style)
	{}

	Vector2i Window::GetWindowPos() const
	{
		Vector2i res;
		SDL_GetWindowPosition(m_window, &res.x, &res.y);
		return res;
	}

	void Window::SetWindowPos(const Vector2i& pos)
	{
		SDL_SetWindowPosition(m_window, pos.x, pos.y);
	}

	Vector2i Window::GetWindowSize() const
	{
		Vector2i res;
		SDL_GetWindowSize(m_window, &res.x, &res.y);
		return res;
	}

	void Window::SetVSync(int8 setting)
	{
		if(SDL_GL_SetSwapInterval(setting) == -1)
			Logf("Failed to set VSync: %s", Logger::Error, SDL_GetError());
	}

	void Window::SetWindowSize(const Vector2i& size)
	{
		SDL_SetWindowSize(m_window, size.x, size.y);
	}

	void Window::SwitchFullscreen(int w, int h, int fsw, int fsh, uint32 monitorID, bool windowedFullscreen)
	{
		if (monitorID == (uint32)-1)
			monitorID = SDL_GetWindowDisplayIndex(m_window);

		if(m_fullscreen)
		{
			SDL_SetWindowFullscreen(m_window, 0);
			SDL_RestoreWindow(m_window);
			SDL_SetWindowSize(m_window, w, h);
			SDL_SetWindowResizable(m_window, SDL_TRUE);
			SDL_SetWindowBordered(m_window, SDL_TRUE);
			SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED_DISPLAY(monitorID), SDL_WINDOWPOS_CENTERED_DISPLAY(monitorID));
			m_fullscreen = false;
		}
		else if (windowedFullscreen)
		{
			SDL_DisplayMode dm;
			SDL_GetDesktopDisplayMode(monitorID, &dm);
			SDL_Rect bounds;
			SDL_GetDisplayBounds(monitorID, &bounds);

			SDL_RestoreWindow(m_window);
			SDL_SetWindowSize(m_window, dm.w, dm.h);
			SDL_SetWindowPosition(m_window, bounds.x, bounds.y);
			SDL_SetWindowResizable(m_window, SDL_FALSE);
			m_fullscreen = true;

		}
		else
		{
			SDL_DisplayMode dm;
			SDL_GetDesktopDisplayMode(monitorID, &dm);
			if (fsw != -1)
				dm.w = fsw;

			if (fsh != -1)
				dm.h = fsh;

			// move to correct display
			SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED_DISPLAY(monitorID), SDL_WINDOWPOS_CENTERED_DISPLAY(monitorID));

			SDL_SetWindowDisplayMode(m_window, &dm);
			SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN);
			m_fullscreen = true;
		}
	}

	bool Window::IsFullscreen() const
	{
		return m_fullscreen;
	}

	int Window::GetDisplayIndex() const
	{
		return SDL_GetWindowDisplayIndex(m_window);
	}

	bool Window::IsActive() const
	{
		return SDL_GetWindowFlags(m_window) & SDL_WindowFlags::SDL_WINDOW_INPUT_FOCUS;
	}

	void Window::StartTextInput()
	{
		SDL_StartTextInput();
	}
	void Window::StopTextInput()
	{
		SDL_StopTextInput();
	}
	const Graphics::TextComposition& Window::GetTextComposition() const
	{
		return m_textComposition;
	}

	void Window::ShowMessageBox(const String& title, const String& message, int severity)
	{
		uint32 flags = 0;
		switch (severity)
		{
		case 0:
			flags = SDL_MESSAGEBOX_ERROR;
			break;
		case 1:
			flags = SDL_MESSAGEBOX_WARNING;
			break;
		default:
			flags = SDL_MESSAGEBOX_INFORMATION;
		}
		SDL_ShowSimpleMessageBox(flags, title.c_str(), message.c_str(), m_window);
	}

	WString Window::GetClipboard() const
	{
		char* utf8Clipboard = SDL_GetClipboardText();
		WString ret = Utility::ConvertToWString(utf8Clipboard);
		SDL_free(utf8Clipboard);

		return ret;
	}

	void Window::SetMousePos(const Vector2i& pos)
	{
		SDL_WarpMouseInWindow(m_window, pos.x, pos.y);
	}

	void Window::SetRelativeMouseMode(bool enabled)
	{
		if (SDL_SetRelativeMouseMode(enabled ? SDL_TRUE : SDL_FALSE) != 0)
			Logf("SetRelativeMouseMode failed: %s", Logger::Severity::Warning, SDL_GetError());
	}

	bool Window::GetRelativeMouseMode()
	{
		return SDL_GetRelativeMouseMode() == SDL_TRUE;
	}

	void Window::HandleKeyEvent(SDL_Keycode code, uint8 newState, int32 repeat)
	{
		SDL_Keymod m = SDL_GetModState();
		m_modKeys = ModifierKeys::None;

		if((m & KMOD_ALT) != 0)
			(uint8&)m_modKeys |= (uint8)ModifierKeys::Alt;

		if((m & KMOD_CTRL) != 0)
			(uint8&)m_modKeys |= (uint8)ModifierKeys::Ctrl;

		if((m & KMOD_SHIFT) != 0)
			(uint8&)m_modKeys |= (uint8)ModifierKeys::Shift;

		uint8& currentState = m_keyStates[code];
		if(currentState != newState)
		{
			currentState = newState;
			if(newState == 1)
				OnKeyPressed.Call(code);
			else
				OnKeyReleased.Call(code);
		}

		if(currentState == 1)
			OnKeyRepeat.Call(code);
	}

	ImplementBitflagEnum(ModifierKeys);
}