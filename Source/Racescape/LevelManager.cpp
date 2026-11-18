#include "LevelManager.h"
#include "SaveSubsystem.h"
#include "Engine/TargetPoint.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

ALevelManager::ALevelManager() { PrimaryActorTick.bCanEverTick = false; }

ULevelData* ALevelManager::GetData() const
{
    USaveSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<USaveSubsystem>() : nullptr;
    return (Save && Save->SelectedLevel) ? Save->SelectedLevel.Get() : FallbackLevel.Get();
}

void ALevelManager::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    if (!GetWorld() || !GetWorld()->IsGameWorld()) return;
    if (USaveSubsystem* Save = GetGameInstance()->GetSubsystem<USaveSubsystem>())
        if (!Save->SelectedLevel) Save->SelectedLevel = FallbackLevel;
}

void ALevelManager::BeginPlay()
{
    Super::BeginPlay();
    ULevelData* Data = GetData();
    if (!Data) { UE_LOG(LogTemp, Error, TEXT("LevelManager: no LevelData")); return; }

    UWorld* W = GetWorld();
    if (APawn* P = UGameplayStatics::GetPlayerPawn(this, 0))
        P->SetActorTransform(Data->PlayerStart, false, nullptr, ETeleportType::TeleportPhysics);

    for (const FTransform& T : Data->CoinSpawns)
        if (CoinClass) W->SpawnActor<AActor>(CoinClass, T);

    for (const FPoliceSpawn& S : Data->PoliceSpawns)
    {
        if (!PoliceClass) break;
        APolicePawn* Pol = W->SpawnActorDeferred<APolicePawn>(PoliceClass, S.Transform, nullptr, nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if (!Pol) continue;
        if (S.Stats) Pol->Stats = S.Stats;
        for (const FVector& L : S.PatrolPoints)
            Pol->PatrolPoints.Add(W->SpawnActor<ATargetPoint>(ATargetPoint::StaticClass(), FTransform(L)));
        UGameplayStatics::FinishSpawningActor(Pol, S.Transform);
    }

    for (const FBlockerSpawn& S : Data->BlockerSpawns)
    {
        if (!BlockerClass) break;
        ABlockerPawn* B = W->SpawnActorDeferred<ABlockerPawn>(BlockerClass, S.Transform, nullptr, nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if (!B) continue;
        if (S.Stats) B->Stats = S.Stats;
        B->AmbushPoint = W->SpawnActor<ATargetPoint>(ATargetPoint::StaticClass(), FTransform(S.AmbushLocation));
        B->BlockPoint = W->SpawnActor<ATargetPoint>(ATargetPoint::StaticClass(), FTransform(S.BlockLocation));
        UGameplayStatics::FinishSpawningActor(B, S.Transform);
    }

    if (Data->TimeLimit > 0.f)
        GetWorldTimerManager().SetTimer(TimeTimer, this, &ALevelManager::TimeUp, Data->TimeLimit, false);
}

void ALevelManager::TimeUp()
{
    UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}

void ALevelManager::CaptureFromLevel()
{
#if WITH_EDITOR
    if (!FallbackLevel || !GetWorld()) return;
    FallbackLevel->Modify();
    FallbackLevel->CoinSpawns.Reset(); FallbackLevel->PoliceSpawns.Reset(); FallbackLevel->BlockerSpawns.Reset();

    if (CoinClass)
        for (TActorIterator<AActor> It(GetWorld(), CoinClass); It; ++It)
            FallbackLevel->CoinSpawns.Add(It->GetActorTransform());

    for (TActorIterator<APolicePawn> It(GetWorld()); It; ++It)
    {
        FPoliceSpawn S; S.Transform = It->GetActorTransform(); S.Stats = It->Stats;
        for (AActor* P : It->PatrolPoints) if (P) S.PatrolPoints.Add(P->GetActorLocation());
        FallbackLevel->PoliceSpawns.Add(S);
    }
    for (TActorIterator<ABlockerPawn> It(GetWorld()); It; ++It)
    {
        FBlockerSpawn S; S.Transform = It->GetActorTransform(); S.Stats = It->Stats;
        if (It->AmbushPoint) S.AmbushLocation = It->AmbushPoint->GetActorLocation();
        if (It->BlockPoint) S.BlockLocation = It->BlockPoint->GetActorLocation();
        FallbackLevel->BlockerSpawns.Add(S);
    }
    FallbackLevel->MarkPackageDirty();
#endif
}