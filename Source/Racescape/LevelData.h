#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PoliceStatsData.h"
#include "BlockerStatsData.h"
#include "LevelData.generated.h"

USTRUCT(BlueprintType)
struct FPoliceSpawn
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FTransform Transform;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FVector> PatrolPoints;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<UPoliceStatsData> Stats;
};

USTRUCT(BlueprintType)
struct FBlockerSpawn
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FTransform Transform;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector AmbushLocation = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector BlockLocation = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<UBlockerStatsData> Stats;
};

UCLASS(BlueprintType)
class ULevelData : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText LevelName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 LevelIndex = 0;  
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 GoalCoins = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float TimeLimit = 0.f; 
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FTransform PlayerStart;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FTransform> CoinSpawns;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FPoliceSpawn> PoliceSpawns;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FBlockerSpawn> BlockerSpawns;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TObjectPtr<ULevelData> NextLevel;
};