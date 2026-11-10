#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PolicePawn.generated.h"

UENUM(BlueprintType)
enum class EPoliceState : uint8 { Patrol, Chase, Search, RamAttack };

UCLASS()
class APolicePawn : public APawn
{
    GENERATED_BODY()
public:
    APolicePawn();
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere) class UBoxComponent* Box;
    UPROPERTY(VisibleAnywhere) class UStaticMeshComponent* Mesh;
    UPROPERTY(VisibleAnywhere) class UFloatingPawnMovement* Movement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<AActor*> PatrolPoints;
    UPROPERTY(EditAnywhere) float PatrolSpeed = 400.f;
    UPROPERTY(EditAnywhere) float ChaseSpeed = 800.f;
    UPROPERTY(EditAnywhere) float RamSpeed = 1800.f;
    UPROPERTY(EditAnywhere) float VisionRange = 2500.f;
    UPROPERTY(EditAnywhere) float VisionHalfAngle = 60.f;
    UPROPERTY(EditAnywhere) float SearchTime = 5.f;
    UPROPERTY(EditAnywhere) float AttackRange = 500.f;
    UPROPERTY(EditAnywhere) float WindupTime = 0.8f;
    UPROPERTY(EditAnywhere) float RamDuration = 0.6f;
    UPROPERTY(EditAnywhere) float AttackCooldown = 4.f;
    UPROPERTY(EditAnywhere) float RamDamage = 1.f;
    UPROPERTY(EditAnywhere) float RamHitDistance = 450.f;   // новое

    UPROPERTY(BlueprintReadOnly) EPoliceState State = EPoliceState::Patrol;

protected:
    virtual void BeginPlay() override;

private:
    void Think();
    bool CanSeePlayer() const;
    void MoveToNextPoint();
    void StartRam();
    void DoRam();
    void EndRam();
    void ReturnToPatrol();

    int32 PatrolIndex = 0;
    FVector LastKnownLocation;
    FVector RamDirection;
    bool bCanAttack = true;
    bool bIsRamming = false;
    bool bHitThisRam = false;
    FTimerHandle ThinkTimer, SearchTimer, RamTimer, CooldownTimer;
};