NewCity LUA API - Camera

##### getCameraTarget() => table

Returns the camera's target, or what the camera is looking at. The return value is a table with three fields, _x_, _y_, and _z_. _z_ is the vertical direction, or the direction of gravity.

Note that camera motion is smoothed. getCameraTarget() returns the unsmoothed target.

##### setCameraTarget(table _target_)

Sets the camera's target, or what the camera is looking at. _target_ should have three fields, _x_, _y_, and _z_. _z_ is the vertical direction, or the direction of gravity.

Note that camera motion is smoothed, including when setCameraTarget is called. To disable smoothing, set CCameraLag = 0.

The game's default camera controls can be disabled with CEnableCameraMovement = false.

##### setCameraYaw(number _yaw_)

Sets the camera's yaw, or the camera's angle in the horizontal plane.

##### setCameraPitch(number _pitch_)

Sets the camera's pitch, or the camera's angle in the vertical plane.

##### setCameraRoll(number _pitch_)

Sets the camera's roll, or the camera image rotation.

##### setCameraZoom(number _distance_)

Sets the camera's zoom. _distance_ is the distance, in meters, from the camera to the camera's target, which defines it's zoom.

##### getFrameDuration() => number

Returns the duration of the last frame, as a fraction of a second.

> getFrameDuration() => $getFrameDuration()$

