#include "PolicePawn.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"

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
    Movement->SetPlaneConstraintOrigin(GetActorLocation());
    Movement->MaxSpeed = PatrolSpeed;
    GetWorldTimerManager().SetTimer(ThinkTimer, this, &APolicePawn::Think, 0.1f, true);
    UE_LOG(LogTemp, Warning, TEXT("Patrol points: %d"), PatrolPoints.Num());
}

bool APolicePawn::CanSeePlayer() const
{
    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Player) return false;

    const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
    if (ToPlayer.Size() > VisionRange) return false;

    const float Angle = FMath::RadiansToDegrees(
        FMath::Acos(FVector::DotProduct(GetActorForwardVector(), ToPlayer.GetSafeNormal())));
    if (Angle > VisionHalfAngle) return false;

    FHitResult Hit;
    FCollisionQueryParams Params; Params.AddIgnoredActor(this);
    const FVector Up(0, 0, 20);
    GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation() + Up,
        Player->GetActorLocation() + Up, ECC_Visibility, Params);
    return Hit.GetActor() == Player;
}

void APolicePawn::Think()
{
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
            Movement->MaxSpeed = ChaseSpeed;
        }

        if (bCanAttack && FVector::Dist(GetActorLocation(), Player->GetActorLocation()) <= AttackRange)
            StartRam();
        else
            AI->MoveToActor(Player, 150.f);
    }
    else if (State == EPoliceState::Chase)
    {
        State = EPoliceState::Search;
        AI->MoveToLocation(LastKnownLocation, 150.f);
        GetWorldTimerManager().SetTimer(SearchTimer, this, &APolicePawn::ReturnToPatrol, SearchTime, false);
    }
    else if (State == EPoliceState::Patrol)
    {
        if (AI->GetMoveStatus() == EPathFollowingStatus::Idle)
            MoveToNextPoint();
    }
}

void APolicePawn::MoveToNextPoint()
{
    AAIController* AI = Cast<AAIController>(GetController());
    if (!AI || PatrolPoints.Num() == 0) return;
    AI->MoveToActor(PatrolPoints[PatrolIndex], 150.f);
    PatrolIndex = (PatrolIndex + 1) % PatrolPoints.Num();
}

void APolicePawn::ReturnToPatrol()
{
    if (State != EPoliceState::Search) return;
    State = EPoliceState::Patrol;
    Movement->MaxSpeed = PatrolSpeed;
    MoveToNextPoint();
}

void APolicePawn::StartRam()
{
    State = EPoliceState::RamAttack;
    bCanAttack = false;
    if (AAIController* AI = Cast<AAIController>(GetController())) AI->StopMovement();
    Movement->MaxSpeed = 0.f;
    GetWorldTimerManager().SetTimer(RamTimer, this, &APolicePawn::DoRam, WindupTime, false);
}

void APolicePawn::DoRam()
{
    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    FVector Dir = Player ? Player->GetActorLocation() - GetActorLocation() : GetActorForwardVector();
    Dir.Z = 0;
    RamDirection = Dir.GetSafeNormal();
    bIsRamming = true;
    bHitThisRam = false;
    Movement->MaxSpeed = RamSpeed;
    GetWorldTimerManager().SetTimer(RamTimer, this, &APolicePawn::EndRam, RamDuration, false);
}

void APolicePawn::EndRam()
{
    bIsRamming = false;
    Movement->MaxSpeed = ChaseSpeed;
    State = EPoliceState::Search;
    GetWorldTimerManager().SetTimer(SearchTimer, this, &APolicePawn::ReturnToPatrol, SearchTime, false);
    GetWorldTimerManager().SetTimer(CooldownTimer,
        FTimerDelegate::CreateLambda([this] { bCanAttack = true; }), AttackCooldown, false);
}

void APolicePawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    const FVector Vel = GetVelocity();
    if (Vel.Size() > 10.f)
    {
        SetActorRotation(FMath::RInterpTo(GetActorRotation(), Vel.Rotation(), DeltaTime, 8.f));
    }

    if (bIsRamming)
    {
        AddMovementInput(RamDirection, 1.f);

        APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
        if (Player && !bHitThisRam &&
            FVector::Dist(GetActorLocation(), Player->GetActorLocation()) < RamHitDistance)
        {
            bHitThisRam = true;
            UGameplayStatics::ApplyDamage(Player, RamDamage, GetController(), this, nullptr);
            GetWorldTimerManager().SetTimer(RamTimer, this, &APolicePawn::EndRam, 0.01f, false);
        }
    }
}