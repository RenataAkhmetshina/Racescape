#include "SaveSubsystem.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/CoreDelegates.h"

FString USaveSubsystem::SavePath() const
{
    return FPaths::ProjectSavedDir() / TEXT("save.json");
}

void USaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Load();
    FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddUObject(this, &USaveSubsystem::Save);
    FCoreDelegates::ApplicationWillDeactivateDelegate.AddUObject(this, &USaveSubsystem::Save);
}

void USaveSubsystem::Deinitialize()
{
    FCoreDelegates::ApplicationWillEnterBackgroundDelegate.RemoveAll(this);
    FCoreDelegates::ApplicationWillDeactivateDelegate.RemoveAll(this);
    Super::Deinitialize();
}

void USaveSubsystem::Sanitize()
{
    Data.UnlockedLevels = FMath::Clamp(Data.UnlockedLevels, 1, 3);
    Data.BestScore = FMath::Max(0, Data.BestScore);
    Data.MusicVolume = FMath::Clamp(Data.MusicVolume, 0.f, 1.f);
    Data.SfxVolume = FMath::Clamp(Data.SfxVolume, 0.f, 1.f);
}

void USaveSubsystem::Load()
{
    FString Text;
    FRaceSave Tmp;
    if (FFileHelper::LoadFileToString(Text, *SavePath()) &&
        FJsonObjectConverter::JsonObjectStringToUStruct(Text, &Tmp, 0, 0))
    {
        Data = Tmp;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Save missing or broken, using defaults"));
        Data = FRaceSave();   
    }
    Sanitize();
}

void USaveSubsystem::Save()
{
    Sanitize();
    FString Text;
    if (FJsonObjectConverter::UStructToJsonObjectString(Data, Text))
        FFileHelper::SaveStringToFile(Text, *SavePath());
}

void USaveSubsystem::ResetProgress()
{
    const float Music = Data.MusicVolume, Sfx = Data.SfxVolume;
    Data = FRaceSave();
    Data.MusicVolume = Music;  
    Data.SfxVolume = Sfx;
    Save();
}

void USaveSubsystem::CompleteLevel(int32 LevelIndex, int32 Score)
{
    Data.UnlockedLevels = FMath::Max(Data.UnlockedLevels, LevelIndex + 1);
    Data.BestScore = FMath::Max(Data.BestScore, Score);
    Save();
}

void USaveSubsystem::SetVolumes(float Music, float Sfx)
{
    Data.MusicVolume = Music;
    Data.SfxVolume = Sfx;
    Save();
}