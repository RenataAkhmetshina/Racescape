#include "AudioSubsystem.h"
#include "SaveSubsystem.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"

void UAudioSubsystem::PlayMusic(USoundBase* Track, float FadeTime)
{
    UWorld* W = GetWorld();
    if (!W || !Track) return;
    if (Track == CurrentTrack && Current && Current->IsPlaying()) return;  

    if (Current) { Current->bAutoDestroy = true; Current->FadeOut(FadeTime, 0.f); }

    Current = UGameplayStatics::CreateSound2D(W, Track, 1.f, 1.f, 0.f, nullptr, true, false); 
    if (Current) { Current->FadeIn(FadeTime, 1.f); CurrentTrack = Track; }
}

void UAudioSubsystem::ApplyVolumes(USoundMix* Mix, USoundClass* Music, USoundClass* Sfx)
{
    UWorld* W = GetWorld();
    USaveSubsystem* S = GetGameInstance()->GetSubsystem<USaveSubsystem>();
    if (!W || !S || !Mix) return;
    UGameplayStatics::PushSoundMixModifier(W, Mix);
    UGameplayStatics::SetSoundMixClassOverride(W, Mix, Music, S->Data.MusicVolume, 1.f, 0.f, true);
    UGameplayStatics::SetSoundMixClassOverride(W, Mix, Sfx, S->Data.SfxVolume, 1.f, 0.f, true);
}