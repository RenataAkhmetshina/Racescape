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
    Movement->SetPlaneConstraintOrigin(GetActorLocation());
    Movement->MaxSpeed = CruiseSpeed;
    GetWorldTimerManager().SetTimer(ThinkTimer, this, &ABlockerPawn::Think, 0.1f, true);
}

bool ABlockerPawn::ShouldIntercept() const
{
    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Player || !BlockPoint) return false;

    if (FVector::Dist2D(GetActorLocation(), Player->GetActorLocation()) < ProximityTriggerDistance)
        return true;

    const FVector ToJunction = BlockPoint->GetActorLocation() - Player->GetActorLocation();
    if (ToJunction.Size2D() > TriggerDistance) return false;

    const FVector Vel = Player->GetVelocity();
    if (Vel.Size2D() < MinPlayerSpeed) return false;

    FVector VelDir = Vel; VelDir.Z = 0;
    FVector JunctionDir = ToJunction; JunctionDir.Z = 0;
    if (FVector::DotProduct(VelDir.GetSafeNormal(), JunctionDir.GetSafeNormal()) < ApproachDot)
        return false;

    if (bRequireLineOfSight)
    {
        FHitResult Hit;
        FCollisionQueryParams Params; Params.AddIgnoredActor(this);
        const FVector Up(0, 0, 20);
        GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation() + Up,
            Player->GetActorLocation() + Up, ECC_Visibility, Params);
        if (Hit.GetActor() != Player) return false;
    }

    return true;
}

void ABlockerPawn::Think()
{
    AAIController* AI = Cast<AAIController>(GetController());
    if (!AI || !AmbushPoint || !BlockPoint) return;

    switch (State)
    {
    case EBlockerState::Moving:
    {
        const float D = FVector::Dist2D(GetActorLocation(), AmbushPoint->GetActorLocation());
        if (D < ArriveRadius || (D < ArriveRadius * 2.f && AI->GetMoveStatus() == EPathFollowingStatus::Idle))
        {
            AI->StopMovement();
            State = EBlockerState::Ambush;
        }
        else if (AI->GetMoveStatus() == EPathFollowingStatus::Idle)
        {
            Movement->MaxSpeed = CruiseSpeed;
            AI->MoveToActor(AmbushPoint, 50.f);
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
        if (FVector::Dist2D(GetActorLocation(), BlockPoint->GetActorLocation()) < ArriveRadius)
            StartBlocking();
        else if (AI->GetMoveStatus() == EPathFollowingStatus::Idle)
            AI->MoveToLocation(BlockPoint->GetActorLocation(), 50.f);
        break;
    }
    default:
        break;
    }
}

void ABlockerPawn::StartWindup()
{
    State = EBlockerState::Windup;
    bCanTrigger = false;
    Movement->MaxSpeed = 0.f;

    if (HornSound)
        UGameplayStatics::PlaySoundAtLocation(this, HornSound, GetActorLocation());

    GetWorldTimerManager().SetTimer(WindupTimer, this, &ABlockerPawn::StartIntercept, WindupTime, false);
}

void ABlockerPawn::StartIntercept()
{
    State = EBlockerState::Intercept;
    Movement->MaxSpeed = InterceptSpeed;
    if (AAIController* AI = Cast<AAIController>(GetController()))
        AI->MoveToLocation(BlockPoint->GetActorLocation(), 50.f);
}

void ABlockerPawn::StartBlocking()
{
    State = EBlockerState::Blocking;
    if (AAIController* AI = Cast<AAIController>(GetController())) AI->StopMovement();
    Movement->MaxSpeed = 0.f;
    GetWorldTimerManager().SetTimer(BlockTimer, this, &ABlockerPawn::EndBlocking, BlockDuration, false);
}

void ABlockerPawn::EndBlocking()
{
    State = EBlockerState::Moving;
    Movement->MaxSpeed = CruiseSpeed;
    GetWorldTimerManager().SetTimer(CooldownTimer,
        FTimerDelegate::CreateLambda([this] { bCanTrigger = true; }), CooldownTime, false);
}

void ABlockerPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (State == EBlockerState::Blocking && BlockPoint)
    {
        SetActorRotation(FMath::RInterpTo(GetActorRotation(),
            BlockPoint->GetActorRotation(), DeltaTime, 6.f));
    }
    else
    {
        const FVector Vel = GetVelocity();
        if (Vel.Size() > 10.f)
            SetActorRotation(FMath::RInterpTo(GetActorRotation(), Vel.Rotation(), DeltaTime, 8.f));
    }
}