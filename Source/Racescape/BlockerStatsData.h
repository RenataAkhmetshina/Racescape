#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BlockerStatsData.generated.h"

UCLASS(BlueprintType)
class UBlockerStatsData : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Speed") float CruiseSpeed = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Speed") float InterceptSpeed = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Speed") float TurnInterpSpeed = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Speed") float BlockTurnSpeed = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Speed") float MinTurnSpeed = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger") float TriggerDistance = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger") float ProximityTriggerDistance = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger") float ApproachDot = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger") float MinPlayerSpeed = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger") bool bRequireLineOfSight = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger") float EyeHeightOffset = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing") float WindupTime = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing") float BlockDuration = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing") float CooldownTime = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing") float ThinkInterval = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI") float ArriveRadius = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI") float ArriveRadiusMultiplier = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI") float PointAcceptance = 0.f;
};