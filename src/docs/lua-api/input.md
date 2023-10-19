NewCity LUA API - Input

##### isKeyDown(number _key_)

Returns true if the player is currently holding down _key_ on their keyboard.

Refer to https://www.glfw.org/docs/latest/group__keys.html for the key numbers. For example, A is 65.

> isKeyDown(65) => $isKeyDown(65)$

##### getMouseLocation_screenspace() => table _location_

Returns the current screenspace coordinates of the mouse cursor. _location_ is a table with fields _x_ and _y_. The coordinates vary between -1 and 1. -1,-1 is the upper left corner.

> getMouseLocation_screenspace().x => $getMouseLocation_screenspace().x$

##### getMouseLocation_worldspace() => table _location_

Returns the current worldspace coordinates of the mouse cursor. _location_ is a table with fields _x_, _y_, and _z_. _z_ is the elevation.

Note: getMouseLocation_worldspace is a performance sensitive function. Don't call it multiple times per frame; instead cache the result.

> getMouseLocation_worldspace().x => $getMouseLocation_worldspace().x$

##### addInputHandler(function _handler_(table _event_) => boolean)

Adds _handler_ to the list of input handlers. Input handlers are called every time the player presses or releases a button (mouse or keyboard) or scrolls the scroll wheel. Input handlers are also called once a frame, to represent mouse movement.

Input handlers are called in the order they were added. If _handler_ returns true, then the event is "consumed" and no more input handlers are called.

_event_ has fields _button_, _key_, _scancode_, _action_, _mods_, and _unicode_.

See https://www.glfw.org/docs/latest/input_guide.html for more information.

