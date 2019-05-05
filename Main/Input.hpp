#pragma once

// Types of input device
DefineEnum(InputDevice,
	Keyboard,
	Mouse,
	Controller);

typedef Ref<int32> MouseLockHandle;

/*
	Class that handles game keyboard (and soon controller input)
*/
class Input : Unique
{
public:
	DefineEnum(Button,
		BT_0,
		BT_1,
		BT_2,
		BT_3,
		FX_0,
		FX_1,
        BT_S, //Start Button
		LS_0Neg, // Left laser- 
		LS_0Pos, // Left laser+		(|---->)
		LS_1Neg, // Right laser-	(<----|)
		LS_1Pos, // Right laser+
		Length);

	~Input();
	void Init(Graphics::Window& wnd);
	void Cleanup();

	// Poll/Update input
	void Update(float deltaTime);

	bool GetButton(Button button) const;
	float GetAbsoluteLaser(int laser) const;
	bool Are3BTsHeld() const;
	int GetNumGamepads() const;
	shared_ptr<Gamepad> OpenGamepad(int index);
	Vector<String> GetGamepadDeviceNames();
	ModifierKeys GetModifierKeys();
	// Controller state as a string
	// Primarily used for debugging
	String GetControllerStateString() const;

	// Returns a handle to a mouse lock, release it to unlock the mouse
	MouseLockHandle LockMouse();

	// Request laser input state
	float GetInputLaserDir(uint32 laserIdx);

	// Button delegates
	Delegate<Button> OnButtonPressed;
	Delegate<Button> OnButtonReleased;

	Delegate<int32> OnKeyPressed;
	Delegate<int32> OnKeyReleased;
	Delegate<MouseButton> OnMousePressed;
	Delegate<MouseButton> OnMouseReleased;
	Delegate<int32, int32> OnMouseMotion;
	Delegate<> OnQuit;
	Delegate<SDL_Event> OnAnyEvent;
	// Mouse scroll wheel 
	//	Positive for scroll down
	//	Negative for scroll up
	Delegate<int32> OnMouseScroll;
	// Called for the initial an repeating presses of a key
	Delegate<int32> OnKeyRepeat;
	Delegate<const WString&> OnTextInput;
	Delegate<const TextComposition&> OnTextComposition;
	Delegate<const Vector2i&> OnResized;

private:
	Map<int32, shared_ptr<class Gamepad_Impl>> m_gamepads;
	Map<SDL_JoystickID, Gamepad_Impl*> m_joystickMap;
	void m_InitKeyboardMapping();
	void m_InitControllerMapping();
	void m_OnButtonInput(Button b, bool pressed);

	void m_OnGamepadButtonPressed(uint8 button);
	void m_OnGamepadButtonReleased(uint8 button);
	void m_HandleKeyEvent(SDL_Keycode code, uint8 newState, int32 repeat);
	void m_SdlPollEvents();
	// Event handlers
	void m_OnKeyPressed(int32 key);
	void m_OnKeyReleased(int32 key);
	void m_OnMouseMotion(int32 x, int32 y);

	int32 m_mouseLockIndex = 0;
	Vector<MouseLockHandle> m_mouseLocks;

	InputDevice m_laserDevice;
	InputDevice m_buttonDevice;

	bool m_buttonStates[(size_t)Button::Length];
	float m_laserStates[2] = { 0.0f };
	float m_rawKeyLaserStates[2] = { 0.0f };
	float m_prevLaserStates[2] = { 0.0f };
	float m_absoluteLaserStates[2] = { 0.0f };
	ModifierKeys m_modKeys;
	Map<SDL_Keycode, uint8> m_keyStates;
	// Keyboard bindings
	Multimap<int32, Button> m_buttonMap;
	float m_keySensitivity;
	float m_keyLaserReleaseTime;

	// Mouse bindings
	uint32 m_mouseAxisMapping[2] = { 0,1 };
	float m_mouseSensitivity;
	int32 m_lastMousePos[2];
	int32 m_mousePos[2];

	// Controller bindings
	Multimap<uint32, Button> m_controllerMap;
	uint32 m_controllerAxisMapping[2] = { 0,1 };
	float m_controllerSensitivity;
	float m_controllerDeadzone;

	shared_ptr<Gamepad> m_gamepad;
	TextComposition m_textComposition;

	Graphics::Window* m_window = nullptr;
};
