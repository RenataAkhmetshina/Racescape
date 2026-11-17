#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PoliceStatsData.generated.h"

UCLASS(BlueprintType)
class UPoliceStatsData : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Speed") float PatrolSpeed = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Speed") float ChaseSpeed = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Speed") float RamSpeed = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Speed") float TurnInterpSpeed = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vision") float VisionRange = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vision") float VisionHalfAngle = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vision") float EyeHeightOffset = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vision") float MinTurnSpeed = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack") float AttackRange = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack") float WindupTime = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack") float RamDuration = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack") float AttackCooldown = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack") float RamDamage = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack") float RamHitDistance = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack") float RamEndDelay = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI") float SearchTime = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI") float AcceptanceRadius = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI") float ThinkInterval = 0.f;
};