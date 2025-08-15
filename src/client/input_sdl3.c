//
// input_sdl3.c
//
// Copyright 1998 Raven Software
//

#include "client.h"
#include "glimp_sdl3.h"
#include "input.h"
#include "qcommon.h"
#include "menus/menu_misc.h"

#include <SDL3/SDL.h>

uint sys_frame_time;

// Mouse vars:
static float mouse_x;
static float mouse_y;
static cvar_t* m_filter;
static qboolean mlooking;

void IN_GetClipboardText(char* out, const size_t n) // YQ2
{
	char* s = SDL_GetClipboardText();

	if (s == NULL || *s == '\0')
	{
		*out = '\0';
		return;
	}

	strcpy_s(out, n - 1, s);
	SDL_free(s);
}

static qboolean IN_ShouldGrabInput(void) //mxd
{
	if (vid_mode->value == 0.0f) // Always hide mouse when in fullscreen mode.
		return true;

	if (cls.key_dest == key_menu) // Show mouse when in menus.
		return false;

	if (cls.key_dest == key_console) // Show mouse when in console.
		return false;

	if (cls.key_dest == key_game && (cls.state == ca_disconnected || cls.state == ca_connecting)) // Show mouse when in fullscreen console.
		return false;

	return true; // Otherwise hide mouse.
}

// This function translates SDL keycodes into the id Tech 2 engines internal representation.
static int IN_TranslateSDLtoQ2Key(const uint keysym)
{
	// These must be translated.
	switch (keysym)
	{
		case SDLK_TAB:			return K_TAB;
		case SDLK_RETURN:		return K_ENTER;
		case SDLK_ESCAPE:		return K_ESCAPE;
		case SDLK_BACKSPACE:	return K_BACKSPACE;
		case SDLK_CAPSLOCK:		return K_CAPSLOCK;
		case SDLK_PAUSE:		return K_PAUSE;

		case SDLK_UP:			return K_UPARROW;
		case SDLK_DOWN:			return K_DOWNARROW;
		case SDLK_LEFT:			return K_LEFTARROW;
		case SDLK_RIGHT:		return K_RIGHTARROW;

		case SDLK_RALT:			return K_ALT;
		case SDLK_LALT:			return K_ALT;

		case SDLK_LCTRL:		return K_CTRL;
		case SDLK_RCTRL:		return K_CTRL;

		case SDLK_LSHIFT:		return K_SHIFT;
		case SDLK_RSHIFT:		return K_SHIFT;

		case SDLK_INSERT:		return K_INS;
		case SDLK_DELETE:		return K_DEL;
		case SDLK_PAGEDOWN:		return K_PGDN;
		case SDLK_PAGEUP:		return K_PGUP;
		case SDLK_HOME:			return K_HOME;
		case SDLK_END:			return K_END;

		case SDLK_F1:			return K_F1;
		case SDLK_F2:			return K_F2;
		case SDLK_F3:			return K_F3;
		case SDLK_F4:			return K_F4;
		case SDLK_F5:			return K_F5;
		case SDLK_F6:			return K_F6;
		case SDLK_F7:			return K_F7;
		case SDLK_F8:			return K_F8;
		case SDLK_F9:			return K_F9;
		case SDLK_F10:			return K_F10;
		case SDLK_F11:			return K_F11;
		case SDLK_F12:			return K_F12;

		case SDLK_KP_0:			return K_KP_INS;
		case SDLK_KP_1:			return K_KP_END;
		case SDLK_KP_2:			return K_KP_DOWNARROW;
		case SDLK_KP_3:			return K_KP_PGDN;
		case SDLK_KP_4:			return K_KP_LEFTARROW;
		case SDLK_KP_5:			return K_KP_5;
		case SDLK_KP_6:			return K_KP_RIGHTARROW;
		case SDLK_KP_7:			return K_KP_HOME;
		case SDLK_KP_8:			return K_KP_UPARROW;
		case SDLK_KP_9:			return K_KP_PGUP;
		case SDLK_KP_ENTER:		return K_KP_ENTER;
		case SDLK_KP_PERIOD:	return K_KP_DEL;
		case SDLK_KP_DIVIDE:	return K_KP_SLASH;
		case SDLK_KP_MINUS:		return K_KP_MINUS;
		case SDLK_KP_PLUS:		return K_KP_PLUS;
		case SDLK_NUMLOCKCLEAR:	return K_KP_NUMLOCK;

		default:				return 0;
	}
}

#pragma region ========================== MOUSE CONTROL ==========================

// Q2 counterpart
static void IN_MLookDown(void) //TODO: ancient "mouselook only when "mlook" key is pressed" logic. Remove?
{
	mlooking = true;
}

// Q2 counterpart
static void IN_MLookUp(void) //TODO: ancient "mouselook only when "mlook" key is pressed" logic. Remove?
{
	mlooking = false;

	if (!(int)freelook->value && (int)lookspring->value)
		IN_CenterView();
}

static void IN_InitMouse(void)
{
	mouse_x = 0.0f;
	mouse_y = 0.0f;

	m_filter = Cvar_Get("m_filter", "0", 0);

	Cmd_AddCommand("+mlook", IN_MLookDown);
	Cmd_AddCommand("-mlook", IN_MLookUp);
}

static void IN_ShutdownMouse(void)
{
	Cmd_RemoveCommand("+mlook");
	Cmd_RemoveCommand("-mlook");
}

static void IN_MouseMove(usercmd_t* cmd)
{
	static float old_mouse_x;
	static float old_mouse_y;

	if ((int)m_filter->value)
	{
		mouse_x = (mouse_x + old_mouse_x) * 0.5f;
		mouse_y = (mouse_y + old_mouse_y) * 0.5f;
	}

	old_mouse_x = mouse_x;
	old_mouse_y = mouse_y;

	if (mouse_x == 0.0f && mouse_y == 0.0f)
		return;

	mouse_x *= mouse_sensitivity_x->value; // 'sensitivity' cvar in Q2
	mouse_y *= mouse_sensitivity_y->value; // 'sensitivity' cvar in Q2

	// Add mouse X/Y movement to cmd.
	if ((in_strafe.state & 1) || ((int)lookstrafe->value && mlooking)) //TODO: remove 'lookstrafe' cvar, always freelook.
		cmd->sidemove += (short)(mouse_x * m_side->value);
	else
		cl.delta_inputangles[YAW] -= mouse_x * m_yaw->value;

	if (!(in_strafe.state & 1) || ((int)freelook->value && mlooking)) // H2: no 'else' case //TODO: remove freelook cvar, always freelook.
		cl.delta_inputangles[PITCH] += mouse_y * m_pitch->value;

	// Reset mouse position.
	mouse_x = 0.0f;
	mouse_y = 0.0f;
}

#pragma endregion

#pragma region ========================== GAMEPAD CONTROL ==========================

static void IN_InitController(void)
{
	//TODO: implement later...
}

static void IN_StartupController(void) // YQ2: IN_Controller_Init().
{
	//TODO: implement later...
}

static void IN_ShutdownController(void) // YQ2: IN_Controller_Shutdown().
{
	//TODO: implement later...
}

static void IN_ControllerMove(usercmd_t* cmd)
{
	//TODO: implement later...
}

#pragma endregion

void IN_Init(void)
{
	IN_InitMouse();
	IN_InitController();

	if (!SDL_WasInit(SDL_INIT_EVENTS) && !SDL_Init(SDL_INIT_EVENTS))
		Com_Error(ERR_FATAL, "Couldn't initialize SDL event subsystem:%s\n", SDL_GetError());

	IN_StartupController();
}

// Shuts the backend down.
void IN_Shutdown(void)
{
	Com_Printf("Shutting down input.\n");

	IN_ShutdownMouse();
	IN_ShutdownController();

	const SDL_InitFlags subsystems = (SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_EVENTS);
	if (SDL_WasInit(subsystems) == subsystems)
		SDL_QuitSubSystem(subsystems);
}

// Updates the input queue state. Called every frame by the client and does nearly all the input magic.
void IN_Update(void) // YQ2
{
#define NANOSECONDS_IN_MILLISECOND	1000000
	SDL_Event event;

	// Get and process an event.
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_EVENT_MOUSE_WHEEL:
			{
				const int key = (event.wheel.y > 0 ? K_MWHEELUP : K_MWHEELDOWN);
				const uint time = (uint)(event.wheel.timestamp / NANOSECONDS_IN_MILLISECOND);
				Key_Event(key, true, time);
				Key_Event(key, false, time);
			} break;

			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			case SDL_EVENT_MOUSE_BUTTON_UP:
			{
				int key;

				switch (event.button.button)
				{
					case SDL_BUTTON_LEFT:
						key = K_MOUSE1;
						break;

					case SDL_BUTTON_MIDDLE:
						key = K_MOUSE3;
						break;

					case SDL_BUTTON_RIGHT:
						key = K_MOUSE2;
						break;

					default:
						return;
				}

				const uint time = (uint)(event.button.timestamp / NANOSECONDS_IN_MILLISECOND);
				Key_Event(key, (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN), time);
			} break;

			case SDL_EVENT_MOUSE_MOTION:
				if (cls.key_dest == key_game && (int)cl_paused->value == 0)
				{
					mouse_x += event.motion.xrel;
					mouse_y += event.motion.yrel;
				}
				break;

			case SDL_EVENT_KEY_DOWN:
			case SDL_EVENT_KEY_UP:
			{
				const qboolean down = (event.type == SDL_EVENT_KEY_DOWN);

				// Workaround for AZERTY-keyboards, which don't have 1, 2, ..., 9, 0 in first row:
				// always map those physical keys (scancodes) to those keycodes anyway. See: https://bugzilla.libsdl.org/show_bug.cgi?id=3188
				const SDL_Scancode sc = event.key.scancode;
				const uint time = (uint)(event.key.timestamp / NANOSECONDS_IN_MILLISECOND);

				if (sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_0)
				{
					// Note that the SDL_SCANCODEs are SDL_SCANCODE_1, _2, ..., _9, SDL_SCANCODE_0 while in ASCII it's '0', '1', ..., '9' => handle 0 and 1-9 separately.
					// (quake2 uses the ASCII values for those keys).
					int key = '0'; // Implicitly handles SDL_SCANCODE_0.

					if (sc <= SDL_SCANCODE_9)
						key = '1' + (sc - SDL_SCANCODE_1);

					Key_Event(key, down, time);
				}
				else
				{
					const SDL_Keycode kc = event.key.key;

					if (sc == SDL_SCANCODE_GRAVE && kc != '\'' && kc != '"')
					{
						// Special case/hack: open the console with the "console key" (beneath Esc, left of 1, above Tab)
						// but not if the keycode for this is a quote (like on Brazilian keyboards) - otherwise you couldn't type them in the console.
						if ((event.key.mod & (SDL_KMOD_CAPS | SDL_KMOD_SHIFT | SDL_KMOD_ALT | SDL_KMOD_CTRL | SDL_KMOD_GUI)) == 0)
						{
							// Also, only do this if no modifiers like shift or AltGr or whatever are pressed,
							// so kc will most likely be the ascii char generated by this and can be ignored
							// in case SDL_TEXTINPUT above (so we don't get ^ or whatever as text in console)
							// (can't just check for mod == 0 because numlock is a KMOD too).
							Key_Event('~', down, time);
						}
					}
					else if (kc >= SDLK_SPACE && kc < SDLK_DELETE)
					{
						Key_Event((int)kc, down, time);
					}
					else
					{
						const int key = IN_TranslateSDLtoQ2Key(kc);

						if (key != 0)
							Key_Event(key, down, time);
						else
							Com_DPrintf("Pressed unknown key with SDL_Keycode %d, SDL_Scancode %d.\n", kc, (int)sc);
					}
				}
			} break;

			case SDL_EVENT_WINDOW_FOCUS_LOST:
				Key_ClearStates();
				se.Activate(false); //mxd. Also deactivates music backend.
				break;

			case SDL_EVENT_WINDOW_FOCUS_GAINED:
				se.Activate(true); //mxd. Also activates music backend.
				break;

			case SDL_EVENT_WINDOW_SHOWN:
				cls.disable_screen = false; // H2
				break;

			case SDL_EVENT_WINDOW_HIDDEN:
				cls.disable_screen = true; // H2
				break;

			case SDL_EVENT_QUIT:
				Com_Quit();
		}
	}

	// Grab and ungrab the mouse if the console is opened.
	// Calling GLimp_GrabInput() each frame is a bit ugly but simple and should work.
	// The called SDL functions return after a cheap check, if there's nothing to do.
	GLimp_GrabInput(IN_ShouldGrabInput());

	// We need to save the frame time so other subsystems know the exact time of the last input events.
	sys_frame_time = Sys_Milliseconds();
}

void IN_Move(usercmd_t* cmd) // Called on packetframe or renderframe.
{
	IN_MouseMove(cmd);
	IN_ControllerMove(cmd);
}

// Removes all pending events from SDLs queue.
void In_FlushQueue(void) // YQ2
{
	SDL_FlushEvents(SDL_EVENT_FIRST, SDL_EVENT_LAST);
	Key_ClearStates();
}