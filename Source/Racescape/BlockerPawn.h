#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BlockerStatsData.h"
#include "BlockerPawn.generated.h"

UENUM(BlueprintType)
enum class EBlockerState : uint8 { Moving, Ambush, Windup, Intercept, Blocking };

UCLASS()
class ABlockerPawn : public APawn
{
    GENERATED_BODY()
public:
    ABlockerPawn();
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere) class UBoxComponent* Box;
    UPROPERTY(VisibleAnywhere) class UStaticMeshComponent* Mesh;
    UPROPERTY(VisibleAnywhere) class UFloatingPawnMovement* Movement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite) AActor* AmbushPoint = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) AActor* BlockPoint = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Balance")
    TObjectPtr<UBlockerStatsData> Stats;

    UPROPERTY(EditAnywhere, Category = "Audio") class USoundBase* HornSound = nullptr;

    UPROPERTY(BlueprintReadOnly) EBlockerState State = EBlockerState::Moving;

protected:
    virtual void BeginPlay() override;

private:
    void Think();
    bool ShouldIntercept() const;
    void StartWindup();
    void StartIntercept();
    void StartBlocking();
    void EndBlocking();

    bool bCanTrigger = true;
    FTimerHandle ThinkTimer, WindupTimer, BlockTimer, CooldownTimer;
};