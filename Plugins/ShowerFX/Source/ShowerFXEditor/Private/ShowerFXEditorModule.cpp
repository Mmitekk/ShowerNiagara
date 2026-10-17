#include "Modules/ModuleManager.h"
#include "Misc/EngineVersionComparison.h"
#include "Containers/Ticker.h"
#include "Editor.h"

#if UE_VERSION_OLDER_THAN(5, 8, 0)

DEFINE_LOG_CATEGORY_STATIC(LogShowerFXEd, Log, All);

class FShowerFXEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		UE_LOG(LogShowerFXEd, Log, TEXT("ShowerFXEditor: UE < 5.8 detected, automatic Niagara linking is skipped. Link User.* parameters manually in the Niagara UI."));
	}
};

#else

#include "NiagaraExternalSystemEditorUtilities.h"
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"
#include "NiagaraTypes.h"
#include "NiagaraRendererProperties.h"
#include "NiagaraSpriteRendererProperties.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "LevelEditorViewport.h"

DEFINE_LOG_CATEGORY_STATIC(LogShowerFXEd, Log, All);

namespace ShowerFX
{
	static const TCHAR* WaterSystemPath = TEXT("/Game/VFX/Shower/NS_ShowerWater.NS_ShowerWater");
	static const TCHAR* SplashSystemPath = TEXT("/Game/VFX/Shower/NS_ShowerSplash.NS_ShowerSplash");
	static const TCHAR* SteamSystemPath = TEXT("/Game/VFX/Shower/NS_ShowerSteam.NS_ShowerSteam");
	static const TCHAR* WaterMatDefaultPath = TEXT("/Game/VFX/Shower/M_WaterDrop_Inst.M_WaterDrop_Inst");
	static const TCHAR* SteamMatDefaultPath = TEXT("/Game/VFX/Shower/M_Steam_Inst.M_Steam_Inst");

	static FNiagaraTypeDefinition FloatType() { return FNiagaraTypeDefinition::GetFloatDef(); }
	static FNiagaraTypeDefinition MaterialType() { return FNiagaraTypeDefinition(UMaterialInterface::StaticClass()); }
	static FNiagaraTypeDefinition LinearColorType() { return FNiagaraTypeDefinition::GetColorDef(); }

	static void CloseEditorsFor(UNiagaraSystem* Sys)
	{
		if (GEditor)
		{
			if (UAssetEditorSubsystem* AES = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
			{
				AES->CloseAllEditorsForAsset(Sys);
			}
		}
	}

	static bool HasUserVar(UNiagaraSystem* Sys, FNiagaraExternalEditContext& Ctx, FName Name)
	{
		FNiagaraExt_SystemSummary Summary;
		UNiagaraExternalEditUtilities::GetSystemSummary(Sys, Summary, Ctx);
		for (const FNiagaraExt_UserVariable& V : Summary.UserVariables)
		{
			if (V.Name == Name)
			{
				return true;
			}
		}
		return false;
	}

	static void EnsureUserVar(UNiagaraSystem* Sys, FName Name, const FNiagaraTypeDefinition& Type, UMaterialInterface* DefaultObject)
	{
		FNiagaraExternalEditContext Ctx(Sys);
		if (HasUserVar(Sys, Ctx, Name))
		{
			return;
		}
		FNiagaraExt_UserVariable Var;
		Var.Name = Name;
		Var.Type = Type;
		FNiagaraExt_VariableValue_Object& Obj = Var.DefaultValue.InitializeAs<FNiagaraExt_VariableValue_Object>();
		Obj.ObjectClass = UMaterialInterface::StaticClass();
		Obj.Object = DefaultObject;
		UNiagaraExternalEditUtilities::AddUserVariable(Sys, Var, Ctx);
		UE_LOG(LogShowerFXEd, Log, TEXT("[%s] added user var %s"), *Sys->GetName(), *Name.ToString());
	}

	static void EnsureLinearColorVar(UNiagaraSystem* Sys, FName Name, const FLinearColor& Default)
	{
		FNiagaraExternalEditContext Ctx(Sys);
		if (HasUserVar(Sys, Ctx, Name))
		{
			return;
		}
		FNiagaraExt_UserVariable Var;
		Var.Name = Name;
		Var.Type = LinearColorType();
		FNiagaraVariant VColor;
		VColor.SetBytesValue<FLinearColor>(LinearColorType(), Default);
		Var.DefaultValue.Set(LinearColorType(), VColor);
		UNiagaraExternalEditUtilities::AddUserVariable(Sys, Var, Ctx);
		UE_LOG(LogShowerFXEd, Log, TEXT("[%s] added user var %s"), *Sys->GetName(), *Name.ToString());
	}

	static void LinkInput(UNiagaraSystem* Sys, FName Emitter, FName Script, FName Module, FName Input, FName VarName, const FNiagaraTypeDefinition& Type)
	{
		FNiagaraExternalEditContext Ctx(Sys);
		FNiagaraExt_StackItemReference Ref(Sys, Emitter, Script, Module);
		Ref.InputNameStack = {Input};

		FNiagaraExt_StackInputValue Current;
		UNiagaraExternalEditUtilities::GetStackInputData(Ref, Current, Ctx);
		if (FNiagaraExt_StackInputData_Linked* Linked = Current.GetMutablePtr<FNiagaraExt_StackInputData_Linked>())
		{
			if (Linked->LinkedVariable.Name == VarName)
			{
				return;
			}
		}

		FNiagaraExt_StackInputValue NewVal;
		FNiagaraExt_StackInputData_Linked& L = NewVal.InitializeAs<FNiagaraExt_StackInputData_Linked>();
		L.LinkedVariable.Name = VarName;
		L.LinkedVariable.Type = Type;
		UNiagaraExternalEditUtilities::SetStackInputData(Ref, NewVal, Ctx);
		UE_LOG(LogShowerFXEd, Log, TEXT("[%s] %s.%s -> %s"), *Sys->GetName(), *Emitter.ToString(), *Input.ToString(), *VarName.ToString());
	}

	static void BindRendererMaterial(UNiagaraSystem* Sys, FName EmitterName, FName VarName)
	{
		for (FNiagaraEmitterHandle& Handle : Sys->GetEmitterHandles())
		{
			if (Handle.GetName() != EmitterName)
			{
				continue;
			}
			FVersionedNiagaraEmitterData* D = Handle.GetEmitterData();
			if (!D)
			{
				continue;
			}
			for (UNiagaraRendererProperties* RP : D->GetRenderers())
			{
				if (UNiagaraSpriteRendererProperties* SR = Cast<UNiagaraSpriteRendererProperties>(RP))
				{
					SR->MaterialUserParamBinding.Parameter = FNiagaraVariable(MaterialType(), VarName);
					UE_LOG(LogShowerFXEd, Log, TEXT("[%s] %s renderer bound to %s"), *Sys->GetName(), *EmitterName.ToString(), *VarName.ToString());
				}
			}
		}
	}

	static void FixShowerSystems()
	{
		UE_LOG(LogShowerFXEd, Log, TEXT("ShowerFX fixer: start"));

		UMaterialInterface* WaterMatDefault = LoadObject<UMaterialInterface>(nullptr, WaterMatDefaultPath);
		UMaterialInterface* SteamMatDefault = LoadObject<UMaterialInterface>(nullptr, SteamMatDefaultPath);

		const TCHAR* Paths[] = {WaterSystemPath, SplashSystemPath, SteamSystemPath};

		TArray<UNiagaraSystem*> Fixed;
		for (const TCHAR* P : Paths)
		{
			UNiagaraSystem* Sys = LoadObject<UNiagaraSystem>(nullptr, P);
			if (!Sys)
			{
				UE_LOG(LogShowerFXEd, Warning, TEXT("ShowerFX fixer: system not found: %s"), P);
				continue;
			}
			CloseEditorsFor(Sys);
			Fixed.Add(Sys);
		}

		for (UNiagaraSystem* Sys : Fixed)
		{
			FNiagaraExternalEditContext Ctx(Sys);
			const bool bIsWater = Sys->GetName() == TEXT("NS_ShowerWater");
			const bool bIsSteam = Sys->GetName() == TEXT("NS_ShowerSteam");

			if (bIsWater)
			{
				EnsureUserVar(Sys, "User.WaterMaterial", MaterialType(), WaterMatDefault);
				LinkInput(Sys, "Spray", "EmitterUpdateScript", "SpawnRate", "SpawnRate", "User.FlowRate", FloatType());
				LinkInput(Sys, "Spray", "ParticleSpawnScript", "AddVelocity", "Velocity Speed", "User.StreamSpeed", FloatType());
				LinkInput(Sys, "Spray", "ParticleSpawnScript", "AddVelocity", "Cone Angle", "User.StreamSpread", FloatType());
				LinkInput(Sys, "Spray", "ParticleSpawnScript", "InitializeParticle", "Uniform Sprite Size Min", "User.DropSizeMin", FloatType());
				LinkInput(Sys, "Spray", "ParticleSpawnScript", "InitializeParticle", "Uniform Sprite Size Max", "User.DropSizeMax", FloatType());
				LinkInput(Sys, "Mist", "EmitterUpdateScript", "SpawnRate", "SpawnRate", "User.MistRate", FloatType());
				BindRendererMaterial(Sys, "Spray", "User.WaterMaterial");
				BindRendererMaterial(Sys, "Mist", "User.WaterMaterial");
			}
			else if (bIsSteam)
			{
				EnsureUserVar(Sys, "User.SteamMaterial", MaterialType(), SteamMatDefault);
				EnsureLinearColorVar(Sys, "User.SteamColor", FLinearColor(0.92f, 0.96f, 1.0f, 0.35f));
				LinkInput(Sys, "Steam", "ParticleSpawnScript", "InitializeParticle", "Color", "User.SteamColor", LinearColorType());
				LinkInput(Sys, "Steam", "EmitterUpdateScript", "SpawnRate", "SpawnRate", "User.SteamRate", FloatType());
				LinkInput(Sys, "Steam", "ParticleSpawnScript", "ShapeLocation", "Sphere Radius", "User.SteamRadius", FloatType());
				LinkInput(Sys, "Steam", "ParticleSpawnScript", "InitializeParticle", "Uniform Sprite Size Min", "User.SteamSizeMin", FloatType());
				LinkInput(Sys, "Steam", "ParticleSpawnScript", "InitializeParticle", "Uniform Sprite Size Max", "User.SteamSizeMax", FloatType());
				BindRendererMaterial(Sys, "Steam", "User.SteamMaterial");
			}
			else // splash
			{
				LinkInput(Sys, "Splash", "EmitterUpdateScript", "SpawnRate", "SpawnRate", "User.SplashRate", FloatType());
				LinkInput(Sys, "Splash", "ParticleSpawnScript", "AddVelocity", "Velocity Speed", "User.SplashSpeed", FloatType());
				LinkInput(Sys, "Splash", "ParticleSpawnScript", "InitializeParticle", "Uniform Sprite Size Min", "User.SplashSizeMin", FloatType());
				LinkInput(Sys, "Splash", "ParticleSpawnScript", "InitializeParticle", "Uniform Sprite Size Max", "User.SplashSizeMax", FloatType());
			}

			Sys->RequestCompile(true);
		}

		for (UNiagaraSystem* Sys : Fixed)
		{
			Sys->MarkPackageDirty();
		}
		UE_LOG(LogShowerFXEd, Log, TEXT("ShowerFX fixer: done (%d systems)"), Fixed.Num());
	}

	static void ForceRealtime()
	{
		if (!GEditor)
		{
			return;
		}
		int32 N = 0;
		for (FLevelEditorViewportClient* VC : GEditor->GetLevelViewportClients())
		{
			if (VC && !VC->IsRealtime())
			{
				VC->SetRealtime(true);
				++N;
			}
		}
		if (N > 0)
		{
			UE_LOG(LogShowerFXEd, Log, TEXT("ShowerFX: forced realtime on %d viewport(s)"), N);
		}
	}

	static bool StartupTick(float)
	{
		ForceRealtime();
		FixShowerSystems();
		return false; // one-shot
	}

	static bool RealtimeTick(float)
	{
		ForceRealtime();
		return false; // one-shot
	}

	static void OnMapOpened(const FString&, bool)
	{
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateStatic(&RealtimeTick), 2.0f);
	}
}

class FShowerFXEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateStatic(&ShowerFX::StartupTick), 20.0f);
		FEditorDelegates::OnMapOpened.AddStatic(&ShowerFX::OnMapOpened);
	}
};

#endif

IMPLEMENT_MODULE(FShowerFXEditorModule, ShowerFXEditor)
