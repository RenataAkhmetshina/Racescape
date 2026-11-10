#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
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

    UPROPERTY(EditAnywhere) float CruiseSpeed = 600.f;
    UPROPERTY(EditAnywhere) float InterceptSpeed = 1500.f;
    UPROPERTY(EditAnywhere) float TriggerDistance = 3000.f;
    UPROPERTY(EditAnywhere) float ApproachDot = 0.5f;
    UPROPERTY(EditAnywhere) float MinPlayerSpeed = 100.f;
    UPROPERTY(EditAnywhere) bool bRequireLineOfSight = true;
    UPROPERTY(EditAnywhere) float WindupTime = 0.5f;
    UPROPERTY(EditAnywhere) float BlockDuration = 5.f;
    UPROPERTY(EditAnywhere) float CooldownTime = 6.f;
    UPROPERTY(EditAnywhere) float ArriveRadius = 150.f;
    UPROPERTY(EditAnywhere) float ProximityTriggerDistance = 1500.f;

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