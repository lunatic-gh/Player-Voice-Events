Scriptname LNTC_PVESKSEFunctions Hidden
; This files includes a few helper-functions from the SKSE Plugin.

; Attempts to play a sound event with the given name.
; This will not necessarily play a sound, the conditions depend on the used pack's configuration.
; This is what you should call when trying to send a custom event.
Function PlaySound(String _eventName) Global Native

; Stops the currently playing sound if present.
; You should usually only call this on situations like dialogues etc.
Function StopSound() Global Native

; Logs a message with [Papyrus] as prefix into the PlayerVoiceEvents.log
Function Log(String _prefix, String _msg) Global Native

; Returns the given _text, with all occurences of _old replaced with _new
String Function Replace(String _text, String _old, String _new) Global Native

; Registers a global condition with the given value.
Function RegisterFloatCondition(String _conditionName, Float _value) Global Native
Function RegisterIntCondition(String _conditionName, Int _value) Global Native
Function RegisterBoolCondition(String _conditionName, Bool _value) Global Native
Function RegisterStringCondition(String _conditionName, String _value) Global Native

; Same as above, but these are cleared after a sound for _eventName stops (or fails to play)
; You want to call these ONLY for event-specific conditions.
Function RegisterDynamicFloatCondition(String _eventName, String _conditionName, Float _value) Global Native
Function RegisterDynamicIntCondition(String _eventName, String _conditionName, Int _value) Global Native
Function RegisterDynamicBoolCondition(String _eventName, String _conditionName, Bool _value) Global Native
Function RegisterDynamicStringCondition(String _eventName, String _conditionName, String _value) Global Native