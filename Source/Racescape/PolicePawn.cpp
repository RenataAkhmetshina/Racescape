#include "PolicePawn.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "Sound/SoundBase.h"

APolicePawn::APolicePawn()
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

void APolicePawn::BeginPlay()
{
    Super::BeginPlay();
    if (!Stats)
    {
        UE_LOG(LogTemp, Error, TEXT("%s: Stats asset not assigned!"), *GetName());
        SetActorTickEnabled(false);
        return;
    }
    Movement->SetPlaneConstraintOrigin(GetActorLocation());
    Movement->MaxSpeed = Stats->PatrolSpeed;
    GetWorldTimerManager().SetTimer(ThinkTimer, this, &APolicePawn::Think, Stats->ThinkInterval, true);
}

bool APolicePawn::CanSeePlayer() const
{
    if (!Stats) return false;
    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Player) return false;

    const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
    if (ToPlayer.Size() > Stats->VisionRange) return false;

    const float Angle = FMath::RadiansToDegrees(
        FMath::Acos(FVector::DotProduct(GetActorForwardVector(), ToPlayer.GetSafeNormal())));
    if (Angle > Stats->VisionHalfAngle) return false;

    FHitResult Hit;
    FCollisionQueryParams Params; Params.AddIgnoredActor(this);
    const FVector Up(0, 0, Stats->EyeHeightOffset);
    GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation() + Up,
        Player->GetActorLocation() + Up, ECC_Visibility, Params);
    return Hit.GetActor() == Player;
}

void APolicePawn::Think()
{
    if (!Stats) return;
    if (State == EPoliceState::RamAttack) return;
    AAIController* AI = Cast<AAIController>(GetController());
    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!AI || !Player) return;

    if (CanSeePlayer())
    {
        LastKnownLocation = Player->GetActorLocation();
        GetWorldTimerManager().ClearTimer(SearchTimer);

        if (State != EPoliceState::Chase)
        {
            State = EPoliceState::Chase;
            Movement->MaxSpeed = Stats->ChaseSpeed;
        }

        if (bCanAttack && FVector::Dist(GetActorLocation(), Player->GetActorLocation()) <= Stats->AttackRange)
            StartRam();
        else
            AI->MoveToActor(Player, Stats->AcceptanceRadius);
    }
    else if (State == EPoliceState::Chase)
    {
        State = EPoliceState::Search;
        AI->MoveToLocation(LastKnownLocation, Stats->AcceptanceRadius);
        GetWorldTimerManager().SetTimer(SearchTimer, this, &APolicePawn::ReturnToPatrol, Stats->SearchTime, false);
    }
    else if (State == EPoliceState::Patrol)
    {
        if (AI->GetMoveStatus() == EPathFollowingStatus::Idle)
            MoveToNextPoint();
    }
}

void APolicePawn::MoveToNextPoint()
{
    if (!Stats) return;
    AAIController* AI = Cast<AAIController>(GetController());
    if (!AI || PatrolPoints.Num() == 0) return;
    AI->MoveToActor(PatrolPoints[PatrolIndex], Stats->AcceptanceRadius);
    PatrolIndex = (PatrolIndex + 1) % PatrolPoints.Num();
}

void APolicePawn::ReturnToPatrol()
{
    if (!Stats) return;
    if (State != EPoliceState::Search) return;
    State = EPoliceState::Patrol;
    Movement->MaxSpeed = Stats->PatrolSpeed;
    MoveToNextPoint();
}

void APolicePawn::StartRam()
{
    if (!Stats) return;
    State = EPoliceState::RamAttack;
    bCanAttack = false;
    if (AAIController* AI = Cast<AAIController>(GetController())) AI->StopMovement();
    Movement->MaxSpeed = 0.f;

    if (HornSound)
        UGameplayStatics::PlaySoundAtLocation(this, HornSound, GetActorLocation());

    GetWorldTimerManager().SetTimer(RamTimer, this, &APolicePawn::DoRam, Stats->WindupTime, false);
}

void APolicePawn::DoRam()
{
    if (!Stats) return;
    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    FVector Dir = Player ? Player->GetActorLocation() - GetActorLocation() : GetActorForwardVector();
    Dir.Z = 0;
    RamDirection = Dir.GetSafeNormal();
    bIsRamming = true;
    bHitThisRam = false;
    Movement->MaxSpeed = Stats->RamSpeed;
    GetWorldTimerManager().SetTimer(RamTimer, this, &APolicePawn::EndRam, Stats->RamDuration, false);
}

void APolicePawn::EndRam()
{
    if (!Stats) return;
    bIsRamming = false;
    Movement->MaxSpeed = Stats->ChaseSpeed;
    State = EPoliceState::Search;
    GetWorldTimerManager().SetTimer(SearchTimer, this, &APolicePawn::ReturnToPatrol, Stats->SearchTime, false);
    GetWorldTimerManager().SetTimer(CooldownTimer,
        FTimerDelegate::CreateLambda([this] { bCanAttack = true; }), Stats->AttackCooldown, false);
}

void APolicePawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!Stats) return;

    const FVector Vel = GetVelocity();
    if (Vel.Size() > Stats->MinTurnSpeed)
    {
        SetActorRotation(FMath::RInterpTo(GetActorRotation(), Vel.Rotation(), DeltaTime, Stats->TurnInterpSpeed));
    }

    if (bIsRamming)
    {
        AddMovementInput(RamDirection, 1.f);

        APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
        if (Player && !bHitThisRam &&
            FVector::Dist(GetActorLocation(), Player->GetActorLocation()) < Stats->RamHitDistance)
        {
            bHitThisRam = true;
            UGameplayStatics::ApplyDamage(Player, Stats->RamDamage, GetController(), this, nullptr);
            GetWorldTimerManager().SetTimer(RamTimer, this, &APolicePawn::EndRam, Stats->RamEndDelay, false);
        }
    }
}