#include "stdafx.h"
#include "Input.hpp"
#include "GameConfig.hpp"
#include "Gamepad/Gamepad_Impl.hpp"

Input::~Input()
{
	// Shoud be set to null by Cleanup
	assert(!m_gamepad);
	assert(!m_window);
}

void Input::Init(Graphics::Window& wnd)
{
	Cleanup();
	m_window = &wnd;

	m_lastMousePos[0] = m_window->GetMousePos().x;
	m_lastMousePos[1] = m_window->GetMousePos().y;

	m_laserDevice = g_gameConfig.GetEnum<Enum_InputDevice>(GameConfigKeys::LaserInputDevice);
	m_buttonDevice = g_gameConfig.GetEnum<Enum_InputDevice>(GameConfigKeys::ButtonInputDevice);

	m_keySensitivity = g_gameConfig.GetFloat(GameConfigKeys::Key_Sensitivity);
	m_keyLaserReleaseTime = g_gameConfig.GetFloat(GameConfigKeys::Key_LaserReleaseTime);

	m_mouseAxisMapping[0] = g_gameConfig.GetInt(GameConfigKeys::Mouse_Laser0Axis);
	m_mouseAxisMapping[1] = g_gameConfig.GetInt(GameConfigKeys::Mouse_Laser1Axis);
	m_mouseSensitivity = g_gameConfig.GetFloat(GameConfigKeys::Mouse_Sensitivity);

	m_controllerAxisMapping[0] = g_gameConfig.GetInt(GameConfigKeys::Controller_Laser0Axis);
	m_controllerAxisMapping[1] = g_gameConfig.GetInt(GameConfigKeys::Controller_Laser1Axis);
	m_controllerSensitivity = g_gameConfig.GetFloat(GameConfigKeys::Controller_Sensitivity);
	m_controllerDeadzone = g_gameConfig.GetFloat(GameConfigKeys::Controller_Deadzone);

	// Init controller mapping
	if(m_laserDevice == InputDevice::Controller || m_buttonDevice == InputDevice::Controller)
	{
		int32 deviceIndex = g_gameConfig.GetInt(GameConfigKeys::Controller_DeviceID);
		if(deviceIndex >= GetNumGamepads())
		{
			Logf("Out of range controller [%d], number of available controllers is %d", Logger::Error, deviceIndex, GetNumGamepads());
		}
		else
		{
			m_gamepad = OpenGamepad(deviceIndex);
			if(m_gamepad)
			{
				m_gamepad->OnButtonPressed.Add(this, &Input::m_OnGamepadButtonPressed);
				m_gamepad->OnButtonReleased.Add(this, &Input::m_OnGamepadButtonReleased);
			}
		}
		m_InitControllerMapping();
	}

	// Init keyboard mapping
	m_InitKeyboardMapping();
}
void Input::Cleanup()
{
	m_window = nullptr;
}

void Input::Update(float deltaTime)
{
	m_SdlPollEvents();
	for(auto it = m_mouseLocks.begin(); it != m_mouseLocks.end();)
	{
		if(it->GetRefCount() == 1)
		{
			it = m_mouseLocks.erase(it);
			continue;
		}
		it++;
	}

	if(!m_mouseLocks.empty())
	{
		if (!m_window->GetRelativeMouseMode())
			m_window->SetRelativeMouseMode(true);
	}
	else if (m_window->GetRelativeMouseMode())
	{
		m_window->SetRelativeMouseMode(false);
	}

	if(m_laserDevice == InputDevice::Mouse)
	{
		for(uint32 i = 0; i < 2; i++)
		{
			if(m_mouseAxisMapping[i] < 0 || m_mouseAxisMapping[i] > 1)
			{
				// INVALID MAPPING
				m_laserStates[i] = 0.0f;
				continue;
			}
			
			m_laserStates[i] = m_mouseSensitivity * m_mousePos[m_mouseAxisMapping[i]];
			m_mousePos[m_mouseAxisMapping[i]] = 0;
		}
	}

	if(m_laserDevice == InputDevice::Keyboard)
	{
		for(uint32 i = 0; i < 2; i++)
		{
			m_laserStates[i] = m_rawKeyLaserStates[i] * deltaTime;

			// if neither laser button is being held fade out the laser input
			if (!m_buttonStates[(int32)Button::LS_0Neg + i * 2 + 1] && !m_buttonStates[(int32)Button::LS_0Neg + i * 2])
			{
				if (m_keyLaserReleaseTime != 0.f)
				{
					float reduction = m_keySensitivity * deltaTime / m_keyLaserReleaseTime;
					if (reduction > fabs(m_rawKeyLaserStates[i]))
						m_rawKeyLaserStates[i] = 0.f;
					else
						m_rawKeyLaserStates[i] -= reduction * Math::Sign(m_rawKeyLaserStates[i]);
				}
				else
				{
					m_rawKeyLaserStates[i] = 0.f;
				}
			}
		}
	}


	if(m_gamepad)
	{
		// Poll controller laser input
		if(m_laserDevice == InputDevice::Controller)
		{
			for(uint32 i = 0; i < 2; i++)
			{
				float axisState = m_gamepad->GetAxis(m_controllerAxisMapping[i]);
				float delta = axisState - m_prevLaserStates[i];
				if (fabs(delta) > 1.5f)
					delta += 2 * (Math::Sign(delta) * -1);
				if (fabs(delta) < m_controllerDeadzone)
					m_laserStates[i] = 0.0f;
				else
					m_laserStates[i] = delta * m_controllerSensitivity;
				m_prevLaserStates[i] = axisState;
			}
		}
	}

	m_absoluteLaserStates[0] = fmodf(m_absoluteLaserStates[0] + m_laserStates[0], Math::pi * 2);
	m_absoluteLaserStates[1] = fmodf(m_absoluteLaserStates[1] + m_laserStates[1], Math::pi * 2);
}

bool Input::GetButton(Button button) const
{
	return m_buttonStates[(size_t)button];
}

float Input::GetAbsoluteLaser(int laser) const
{
	return m_absoluteLaserStates[laser];
}

bool Input::Are3BTsHeld() const
{
	bool bta = GetButton(Input::Button::BT_0);
	bool btb = GetButton(Input::Button::BT_1);
	bool btc = GetButton(Input::Button::BT_2);
	bool btd = GetButton(Input::Button::BT_3);

	return (bta && btb && btc) || (bta && btb && btd) || (bta && btc && btd) || (btb && btc && btd);
}

int Input::GetNumGamepads() const
{
	return SDL_NumJoysticks();
}

shared_ptr<Gamepad> Input::OpenGamepad(int deviceIndex)
{
	auto openGamepad = m_gamepads.find(deviceIndex);
	if (openGamepad != m_gamepads.end())
		return openGamepad->second;

	shared_ptr<Gamepad_Impl> newGamepad = make_shared<Gamepad_Impl>();
	// Try to initialize new device
	if (newGamepad->Init(m_window, deviceIndex))
	{
		// Receive joystick events
		SDL_JoystickEventState(SDL_ENABLE);
	}
	else
	{
		newGamepad.reset();
	}
	if (newGamepad)
	{
		m_gamepads.Add(deviceIndex, newGamepad);
		m_joystickMap.Add(SDL_JoystickInstanceID(newGamepad->m_joystick), newGamepad.get());
	}
	return newGamepad;
}

Vector<String> Input::GetGamepadDeviceNames()
{
	Vector<String> ret;
	uint32 numJoysticks = SDL_NumJoysticks();
	for (uint32 i = 0; i < numJoysticks; i++)
	{
		SDL_Joystick* joystick = SDL_JoystickOpen(i);
		if (!joystick)
		{
			continue;
		}
		String deviceName = SDL_JoystickName(joystick);
		ret.Add(deviceName);

		SDL_JoystickClose(joystick);
	}
	return ret;
}

ModifierKeys Input::GetModifierKeys()
{
	return m_modKeys;
}

String Input::GetControllerStateString() const
{
	if(m_gamepad)
	{
		String s = "Buttons\n";
		for(uint32 i = 0; i < m_gamepad->NumButtons(); i++)
		{
			s += Utility::Sprintf("  [%d]%d\n", i, m_gamepad->GetButton(i));
		}
		s += "\nAxes\n";
		for(uint32 i = 0; i < m_gamepad->NumAxes(); i++)
		{
			s += Utility::Sprintf("  [%d]%.2f\n", i, m_gamepad->GetAxis(i));
		}
		for (uint32 i = 0; i < 2; i++)
		{
			s += Utility::Sprintf("Delta for knob %d: %.2f\n", i, m_laserStates[i]);
		}
		return s;
	}
	return String();
}

Ref<int32> Input::LockMouse()
{
	return m_mouseLocks.Add(MouseLockHandle(new int32(m_mouseLockIndex++)));
}

float Input::GetInputLaserDir(uint32 laserIdx)
{
	return m_laserStates[laserIdx];
}
void Input::m_InitKeyboardMapping()
{
	memset(m_buttonStates, 0, sizeof(m_buttonStates));
	m_buttonMap.clear();

	if(m_buttonDevice == InputDevice::Keyboard)
	{
		// Button mappings
		m_buttonMap.Add(g_gameConfig.GetInt(GameConfigKeys::Key_BTS), Button::BT_S);
		m_buttonMap.Add(g_gameConfig.GetInt(GameConfigKeys::Key_BT0), Button::BT_0);
		m_buttonMap.Add(g_gameConfig.GetInt(GameConfigKeys::Key_BT1), Button::BT_1);
		m_buttonMap.Add(g_gameConfig.GetInt(GameConfigKeys::Key_BT2), Button::BT_2);
		m_buttonMap.Add(g_gameConfig.GetInt(GameConfigKeys::Key_BT3), Button::BT_3);
		// Alternate button mappings
		m_buttonMap.Add(g_gameConfig.GetInt(GameConfigKeys::Key_BT0Alt), Button::BT_0);
		m_buttonMap.Add(g_gameConfig.GetInt(GameConfigKeys::Key_BT1Alt), Button::BT_1);
		m_buttonMap.Add(g_gameConfig.GetInt(GameConfigKeys::Key_BT2Alt), Button::BT_2);
		m_buttonMap.Add(g_gameConfig.GetInt(GameConfigKeys::Key_BT3Alt), Button::BT_3);

		m_buttonMap.Add(g_gameConfig.GetInt(GameConfigKeys::Key_FX0), Button::FX_0);
		m_buttonMap.Add(g_gameConfig.GetInt(GameConfigKeys::Key_FX1), Button::FX_1);
		// Alternate button mappings
		m_buttonMap.Add(g_gameConfig.GetInt(GameConfigKeys::Key_FX0Alt), Button::FX_0);
		m_buttonMap.Add(g_gameConfig.GetInt(GameConfigKeys::Key_FX1Alt), Button::FX_1);
	}

	if(m_laserDevice == InputDevice::Keyboard)
	{
		// Laser button mappings
		m_buttonMap.Add(g_gameConfig.GetInt(GameConfigKeys::Key_Laser0Neg), Button::LS_0Neg);
		m_buttonMap.Add(g_gameConfig.GetInt(GameConfigKeys::Key_Laser0Pos), Button::LS_0Pos);
		m_buttonMap.Add(g_gameConfig.GetInt(GameConfigKeys::Key_Laser1Neg), Button::LS_1Neg);
		m_buttonMap.Add(g_gameConfig.GetInt(GameConfigKeys::Key_Laser1Pos), Button::LS_1Pos);
	}
}

void Input::m_InitControllerMapping()
{
	m_controllerMap.clear();
	if(m_buttonDevice == InputDevice::Controller)
	{
		m_controllerMap.Add(g_gameConfig.GetInt(GameConfigKeys::Controller_BTS), Button::BT_S);
		m_controllerMap.Add(g_gameConfig.GetInt(GameConfigKeys::Controller_BT0), Button::BT_0);
		m_controllerMap.Add(g_gameConfig.GetInt(GameConfigKeys::Controller_BT1), Button::BT_1);
		m_controllerMap.Add(g_gameConfig.GetInt(GameConfigKeys::Controller_BT2), Button::BT_2);
		m_controllerMap.Add(g_gameConfig.GetInt(GameConfigKeys::Controller_BT3), Button::BT_3);
		m_controllerMap.Add(g_gameConfig.GetInt(GameConfigKeys::Controller_FX0), Button::FX_0);
		m_controllerMap.Add(g_gameConfig.GetInt(GameConfigKeys::Controller_FX1), Button::FX_1);
	}
}

void Input::m_OnButtonInput(Button b, bool pressed)
{
	bool& state = m_buttonStates[(size_t)b];
	if(state != pressed)
	{
		state = pressed;
		if(state)
		{
			OnButtonPressed.Call(b);
		}
		else
		{
			OnButtonReleased.Call(b);
		}
	}

	static Timer t;
	if(b >= Button::LS_0Neg)
	{
		int32 btnIdx = (int32)b - (int32)Button::LS_0Neg;
		int32 laserIdx = btnIdx / 2;
		// Set laser state based uppon the button that was pressed last
		if(pressed)
			m_rawKeyLaserStates[laserIdx] = (btnIdx % 2) == 0 ? -m_keySensitivity : m_keySensitivity;
		else // If a button was released check if the other one is still held
		{
			if(m_buttonStates[(int32)Button::LS_0Neg + laserIdx * 2])
				m_rawKeyLaserStates[laserIdx] = -m_keySensitivity;
			else if(m_buttonStates[(int32)Button::LS_0Neg + laserIdx * 2 + 1])
				m_rawKeyLaserStates[laserIdx] = m_keySensitivity;

		}
	}
}

void Input::m_OnGamepadButtonPressed(uint8 button)
{
	// Handle button mappings
	auto it = m_controllerMap.equal_range(button);
	for(auto it1 = it.first; it1 != it.second; it1++)
		m_OnButtonInput(it1->second, true);
}
void Input::m_OnGamepadButtonReleased(uint8 button)
{
	// Handle button mappings
	auto it = m_controllerMap.equal_range(button);
	for(auto it1 = it.first; it1 != it.second; it1++)
		m_OnButtonInput(it1->second, false);
}

void Input::m_HandleKeyEvent(SDL_Keycode code, uint8 newState, int32 repeat)
{
	SDL_Keymod m = SDL_GetModState();
	m_modKeys = ModifierKeys::None;
	if ((m & KMOD_ALT) != 0)
	{
		(uint8&)m_modKeys |= (uint8)ModifierKeys::Alt;
	}
	if ((m & KMOD_CTRL) != 0)
	{
		(uint8&)m_modKeys |= (uint8)ModifierKeys::Ctrl;
	}
	if ((m & KMOD_SHIFT) != 0)
	{
		(uint8&)m_modKeys |= (uint8)ModifierKeys::Shift;
	}


	uint8& currentState = m_keyStates[code];
	if (currentState != newState)
	{
		currentState = newState;
		if (newState == 1)
		{
			OnKeyPressed.Call(code);
			m_OnKeyPressed(code);
		}
		else
		{
			OnKeyReleased.Call(code);
			m_OnKeyReleased(code);
		}
	}
	if (currentState == 1)
	{
		OnKeyRepeat.Call(code);
	}
}

void Input::m_SdlPollEvents()
{
	// Update loop
	Timer t;
	SDL_Event evt;
	while (SDL_PollEvent(&evt))
	{
		if (evt.type == SDL_EventType::SDL_KEYDOWN)
		{
			if (m_textComposition.composition.empty())
			{
				// Ignore key input when composition is being typed
				m_HandleKeyEvent(evt.key.keysym.sym, 1, evt.key.repeat);
			}
		}
		else if (evt.type == SDL_EventType::SDL_KEYUP)
		{
			m_HandleKeyEvent(evt.key.keysym.sym, 0, 0);
		}
		else if (evt.type == SDL_EventType::SDL_JOYBUTTONDOWN)
		{
			Gamepad_Impl** gp = m_joystickMap.Find(evt.jbutton.which);
			if (gp)
				gp[0]->HandleInputEvent(evt.jbutton.button, true);
		}
		else if (evt.type == SDL_EventType::SDL_JOYBUTTONUP)
		{
			Gamepad_Impl** gp = m_joystickMap.Find(evt.jbutton.which);
			if (gp)
				gp[0]->HandleInputEvent(evt.jbutton.button, false);
		}
		else if (evt.type == SDL_EventType::SDL_JOYAXISMOTION)
		{
			Gamepad_Impl** gp = m_joystickMap.Find(evt.jaxis.which);
			if (gp)
				gp[0]->HandleAxisEvent(evt.jaxis.axis, evt.jaxis.value);
		}
		else if (evt.type == SDL_EventType::SDL_JOYHATMOTION)
		{
			Gamepad_Impl** gp = m_joystickMap.Find(evt.jhat.which);
			if (gp)
				gp[0]->HandleHatEvent(evt.jhat.hat, evt.jhat.value);
		}
		else if (evt.type == SDL_EventType::SDL_MOUSEBUTTONDOWN)
		{
			switch (evt.button.button)
			{
			case SDL_BUTTON_LEFT:
				OnMousePressed.Call(MouseButton::Left);
				break;
			case SDL_BUTTON_MIDDLE:
				OnMousePressed.Call(MouseButton::Middle);
				break;
			case SDL_BUTTON_RIGHT:
				OnMousePressed.Call(MouseButton::Right);
				break;
			}
		}
		else if (evt.type == SDL_EventType::SDL_MOUSEBUTTONUP)
		{
			switch (evt.button.button)
			{
			case SDL_BUTTON_LEFT:
				OnMouseReleased.Call(MouseButton::Left);
				break;
			case SDL_BUTTON_MIDDLE:
				OnMouseReleased.Call(MouseButton::Middle);
				break;
			case SDL_BUTTON_RIGHT:
				OnMouseReleased.Call(MouseButton::Right);
				break;
			}
		}
		else if (evt.type == SDL_EventType::SDL_MOUSEWHEEL)
		{
			if (evt.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
			{
				OnMouseScroll.Call(evt.wheel.y);
			}
			else
			{
				OnMouseScroll.Call(-evt.wheel.y);
			}
		}
		else if (evt.type == SDL_EventType::SDL_MOUSEMOTION)
		{
			OnMouseMotion.Call(evt.motion.xrel, evt.motion.yrel);
			m_OnMouseMotion(evt.motion.xrel, evt.motion.yrel);
		}
		else if (evt.type == SDL_EventType::SDL_QUIT)
		{
			OnQuit.Call();
		}
		else if (evt.type == SDL_EventType::SDL_WINDOWEVENT)
		{
			if (evt.window.windowID == SDL_GetWindowID((SDL_Window*)m_window->Handle()))
			{
				if (evt.window.event == SDL_WindowEventID::SDL_WINDOWEVENT_SIZE_CHANGED)
				{
					Vector2i newSize(evt.window.data1, evt.window.data2);
					OnResized.Call(newSize);
				}
			}
		}
		else if (evt.type == SDL_EventType::SDL_TEXTINPUT)
		{
			WString wstr = Utility::ConvertToWString(evt.text.text);
			OnTextInput.Call(wstr);
		}
		else if (evt.type == SDL_EventType::SDL_TEXTEDITING)
		{
			SDL_Rect scr;
			SDL_GetWindowPosition((SDL_Window*)m_window->Handle(), &scr.x, &scr.y);
			SDL_GetWindowSize((SDL_Window*)m_window->Handle(), &scr.w, &scr.h);
			SDL_SetTextInputRect(&scr);

			m_textComposition.composition = Utility::ConvertToWString(evt.edit.text);
			m_textComposition.cursor = evt.edit.start;
			m_textComposition.selectionLength = evt.edit.length;
			OnTextComposition.Call(m_textComposition);
		}
		OnAnyEvent.Call(evt);
	}
}



void Input::m_OnKeyPressed(int32 key)
{
	// Handle button mappings
	auto it = m_buttonMap.equal_range(key);
	for(auto it1 = it.first; it1 != it.second; it1++)
		m_OnButtonInput(it1->second, true);
}
void Input::m_OnKeyReleased(int32 key)
{
	// Handle button mappings
	auto it = m_buttonMap.equal_range(key);
	for(auto it1 = it.first; it1 != it.second; it1++)
		m_OnButtonInput(it1->second, false);
}

void Input::m_OnMouseMotion(int32 x, int32 y)
{
	m_mousePos[0] += x;
	m_mousePos[1] += y;
}
