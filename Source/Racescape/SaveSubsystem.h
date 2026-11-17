#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FRaceSave
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadWrite) int32 Version = 1;
    UPROPERTY(BlueprintReadWrite) int32 UnlockedLevels = 1;
    UPROPERTY(BlueprintReadWrite) int32 BestScore = 0;
    UPROPERTY(BlueprintReadWrite) float MusicVolume = 1.f;
    UPROPERTY(BlueprintReadWrite) float SfxVolume = 1.f;
};

UCLASS()
class USaveSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintReadOnly) FRaceSave Data;

    UFUNCTION(BlueprintCallable) void Load();
    UFUNCTION(BlueprintCallable) void Save();
    UFUNCTION(BlueprintCallable) void ResetProgress();
    UFUNCTION(BlueprintCallable) void CompleteLevel(int32 LevelIndex, int32 Score);
    UFUNCTION(BlueprintCallable) void SetVolumes(float Music, float Sfx);

private:
    FString SavePath() const;
    void Sanitize();
};