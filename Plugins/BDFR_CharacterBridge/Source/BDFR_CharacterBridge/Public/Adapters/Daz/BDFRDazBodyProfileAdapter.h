#pragma once

#include "CoreMinimal.h"
#include "Body/BDFRBodyProfileTypes.h"

struct FBDFRDazBodyProfileAdapter
{
    static bool ParseBodyProfileJson(
        const FString& JsonText,
        FBDFRBodyProfile& OutProfile,
        TArray<FString>* OutWarnings = nullptr);

    static EBDFRBodyRegionType ParseRegionType(const FString& Value);
    static EBDFRBodySide ParseBodySide(const FString& Value);
};
