#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PoliceStatsData.h"
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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Balance")
    TObjectPtr<UPoliceStatsData> Stats;

    UPROPERTY(EditAnywhere, Category = "Audio") class USoundBase* HornSound = nullptr;

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