#include "Utilities.h"
#include "Serialization.h"

//Keyword logic from powerof3's CommonLibSSE implementation
bool Utilities::Keywords::AddKeyword(RE::TESForm* form, RE::BGSKeyword* newKeyword)
{
	const auto keywordForm = form->As<RE::BGSKeywordForm>();
	if (!keywordForm) {
		return false;
	}

	//Try and find keyword in existing keyword array
	//If already exists return false
	if (keywordForm->keywords) {
		for (uint32_t i = 0; i < keywordForm->numKeywords; i++) {
			if (keywordForm->keywords[i] == newKeyword) {
				return false;
			}
		}
	}

	//Create new keywords array for keywordForm with added keyword
	//Copy existing keywords to vector
	std::vector<RE::BGSKeyword*> newKeywordsTemp(keywordForm->keywords, keywordForm->keywords + keywordForm->numKeywords);
	//Push back new keyword
	newKeywordsTemp.push_back(newKeyword);

	auto oldData = keywordForm->keywords;

	//Now copy newly formed vector back to form (need to explicitly allocate so memory stays when falls out of scope
	auto newKeywordsAlloc = RE::calloc<RE::BGSKeyword*>(newKeywordsTemp.size());
	std::copy(newKeywordsTemp.begin(), newKeywordsTemp.end(), newKeywordsAlloc);
	keywordForm->keywords = newKeywordsAlloc;
	keywordForm->numKeywords = static_cast<uint32_t>(newKeywordsTemp.size());

	//Free up old keyword data
	RE::free(oldData);

	//Add keyword to saved keyword distribution
	Serialization::ArmorKeywordData::GetSingleton()->AppendData(form->formID, newKeyword->formID);

	return true;
}

bool Utilities::Keywords::RemoveKeyword(RE::TESForm* form, RE::BGSKeyword* keyword)
{
	const auto keywordForm = form->As<RE::BGSKeywordForm>();
	if (!keywordForm) {
		return false;
	}

	//Try and find keyword in existing keyword array
	//If doesnt exist return true (As indicates that keyword is removed)
	int keywordIndex = -1;
	if (keywordForm->keywords) {
		for (uint32_t i = 0; i < keywordForm->numKeywords; i++) {
			if (keywordForm->keywords[i] == keyword) {
				keywordIndex = i;
			}
		}
	}

	//Keyword not found in keywords array, so indicates keyword is removed
	if (keywordIndex == -1) {
		return true;
	}

	//Create new keywords array for keywordForm with removed keyword

	std::vector<RE::BGSKeyword*> keywordsTemp(keywordForm->keywords, keywordForm->keywords + keywordForm->numKeywords);
	//Push back new keyword
	keywordsTemp.erase(keywordsTemp.begin() + keywordIndex);

	auto oldData = keywordForm->keywords;

	//Now copy newly formed vector back to form (need to explicitly allocate so memory stays when falls out of scope
	auto newKeywordsAlloc = RE::calloc<RE::BGSKeyword*>(keywordsTemp.size());
	std::copy(keywordsTemp.begin(), keywordsTemp.end(), newKeywordsAlloc);
	keywordForm->keywords = newKeywordsAlloc;
	keywordForm->numKeywords = static_cast<uint32_t>(keywordsTemp.size());

	//Free up old keyword data
	RE::free(oldData);

	//Remove keyword from saved keyword distribution if exists
	Serialization::ArmorKeywordData::GetSingleton()->RemoveData(form->formID, keyword->formID);

	return true;
}

void Utilities::Keywords::DistributeKeywords()
{
	const auto keywordData = Serialization::ArmorKeywordData::GetSingleton()->GetData();

	logger::info("Trying To Distribute {} Keywords", keywordData.size());

	std::map<RE::FormID, RE::BGSKeyword*> subFormCache;

	for (const auto& [formId, subForms] : keywordData) {
		logger::info("Trying to find formid {:X}", formId);
		auto form = RE::TESForm::LookupByID(formId);
		if (!form) {
			logger::info("Form is null");
			continue;
		}

		for (const auto subformId : subForms) {
			auto keywordSubForm = subFormCache[subformId];
			if (!keywordSubForm) {
				auto subForm = RE::TESForm::LookupByID(subformId);
				if (!subForm) {
					continue;
				}

				keywordSubForm = subForm->As<RE::BGSKeyword>();
				if (!keywordSubForm) {
					continue;
				}

				subFormCache[subformId] = keywordSubForm;
			}
	
			AddKeyword(form, keywordSubForm);
		}
	}
}
