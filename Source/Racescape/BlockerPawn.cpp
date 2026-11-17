#include "BlockerPawn.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "Sound/SoundBase.h"

ABlockerPawn::ABlockerPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    Box = CreateDefaultSubobject<UBoxComponent>("Box");
    RootComponent = Box;
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
    Mesh->SetupAttachment(Box);
    Movement = CreateDefaultSubobject<UFloatingPawnMovement>("Movement");

    Movement->SetPlaneConstraintEnabled(true);
    Movement->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::Z);
    Movement->bSnapToPlaneAtStart = false;

    Box->SetCanEverAffectNavigation(false);
    Mesh->SetCanEverAffectNavigation(false);

    AIControllerClass = AAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ABlockerPawn::BeginPlay()
{
    Super::BeginPlay();
    if (!Stats)
    {
        UE_LOG(LogTemp, Error, TEXT("%s: Stats asset not assigned!"), *GetName());
        SetActorTickEnabled(false);
        return;
    }
    Movement->SetPlaneConstraintOrigin(GetActorLocation());
    Movement->MaxSpeed = Stats->CruiseSpeed;
    GetWorldTimerManager().SetTimer(ThinkTimer, this, &ABlockerPawn::Think, Stats->ThinkInterval, true);
}

bool ABlockerPawn::ShouldIntercept() const
{
    if (!Stats) return false;
    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Player || !BlockPoint) return false;

    if (FVector::Dist2D(GetActorLocation(), Player->GetActorLocation()) < Stats->ProximityTriggerDistance)
        return true;

    const FVector ToJunction = BlockPoint->GetActorLocation() - Player->GetActorLocation();
    if (ToJunction.Size2D() > Stats->TriggerDistance) return false;

    const FVector Vel = Player->GetVelocity();
    if (Vel.Size2D() < Stats->MinPlayerSpeed) return false;

    FVector VelDir = Vel; VelDir.Z = 0;
    FVector JunctionDir = ToJunction; JunctionDir.Z = 0;
    if (FVector::DotProduct(VelDir.GetSafeNormal(), JunctionDir.GetSafeNormal()) < Stats->ApproachDot)
        return false;

    if (Stats->bRequireLineOfSight)
    {
        FHitResult Hit;
        FCollisionQueryParams Params; Params.AddIgnoredActor(this);
        const FVector Up(0, 0, Stats->EyeHeightOffset);
        GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation() + Up,
            Player->GetActorLocation() + Up, ECC_Visibility, Params);
        if (Hit.GetActor() != Player) return false;
    }

    return true;
}

void ABlockerPawn::Think()
{
    UE_LOG(LogTemp, Warning, TEXT("Blocker %s: state %d, controller %s, ambush %s, block %s"),
        *GetName(), (int32)State, GetController() ? TEXT("ok") : TEXT("NULL"),
        AmbushPoint ? TEXT("ok") : TEXT("NULL"), BlockPoint ? TEXT("ok") : TEXT("NULL"));
    if (!Stats) return;
    AAIController* AI = Cast<AAIController>(GetController());
    if (!AI || !AmbushPoint || !BlockPoint) return;

    switch (State)
    {
    case EBlockerState::Moving:
    {
        const float D = FVector::Dist2D(GetActorLocation(), AmbushPoint->GetActorLocation());
        if (D < Stats->ArriveRadius ||
            (D < Stats->ArriveRadius * Stats->ArriveRadiusMultiplier && AI->GetMoveStatus() == EPathFollowingStatus::Idle))
        {
            AI->StopMovement();
            State = EBlockerState::Ambush;
        }
        else if (AI->GetMoveStatus() == EPathFollowingStatus::Idle)
        {
            Movement->MaxSpeed = Stats->CruiseSpeed;
            AI->MoveToActor(AmbushPoint, Stats->PointAcceptance);
        }
        break;
    }
    case EBlockerState::Ambush:
    {
        if (bCanTrigger && ShouldIntercept())
            StartWindup();
        break;
    }
    case EBlockerState::Intercept:
    {
        if (FVector::Dist2D(GetActorLocation(), BlockPoint->GetActorLocation()) < Stats->ArriveRadius)
            StartBlocking();
        else if (AI->GetMoveStatus() == EPathFollowingStatus::Idle)
            AI->MoveToLocation(BlockPoint->GetActorLocation(), Stats->PointAcceptance);
        break;
    }
    default:
        break;
    }
}

void ABlockerPawn::StartWindup()
{
    if (!Stats) return;
    State = EBlockerState::Windup;
    bCanTrigger = false;
    Movement->MaxSpeed = 0.f;

    if (HornSound)
        UGameplayStatics::PlaySoundAtLocation(this, HornSound, GetActorLocation());

    GetWorldTimerManager().SetTimer(WindupTimer, this, &ABlockerPawn::StartIntercept, Stats->WindupTime, false);
}

void ABlockerPawn::StartIntercept()
{
    if (!Stats) return;
    State = EBlockerState::Intercept;
    Movement->MaxSpeed = Stats->InterceptSpeed;
    if (AAIController* AI = Cast<AAIController>(GetController()))
        AI->MoveToLocation(BlockPoint->GetActorLocation(), Stats->PointAcceptance);
}

void ABlockerPawn::StartBlocking()
{
    if (!Stats) return;
    State = EBlockerState::Blocking;
    if (AAIController* AI = Cast<AAIController>(GetController())) AI->StopMovement();
    Movement->MaxSpeed = 0.f;
    GetWorldTimerManager().SetTimer(BlockTimer, this, &ABlockerPawn::EndBlocking, Stats->BlockDuration, false);
}

void ABlockerPawn::EndBlocking()
{
    if (!Stats) return;
    State = EBlockerState::Moving;
    Movement->MaxSpeed = Stats->CruiseSpeed;
    GetWorldTimerManager().SetTimer(CooldownTimer,
        FTimerDelegate::CreateLambda([this] { bCanTrigger = true; }), Stats->CooldownTime, false);
}

void ABlockerPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!Stats) return;

    if (State == EBlockerState::Blocking && BlockPoint)
    {
        SetActorRotation(FMath::RInterpTo(GetActorRotation(),
            BlockPoint->GetActorRotation(), DeltaTime, Stats->BlockTurnSpeed));
    }
    else
    {
        const FVector Vel = GetVelocity();
        if (Vel.Size() > Stats->MinTurnSpeed)
            SetActorRotation(FMath::RInterpTo(GetActorRotation(), Vel.Rotation(), DeltaTime, Stats->TurnInterpSpeed));
    }
}