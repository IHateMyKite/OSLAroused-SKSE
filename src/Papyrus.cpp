#include "Papyrus.h"
#include "ArousalManager.h"
#include "Serialization.h"
#include "RuntimeEvents.h"
#include "Settings.h"
#include "Debug.h"

void Papyrus::UpdatePlayerNudityCheck(RE::StaticFunctionTag*, bool newVal)
{
	Settings::GetSingleton()->SetPlayerNudityCheckEnabled(newVal);
	if (newVal) {
		//starts Poller if stopped
		WorldChecks::NakedArousalTicker::GetSingleton()->Start();
		logger::trace("Starting PlayerNakedUpdateLoop...");
	} else {
		//Stops Poller if running
		WorldChecks::NakedArousalTicker::GetSingleton()->Stop();
		logger::trace("Stopping PlayerNakedUpdateLoop...");
	}
}

void Papyrus::UpdateHourlyNudityArousalModifier(RE::StaticFunctionTag*, float newVal)
{
	Settings::GetSingleton()->SetHourlyNudityArousalModifier(newVal);
}

void Papyrus::UpdateArousalMode(RE::StaticFunctionTag*, int newArousalMode)
{
	logger::trace("UpdateArousalMode: {}", newArousalMode);

	Settings::ArousalMode newMode;
	switch (newArousalMode) {
	case 0:
		newMode = Settings::ArousalMode::kSexlabAroused;
		break;
	case 1:
		newMode = Settings::ArousalMode::kOAroused;
		break;
	default:
		return;
	}

	Settings::GetSingleton()->SetArousalMode(newMode);
	logger::trace("Arousal Mode Updated...");
}

void Papyrus::UpdateDefaultArousalMultiplier(RE::StaticFunctionTag*, float newMultiplier)
{
	Settings::GetSingleton()->SetDefaultArousalMultiplier(newMultiplier);
}

float Papyrus::GetArousal(RE::StaticFunctionTag*, RE::Actor* actorRef)
{
	return ArousalManager::GetArousal(actorRef);
}

std::vector<float> Papyrus::GetArousalMultiple(RE::StaticFunctionTag*, RE::reference_array<RE::Actor*> actorRefs)
{
	std::vector<float> results;

	for (const auto actorRef : actorRefs) {
		results.push_back(ArousalManager::GetArousal(actorRef));
	}

	return results;
}

float Papyrus::GetArousalNoSideEffects(RE::StaticFunctionTag*, RE::Actor* actorRef)
{
	return ArousalManager::GetArousal(actorRef, false);
}

float Papyrus::SetArousal(RE::StaticFunctionTag*, RE::Actor* actorRef, float value)
{
	return ArousalManager::SetArousal(actorRef, value);
}

void Papyrus::SetArousalMultiple(RE::StaticFunctionTag*, RE::reference_array<RE::Actor*> actorRefs, float value)
{
	for (const auto actorRef : actorRefs) {
		ArousalManager::SetArousal(actorRef, value);
	}
}

float Papyrus::ModifyArousal(RE::StaticFunctionTag*, RE::Actor* actorRef, float value)
{
	return ArousalManager::ModifyArousal(actorRef, value);
}

void Papyrus::ModifyArousalMultiple(RE::StaticFunctionTag*, RE::reference_array<RE::Actor*> actorRefs, float value)
{
	for (const auto actorRef : actorRefs) {
		ArousalManager::ModifyArousal(actorRef, value);
	}
}

void Papyrus::SetArousalMultiplier(RE::StaticFunctionTag*, RE::Actor* actorRef, float value)
{
	//Just clamp between 0 and 100?
	value = std::clamp(value, 0.0f, 100.f);

	Serialization::MultiplierData::GetSingleton()->SetData(actorRef->formID, value);
}

float Papyrus::GetArousalMultiplier(RE::StaticFunctionTag*, RE::Actor* actorRef)
{
	return Serialization::MultiplierData::GetSingleton()->GetData(actorRef->formID, Settings::GetSingleton()->GetDefaultArousalMultiplier());
}

float Papyrus::GetExposure(RE::StaticFunctionTag*, RE::Actor* actorRef)
{
	//If we are in sla mode get exposure, otherwise just return arousal
	if (Settings::GetSingleton()->GetArousalMode() == Settings::ArousalMode::kSexlabAroused) {
		float curTime = RE::Calendar::GetSingleton()->GetCurrentGameTime();
		auto lastCheckTime = Serialization::LastCheckTimeData::GetSingleton()->GetData(actorRef->formID, 0.f);
		Serialization::LastCheckTimeData::GetSingleton()->SetData(actorRef->formID, curTime);
		return ArousalManager::GetSexlabExposure(actorRef, curTime - lastCheckTime);
	}

	return ArousalManager::GetArousal(actorRef);
}

float Papyrus::GetDaysSinceLastOrgasm(RE::StaticFunctionTag*, RE::Actor* actorRef)
{
	float lastOrgasmTime = Serialization::LastOrgasmTimeData::GetSingleton()->GetData(actorRef->formID, 0.f);
	if (lastOrgasmTime < 0) {
		lastOrgasmTime = 0;
	}

	return RE::Calendar::GetSingleton()->GetCurrentGameTime() - lastOrgasmTime;
}

void Papyrus::SetTimeRate(RE::StaticFunctionTag*, RE::Actor* actorRef, float value)
{
	if (!actorRef) {
		return;
	}
	value = std::clamp(value, 0.f, 100.f);
	Serialization::TimeRateData::GetSingleton()->SetData(actorRef->formID, value);
}

float Papyrus::GetTimeRate(RE::StaticFunctionTag*, RE::Actor* actorRef)
{
	return Serialization::TimeRateData::GetSingleton()->GetData(actorRef->formID, 10.0);
}

bool Papyrus::IsActorNaked(RE::StaticFunctionTag*, RE::Actor* actorRef)
{
	return Utilities::Actor::IsNaked(actorRef);
}

void Papyrus::SetPlayerInSexScene(RE::StaticFunctionTag*, bool bInScene)
{
	Settings::GetSingleton()->SetPlayerInSexScene(bInScene);
}

bool Papyrus::AddKeywordToForm(RE::StaticFunctionTag*, RE::TESForm* form, RE::BGSKeyword* keyword)
{
	if (!form || !keyword) {
		return false;
	}
	
	return Utilities::Keywords::AddKeyword(form, keyword);
}

bool Papyrus::RemoveKeywordFromForm(RE::StaticFunctionTag*, RE::TESForm* form, RE::BGSKeyword* keyword)
{
	if (!form || !keyword) {
		return false;
	}

	return Utilities::Keywords::RemoveKeyword(form, keyword);
}

void Papyrus::DumpArousalData(RE::StaticFunctionTag*)
{
	Debug::DumpAllArousalData();
}

void Papyrus::ClearSecondaryArousalData(RE::StaticFunctionTag*)
{
	Debug::ClearSecondaryArousalData();
}

void Papyrus::ClearAllArousalData(RE::StaticFunctionTag*)
{
	Debug::ClearAllArousalData();
}

bool Papyrus::RegisterFunctions(RE::BSScript::IVirtualMachine* vm)
{
	//OSLAroused Settings
	vm->RegisterFunction("UpdatePlayerNudityCheck", "OSLArousedNative", UpdatePlayerNudityCheck);
	vm->RegisterFunction("UpdateHourlyNudityArousalModifier", "OSLArousedNative", UpdateHourlyNudityArousalModifier);
	vm->RegisterFunction("UpdateArousalMode", "OSLArousedNative", UpdateArousalMode);
	vm->RegisterFunction("UpdateDefaultArousalMultiplier", "OSLArousedNative", UpdateDefaultArousalMultiplier);
	
	//General State
	vm->RegisterFunction("GetArousal", "OSLArousedNative", GetArousal);
	vm->RegisterFunction("GetArousalMultiple", "OSLArousedNative", GetArousalMultiple);
	vm->RegisterFunction("GetArousalNoSideEffects", "OSLArousedNative", GetArousalNoSideEffects);

	vm->RegisterFunction("SetArousal", "OSLArousedNative", SetArousal);
	vm->RegisterFunction("SetArousalMultiple", "OSLArousedNative", SetArousalMultiple);
	vm->RegisterFunction("ModifyArousal", "OSLArousedNative", ModifyArousal);
	vm->RegisterFunction("ModifyArousalMultiple", "OSLArousedNative", ModifyArousalMultiple);

	vm->RegisterFunction("SetArousalMultiplier", "OSLArousedNative", SetArousalMultiplier);
	vm->RegisterFunction("GetArousalMultiplier", "OSLArousedNative", GetArousalMultiplier);

	vm->RegisterFunction("GetExposure", "OSLArousedNative", GetExposure);
	vm->RegisterFunction("GetDaysSinceLastOrgasm", "OSLArousedNative", GetDaysSinceLastOrgasm);

	vm->RegisterFunction("SetTimeRate", "OSLArousedNative", SetTimeRate);
	vm->RegisterFunction("GetTimeRate", "OSLArousedNative", GetTimeRate);

	//Actor State
	vm->RegisterFunction("IsActorNaked", "OSLArousedNative", IsActorNaked);
	vm->RegisterFunction("SetPlayerInSexScene", "OSLArousedNative", SetPlayerInSexScene);

	//Keyword
	vm->RegisterFunction("AddKeywordToForm", "OSLArousedNative", AddKeywordToForm);
	vm->RegisterFunction("RemoveKeywordFromForm", "OSLArousedNative", RemoveKeywordFromForm);

	//Debug
	vm->RegisterFunction("DumpArousalData", "OSLArousedNative", DumpArousalData);
	vm->RegisterFunction("ClearSecondaryArousalData", "OSLArousedNative", ClearSecondaryArousalData);
	vm->RegisterFunction("ClearAllArousalData", "OSLArousedNative", ClearAllArousalData);

	return true;
}

void SendModEvent(RE::BSFixedString eventName, float numArg, RE::TESForm* sender)
{
	SKSE::ModCallbackEvent modEvent{
		eventName,
		RE::BSFixedString(),
		numArg,
		sender
	};

	auto modCallback = SKSE::GetModCallbackEventSource();
	modCallback->SendEvent(&modEvent);
}

void Papyrus::Events::SendPlayerArousalUpdatedEvent(float newVal)
{
	SendModEvent("OSLA_PlayerArousalUpdated", newVal, nullptr);
}

void Papyrus::Events::SendActorNakedUpdatedEvent(RE::Actor* actorRef, bool newNaked)
{
	SendModEvent("OSLA_ActorNakedUpdated", newNaked ? 1.f : 0.f, actorRef);
}
