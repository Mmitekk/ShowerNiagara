#include "ShowerActor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundAttenuation.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogShowerFX, Log, All);

AShowerActor::AShowerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Lazy asset defaults (resolved in OnConstruction so the plugin works in any project
	// that keeps the /Game/VFX/Shower folder).
	WaterSystemPath = TEXT("/Game/VFX/Shower/NS_ShowerWater.NS_ShowerWater");
	SplashSystemPath = TEXT("/Game/VFX/Shower/NS_ShowerSplash.NS_ShowerSplash");
	SteamSystemPath = TEXT("/Game/VFX/Shower/NS_ShowerSteam.NS_ShowerSteam");
	PipeMeshPath = TEXT("/Game/VFX/Shower/Meshes/SM_ShowerCylinder.SM_ShowerCylinder");
	ElbowMeshPath = TEXT("/Game/VFX/Shower/Meshes/SM_ShowerSphere.SM_ShowerSphere");
	BrassMaterialPath = TEXT("/Game/VFX/Shower/M_Brass.M_Brass");
	WaterMaterialPath = TEXT("/Game/VFX/Shower/M_WaterDrop_Inst.M_WaterDrop_Inst");
	WaterSoundPath = TEXT("/Game/VFX/Shower/S_WaterLoop.S_WaterLoop");
	SteamMaterialPath = TEXT("/Game/VFX/Shower/M_Steam_Inst.M_Steam_Inst");
}

void AShowerActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Lazy defaults
	if (!WaterSystem)   WaterSystem   = LoadObject<UNiagaraSystem>(nullptr, *WaterSystemPath);
	if (!SplashSystem)  SplashSystem  = LoadObject<UNiagaraSystem>(nullptr, *SplashSystemPath);
	if (!SteamSystem)   SteamSystem   = LoadObject<UNiagaraSystem>(nullptr, *SteamSystemPath);
	if (!PipeMesh)      PipeMesh      = LoadObject<UStaticMesh>(nullptr, *PipeMeshPath);
	if (!ElbowMesh)     ElbowMesh     = LoadObject<UStaticMesh>(nullptr, *ElbowMeshPath);
	if (!BrassMaterial) BrassMaterial = LoadObject<UMaterialInterface>(nullptr, *BrassMaterialPath);
	if (!WaterMaterial) WaterMaterial = LoadObject<UMaterialInterface>(nullptr, *WaterMaterialPath);
	if (!SteamMaterial) SteamMaterial = LoadObject<UMaterialInterface>(nullptr, *SteamMaterialPath);
	if (!WaterSound)   WaterSound   = LoadObject<USoundBase>(nullptr, *WaterSoundPath);

	BuildComponents();
	ApplyParams();
}

UNiagaraComponent* AShowerActor::MakeFXComponent(FName Name, UNiagaraSystem* System, const FVector& Loc, bool bVisible)
{
	UNiagaraComponent* C = NewObject<UNiagaraComponent>(this, Name);
	C->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	C->SetupAttachment(GetRootComponent());
	C->SetRelativeLocation(Loc);
	C->SetAsset(System, false);
	C->SetVisibility(bVisible, true);
	C->bAutoActivate = true;
	C->RegisterComponent();
	return C;
}

void AShowerActor::BuildComponents()
{
	// UserConstruction components are destroyed automatically before each construction
	// rerun, so we just recreate everything.

	struct FPart { const TCHAR* Name; UStaticMesh* Mesh; FVector Loc; FRotator Rot; FVector Scale; float RadiusScale = 1.f; };
	if (!PipeMesh || !ElbowMesh)
	{
		UE_LOG(LogShowerFX, Warning, TEXT("PipeMesh/ElbowMesh missing; geometry skipped"));
	}
	else
	{
		const FPart Parts[] = {
			{TEXT("PipeFlange"),    PipeMesh,  FVector(5.f,  0.f, 160.f), FRotator(0.f, 0.f, 0.f),  FVector(0.12f, 0.12f, 0.06f)},
			{TEXT("PipeVertical"),  PipeMesh,  FVector(5.f,  0.f, 210.f), FRotator(0.f, 0.f, 0.f),  FVector(0.064f, 0.064f, 1.f)},
			{TEXT("PipeElbow"),     ElbowMesh, FVector(5.f,  0.f, 260.f), FRotator(0.f, 0.f, 0.f),  FVector(0.09f, 0.09f, 0.09f)},
			{TEXT("PipeHorizontal"),PipeMesh,  FVector(30.f, 0.f, 260.f), FRotator(90.f, 0.f, 0.f), FVector(0.064f, 0.064f, 0.5f)},
			{TEXT("HeadNeck"),      PipeMesh,  FVector(55.f, 0.f, 256.f), FRotator(0.f, 0.f, 0.f),  FVector(0.14f, 0.14f, 0.12f)},
			{TEXT("HeadDisc"),      PipeMesh,  FVector(55.f, 0.f, 248.f), FRotator(15.f, 0.f, 0.f), FVector(0.52f, 0.52f, 0.08f)},
		};
		for (const FPart& P : Parts)
		{
			UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this, P.Name);
			C->CreationMethod = EComponentCreationMethod::UserConstructionScript;
			C->SetupAttachment(GetRootComponent());
			C->SetRelativeLocation(P.Loc);
			C->SetRelativeRotation(P.Rot);
			C->SetRelativeScale3D(P.Scale);
			C->SetStaticMesh(P.Mesh);
			if (BrassMaterial)
			{
				C->SetMaterial(0, BrassMaterial);
			}
			C->RegisterComponent();
		}
	}

	// water sound
	SoundComp = NewObject<UAudioComponent>(this, TEXT("WaterSoundFX"));
	SoundComp->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	SoundComp->SetupAttachment(GetRootComponent());
	SoundComp->SetRelativeLocation(WaterOffset);
	SoundComp->bAutoActivate = false;
	SoundComp->RegisterComponent();

	WaterComp  = MakeFXComponent(TEXT("WaterFX"),  WaterSystem,  WaterOffset,  bWaterVisible);
	SplashComp = MakeFXComponent(TEXT("SplashFX"), SplashSystem, SplashOffset, bSplashVisible);
	SteamComp  = MakeFXComponent(TEXT("SteamFX"),  SteamSystem,  SteamOffset,  bSteamVisible);
}

void AShowerActor::ApplyParams()
{
	const bool bEditorPreview = GIsEditor && !GetWorld()->IsGameWorld();

	auto ApplyWater = [&](UNiagaraComponent* C)
	{
		if (!C) return;
		C->SetVariableFloat("User.FlowRate",     FlowRate);
		C->SetVariableFloat("User.StreamSpeed",  StreamSpeed);
		C->SetVariableFloat("User.StreamSpread", StreamSpread);
		C->SetVariableFloat("User.DropSizeMin",  DropSizeMin);
		C->SetVariableFloat("User.DropSizeMax",  DropSizeMax);
		C->SetVariableFloat("User.MistRate",     MistRate);
		if (WaterMaterial) C->SetVariableObject("User.WaterMaterial", WaterMaterial);
	};
	auto ApplySplash = [&](UNiagaraComponent* C)
	{
		if (!C) return;
		C->SetVariableFloat("User.SplashRate",  SplashRate);
		C->SetVariableFloat("User.SplashSpeed", SplashSpeed);
		C->SetVariableFloat("User.SplashSizeMin", SplashSizeMin);
		C->SetVariableFloat("User.SplashSizeMax", SplashSizeMax);
	};
	auto ApplySteam = [&](UNiagaraComponent* C)
	{
		if (!C) return;
		C->SetVariableFloat("User.SteamRate", SteamRate);
		C->SetVariableFloat("User.SteamRadius", SteamRadius);
		C->SetVariableFloat("User.SteamSizeMin", SteamSizeMin);
		C->SetVariableFloat("User.SteamSizeMax", SteamSizeMax);
		C->SetVariableLinearColor("User.SteamColor", SteamColor);
		if (SteamMaterial) C->SetVariableObject("User.SteamMaterial", SteamMaterial);
	};

	ApplyWater(WaterComp);
	ApplySplash(SplashComp);
	ApplySteam(SteamComp);

	// water sound
	if (SoundComp)
	{
		SoundComp->SetRelativeLocation(WaterOffset);
		SoundComp->SetSound(WaterSound);
		SoundComp->SetVolumeMultiplier(SoundVolume);
		SoundComp->bOverrideAttenuation = true;
		SoundComp->AttenuationOverrides.bAttenuate = true;
		SoundComp->AttenuationOverrides.FalloffDistance = FMath::Max(SoundAttenuationRadius, 100.0f);
		const bool bShouldPlay = bSoundOn && (WaterSound != nullptr) && bWaterVisible;
		if (bShouldPlay)
		{
			if (!SoundComp->IsPlaying())
			{
				SoundComp->Play();
			}
		}
		else if (SoundComp->IsPlaying())
		{
			SoundComp->Stop();
		}
	}

#if WITH_EDITOR
	if (bEditorPreview)
	{
		// Restart preview simulations so every override is picked up immediately
		// when a value is changed on the instance.
		if (WaterComp)  WaterComp->ReinitializeSystem();
		if (SplashComp) SplashComp->ReinitializeSystem();
		if (SteamComp)  SteamComp->ReinitializeSystem();
	}
#endif

	UE_LOG(LogShowerFX, Log, TEXT("Applied: Flow=%f Speed=%f Spread=%f Drop=%f..%f Mist=%f | Splash=%f/%f %f..%f | Steam=%f %f..%f | Sound=%d %f/%f"),
		FlowRate, StreamSpeed, StreamSpread, DropSizeMin, DropSizeMax, MistRate,
		SplashRate, SplashSpeed, SplashSizeMin, SplashSizeMax,
		SteamRate, SteamSizeMin, SteamSizeMax,
		(int32)bSoundOn, SoundVolume, SoundAttenuationRadius);
}

void AShowerActor::SetWaterVisible(bool bVisible)
{
	bWaterVisible = bVisible;
	if (WaterComp) WaterComp->SetVisibility(bVisible, true);
	if (SoundComp && !bVisible && SoundComp->IsPlaying()) SoundComp->Stop();
	if (bVisible) OnConstruction(GetActorTransform());
	OnWaterChanged.Broadcast(bVisible);
}

void AShowerActor::SetSplashVisible(bool bVisible)
{
	bSplashVisible = bVisible;
	if (SplashComp) SplashComp->SetVisibility(bVisible, true);
	OnSplashChanged.Broadcast(bVisible);
}

void AShowerActor::SetSteamVisible(bool bVisible)
{
	bSteamVisible = bVisible;
	if (SteamComp) SteamComp->SetVisibility(bVisible, true);
	OnSteamChanged.Broadcast(bVisible);
}

void AShowerActor::SetSoundOn(bool bOn)
{
	bSoundOn = bOn;
	if (SoundComp)
	{
		if (bOn && bWaterVisible && WaterSound)
		{
			if (!SoundComp->IsPlaying()) SoundComp->Play();
		}
		else if (SoundComp->IsPlaying())
		{
			SoundComp->Stop();
		}
	}
	OnSoundChanged.Broadcast(bOn);
}

void AShowerActor::StartShower()
{
	bWaterVisible = true;
	bSplashVisible = true;
	bSteamVisible = true;
	bSoundOn = true;
	ApplyParams();
	if (WaterComp) WaterComp->SetVisibility(true, true);
	if (SplashComp) SplashComp->SetVisibility(true, true);
	if (SteamComp) SteamComp->SetVisibility(true, true);
	SetSoundOn(true);
	OnWaterChanged.Broadcast(true);
	OnSplashChanged.Broadcast(true);
	OnSteamChanged.Broadcast(true);
	OnSoundChanged.Broadcast(true);
#if WITH_EDITOR
	if (GIsEditor && !GetWorld()->IsGameWorld())
	{
		if (WaterComp) WaterComp->ReinitializeSystem();
		if (SplashComp) SplashComp->ReinitializeSystem();
		if (SteamComp) SteamComp->ReinitializeSystem();
	}
#endif
	OnShowerStarted();
}

void AShowerActor::StopShower()
{
	bWaterVisible = false;
	bSplashVisible = false;
	bSteamVisible = false;
	bSoundOn = false;
	if (WaterComp) WaterComp->SetVisibility(false, true);
	if (SplashComp) SplashComp->SetVisibility(false, true);
	if (SteamComp) SteamComp->SetVisibility(false, true);
	SetSoundOn(false);
	OnWaterChanged.Broadcast(false);
	OnSplashChanged.Broadcast(false);
	OnSteamChanged.Broadcast(false);
	OnSoundChanged.Broadcast(false);
	OnShowerStopped();
}

void AShowerActor::SetFlowRate(float NewRate)
{
	FlowRate = FMath::Max(NewRate, 0.0f);
	if (WaterComp)
	{
		WaterComp->SetVariableFloat("User.FlowRate", FlowRate);
#if WITH_EDITOR
		if (GIsEditor && !GetWorld()->IsGameWorld()) WaterComp->ReinitializeSystem();
#endif
	}
}

void AShowerActor::SetSteamRate(float NewRate)
{
	SteamRate = FMath::Max(NewRate, 0.0f);
	if (SteamComp)
	{
		SteamComp->SetVariableFloat("User.SteamRate", SteamRate);
#if WITH_EDITOR
		if (GIsEditor && !GetWorld()->IsGameWorld()) SteamComp->ReinitializeSystem();
#endif
	}
}
