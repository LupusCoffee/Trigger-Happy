#pragma once

#if WITH_EDITOR

#include "IDetailCustomization.h"

class IDetailLayoutBuilder;

class GP4PROTOTYPEEDITOR_API FSubtitleScriptDataCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
};

#endif // WITH_EDITOR

