Lua API - Events

Events are a method of firing Lua code at certain times during the execution of the program. Event handlers can be added by calling on(...) in modpacks/<your modpack>/data/scripts.lua

Certain event handlers are added by default in data/scripts.lua. If you would like to override this behavoir, set CDisableDefaultEvents = true.

---

##### on(number _event_, function _handler_)

Adds an event handler. The function _handler_ will get called every time _event_ is fired. _handler_ should be a function which takes no arguments and has no return value.

> on(GameUpdate, function() log("yo") end) => Logs "yo" every game update.

---

##### Event Load

Fires after a game is loaded.

---

##### Event Unload

Fires before a game is unloaded.

---

##### Event NewGame

Fires after a new city (or test, or building design) is created. Fires before Load.

---

##### Event Save

Fires before a game is saved, including autosaves.

---

##### Event Frame

Fires every rendered frame. NOTE: do not do performance-heavy work, or logging, every frame.

---

##### Event GameUpdate

Fires every game update. Fires after the C++ work of the game update is complete, but before rendering and UI rendering. Generally there are less game updates than frames, but some frames may have multiple game updates.

---

##### Event EconomyUpdate

Fires every economic update. Fires after the C++ work of the economic update is complete, but before statistics processing. Generally there are less economic updates than game updates.

---

##### Event StatisticsUpdate

Fires every statistics update. Statistics are updated and written to the statistics history every hour. This event fires before the statistics are written to the history.

---

##### Event Newspaper

Fires every time an issue of the newspaper is generated. Fires before the issue is generated.

---

##### Event Budget

Fires every time the budget rolls over, or every year. Fires before the new budget is created.

