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

	// Controller state as a string
	// Primarily used for debugging
	String GetControllerStateString() const;

	// Returns a handle to a mouse lock, release it to unlock the mouse
	MouseLockHandle LockMouse();

	// Event handlers
	virtual void OnKeyPressed(Key key);
	virtual void OnKeyReleased(Key key);

	// Request laser input state
	float GetInputLaserDir(uint32 laserIdx);

	// Button delegates
	Delegate<Button> OnButtonPressed;
	Delegate<Button> OnButtonReleased;

private:
	void m_InitKeyboardMapping();
	void m_InitControllerMapping();
	void m_OnButtonInput(Button b, bool pressed);

	void m_OnGamepadButtonPressed(uint8 button);
	void m_OnGamepadButtonReleased(uint8 button);

	int32 m_mouseLockIndex = 0;
	Vector<MouseLockHandle> m_mouseLocks;

	InputDevice m_laserDevice;
	InputDevice m_buttonDevice;

	bool m_buttonStates[(size_t)Button::Length];
	float m_laserStates[2] = { 0.0f };

	// Keyboard bindings
	Multimap<Key, Button> m_buttonMap;

	// Mouse bindings
	uint32 m_mouseAxisMapping[2] = { 0,1 };
	float m_mouseSensitivity;
	Vector2 m_lastMousePos;

	// Controller bindings
	Multimap<uint32, Button> m_controllerMap;
	uint32 m_controllerAxisMapping[2] = { 0,1 };
	float m_controllerSensitivity;
	float m_controllerDeadzone;

	Ref<Gamepad> m_gamepad;

	Graphics::Window* m_window = nullptr;
};