Scriptname PVEMainQuestScript extends ReferenceAlias  
; This script (and it's quest) is only here to manage the voice type. It does nothing else. Really.
; For some reason, i cannot get this to work properly in C++, so here we go... Fairly small though ._.

VoiceType Property PVEVoiceTypeMale Auto
VoiceType Property PVEVoiceTypeFemale Auto

Event OnInit()
    SetVoiceType()
EndEvent

Event OnPlayerLoadGame()
    SetVoiceType()
EndEvent

Event OnRaceSwitchComplete()
    SetVoiceType()
EndEvent

Function SetVoiceType()
    Int sex = Self.GetActorRef().GetActorBase().GetSex()
    If sex == 0
        Self.GetActorRef().GetRace().SetDefaultVoiceType(false, PVEVoiceTypeMale)
        LNTC_PVESKSEFunctions.Log("Debug", "Changed Voice Type to PVEVoiceTypeMale")
    ElseIf sex == 1
        Self.GetActorRef().GetRace().SetDefaultVoiceType(true, PVEVoiceTypeFemale)
        LNTC_PVESKSEFunctions.Log("Debug", "Changed Voice Type to PVEVoiceTypeFemale")
    EndIf
EndFunction