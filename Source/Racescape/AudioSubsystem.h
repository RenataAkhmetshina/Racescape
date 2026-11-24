#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AudioSubsystem.generated.h"

UCLASS()
class UAudioSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable) void PlayMusic(USoundBase* Track, float FadeTime = 1.f);
    UFUNCTION(BlueprintCallable) void ApplyVolumes(USoundMix* VolumeMix, USoundClass* MusicClass, USoundClass* SfxClass);
private:
    UPROPERTY() TObjectPtr<UAudioComponent> Current;
    UPROPERTY() TObjectPtr<USoundBase> CurrentTrack;
};