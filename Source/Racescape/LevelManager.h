#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PolicePawn.h"
#include "BlockerPawn.h"
#include "LevelData.h"
#include "LevelManager.generated.h"

UCLASS()
class ALevelManager : public AActor
{
    GENERATED_BODY()
public:
    ALevelManager();
    virtual void PostInitializeComponents() override;

    UPROPERTY(EditAnywhere, Category = "Level") TSubclassOf<AActor> CoinClass;
    UPROPERTY(EditAnywhere, Category = "Level") TSubclassOf<APolicePawn> PoliceClass;
    UPROPERTY(EditAnywhere, Category = "Level") TSubclassOf<ABlockerPawn> BlockerClass;
    UPROPERTY(EditAnywhere, Category = "Level") TObjectPtr<ULevelData> FallbackLevel; 

    UFUNCTION(CallInEditor, Category = "Tools") void CaptureFromLevel();

protected:
    virtual void BeginPlay() override;

private:
    void TimeUp();
    ULevelData* GetData() const;
    FTimerHandle TimeTimer;
};