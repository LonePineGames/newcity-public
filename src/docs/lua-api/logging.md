Lua API - Logging

##### log(_message_)

Logs _message_ to the in-game console.

> log("yo") => logs "yo" to the console

---

##### warn(_message_)

Logs _message_ to the in-game console, with level WARN.

> warn("yo") => logs "yo" to the console

---

##### logError(_message_)

Logs _message_ to the in-game console, with level ERROR.

> logError("yo") => logs "yo" to the console

---

##### handleError(_message_)

Displays an error prompt to alert the player of an error. Use sparingly.

Also logs _message_ to the in-game console, with level ERROR, as well as a stacktrace.

> handleError("yo") => Displays an error prompt with message "yo"

---

