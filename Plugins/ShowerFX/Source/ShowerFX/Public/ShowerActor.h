#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "ShowerActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FShowerFXBoolEvent, bool, bOn);

class UNiagaraSystem;
class UMaterialInterface;
class UStaticMesh;
class USoundBase;
class UAudioComponent;

/**
 * Self-contained shower VFX actor: brass geometry + water/floor-splash/steam Niagara systems.
 * Every parameter is applied on each construction pass — edit values on the instance and the effect updates instantly.
 *
 * Самодостаточный актор душа: латунная геометрия + системы Niagara (вода / брызги / пар).
 * Все параметры применяются при каждой перестройке актора — правьте значения прямо на инстансе.
 */
UCLASS(Blueprintable)
class SHOWERFX_API AShowerActor : public AActor
{
	GENERATED_BODY()

public:
	AShowerActor();

	// --- Assets / Ассеты ---

	/** Niagara system with the falling water stream + mist / Система Niagara: падающая струя воды + туман */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Assets")
	TObjectPtr<UNiagaraSystem> WaterSystem;

	/** Niagara system with floor splash droplets / Система Niagara: брызги на полу */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Assets")
	TObjectPtr<UNiagaraSystem> SplashSystem;

	/** Niagara system with steam in the stall / Система Niagara: пар в кабинке */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Assets")
	TObjectPtr<UNiagaraSystem> SteamSystem;

	/** Static mesh used for pipes and the head disc / Статик-меш труб и диска лейки */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Assets")
	TObjectPtr<UStaticMesh> PipeMesh;

	/** Static mesh for the curved elbow joint / Статик-меш сферического колена трубы */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Assets")
	TObjectPtr<UStaticMesh> ElbowMesh;

	/** Brass material for all shower geometry / Материал латуни для всей геометрии душа */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Assets")
	TObjectPtr<UMaterialInterface> BrassMaterial;

	// --- Materials / Материалы воды и пара ---

	/** Material (or Material Instance) for water drops — applied per instance / Материал (лучше Material Instance) капель воды — назначается на инстанс */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Materials")
	TObjectPtr<UMaterialInterface> WaterMaterial;

	/** Material (or Material Instance) for steam — applied per instance / Материал (лучше Material Instance) пара — назначается на инстанс */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Materials")
	TObjectPtr<UMaterialInterface> SteamMaterial;

	// --- Water stream / Водяная струя ---

	/** Water drops spawned per second / Сколько капель воды появляется в секунду */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Water", meta = (ClampMin = "0", ToolTip = "Water spawn rate, drops per second / Частота спавна воды (капель в секунду). Больше = плотнее струя."))
	float FlowRate = 800.0f;

	/** Initial speed of drops leaving the head / Начальная скорость капель у лейки */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Water", meta = (ClampMin = "0", ToolTip = "Initial drop speed, cm/s / Начальная скорость капель (см/с). Больше = сильнее и дальше бьёт струя."))
	float StreamSpeed = 200.0f;

	/** Cone spread angle of the stream, degrees / Угол разлёта струи (градусы) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Water", meta = (ClampMin = "0", ClampMax = "89", ToolTip = "Cone spread angle of the stream in degrees / Угол конуса разлёта струи в градусах. Больше = шире «душ», меньше = тоньше струя."))
	float StreamSpread = 12.0f;

	/** Minimum drop sprite size, cm / Минимальный размер капли (см) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Water", meta = (ClampMin = "0", ToolTip = "Minimum drop size, cm / Минимальный размер капли (см)."))
	float DropSizeMin = 4.0f;

	/** Maximum drop sprite size, cm / Максимальный размер капли (см) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Water", meta = (ClampMin = "0", ToolTip = "Maximum drop size, cm / Максимальный размер капли (см)."))
	float DropSizeMax = 6.0f;

	/** Mist emission rate around the stream / Интенсивность тумана вокруг струи */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Water", meta = (ClampMin = "0", ToolTip = "Mist particles per second / Частиц тумана в секунду. 0 = выключить туман."))
	float MistRate = 90.0f;

	/** Water emitter position relative to the actor / Позиция эмиттера воды относительно актора */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Water", meta = (ToolTip = "Water emitter offset from the actor origin (should sit under the head) / Смещение эмиттера воды от пивота актора (должен быть под лейкой)."))
	FVector WaterOffset = FVector(55.f, 0.f, 244.f);

	/** Show/hide the water / Показать/скрыть воду */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Water", meta = (ToolTip = "Enable the water stream / Включить водяную струю."))
	bool bWaterVisible = true;

	// --- Floor splash / Брызги на полу ---

	/** Splash droplets spawned per second / Сколько брызг появляется в секунду */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Splash", meta = (ClampMin = "0", ToolTip = "Splash droplets per second / Брызг в секунду. Больше = плотнее всплеск у пола."))
	float SplashRate = 380.0f;

	/** Splash bounce speed / Скорость отскока брызг */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Splash", meta = (ClampMin = "0", ToolTip = "Splash bounce speed, cm/s / Скорость отскока брызг (см/с). Больше = брызги летят выше и дальше."))
	float SplashSpeed = 120.0f;

	/** Minimum splash droplet size, cm / Минимальный размер брызги (см) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Splash", meta = (ClampMin = "0", ToolTip = "Minimum splash size, cm / Минимальный размер брызг (см)."))
	float SplashSizeMin = 2.0f;

	/** Maximum splash droplet size, cm / Максимальный размер брызги (см) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Splash", meta = (ClampMin = "0", ToolTip = "Maximum splash size, cm / Максимальный размер брызг (см)."))
	float SplashSizeMax = 3.2f;

	/** Splash emitter position (floor point under the stream) / Позиция брызг (точка удара струи о пол) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Splash", meta = (ToolTip = "Splash emitter offset from the actor origin (the floor impact point) / Смещение эмиттера брызг от пивота актора (точка, куда падает струя)."))
	FVector SplashOffset = FVector(55.f, 0.f, 6.f);

	/** Show/hide the splash / Показать/скрыть брызги */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Splash", meta = (ToolTip = "Enable floor splash / Включить брызги на полу."))
	bool bSplashVisible = true;

	// --- Steam / Пар ---

	/** Steam puffs spawned per second / Сколько клубов пара появляется в секунду */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Steam", meta = (ClampMin = "0", ToolTip = "Steam particles per second / Частиц пара в секунду. 0 = выключить пар."))
	float SteamRate = 25.0f;

	/** Minimum steam puff size, cm / Минимальный размер клуба пара (см) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Steam", meta = (ClampMin = "0", ToolTip = "Minimum steam size, cm / Минимальный размер пара (см)."))
	float SteamSizeMin = 18.0f;

	/** Maximum steam puff size, cm / Максимальный размер клуба пара (см) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Steam", meta = (ClampMin = "0", ToolTip = "Maximum steam size, cm / Максимальный размер пара (см)."))
	float SteamSizeMax = 34.0f;

	/** Steam emitter position inside the stall / Позиция пара в кабинке */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Steam", meta = (ToolTip = "Steam emitter offset from the actor origin / Смещение эмиттера пара от пивота актора."))
	FVector SteamOffset = FVector(55.f, 0.f, 150.f);

	/** Show/hide the steam / Показать/скрыть пар */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Steam", meta = (ToolTip = "Enable steam / Включить пар."))
	bool bSteamVisible = true;

	/** Steam puff radius / Радиус облака пара */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Steam", meta = (ClampMin = "0", ToolTip = "Steam cloud radius, cm / Радиус области появления пара (см). Больше = шире облако."))
	float SteamRadius = 25.0f;

	/** Steam tint + transparency (alpha) / Цвет пара и прозрачность (альфа) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Steam", meta = (ToolTip = "Steam tint and transparency (alpha channel) / Цвет пара; альфа-канал = прозрачность пара."))
	FLinearColor SteamColor = FLinearColor(0.92f, 0.96f, 1.0f, 0.35f);

	// --- Water sound / Звук воды ---

	/** Looping water sound asset / Зацикленный звук воды */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Sound", meta = (ToolTip = "Looping water sound / Зацикленный звук воды (Sound Wave)."))
	TObjectPtr<USoundBase> WaterSound;

	/** Sound volume / Громкость звука */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Sound", meta = (ClampMin = "0", ToolTip = "Sound volume multiplier / Множитель громкости звука (0 = тихо)."))
	float SoundVolume = 1.0f;

	/** Audible radius / Радиус слышимости */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Sound", meta = (ClampMin = "0", ToolTip = "Audible radius, cm / Радиус слышимости звука (см)."))
	float SoundAttenuationRadius = 1500.0f;

	/** Sound on/off / Звук вкл/выкл */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowerFX|Sound", meta = (ToolTip = "Enable water sound / Включить звук воды."))
	bool bSoundOn = true;

	virtual void OnConstruction(const FTransform& Transform) override;

	// --- Blueprint control nodes / Ноды для блюпринта ---

	/** Show or hide the water stream / Показать или скрыть воду */
	UFUNCTION(BlueprintCallable, Category = "ShowerFX", meta = (ToolTip = "Show or hide the water stream / Показать или скрыть водяную струю."))
	void SetWaterVisible(bool bVisible);

	/** Show or hide the floor splash / Показать или скрыть брызги */
	UFUNCTION(BlueprintCallable, Category = "ShowerFX", meta = (ToolTip = "Show or hide the floor splash / Показать или скрыть брызги на полу."))
	void SetSplashVisible(bool bVisible);

	/** Show or hide the steam / Показать или скрыть пар */
	UFUNCTION(BlueprintCallable, Category = "ShowerFX", meta = (ToolTip = "Show or hide the steam / Показать или скрыть пар."))
	void SetSteamVisible(bool bVisible);

	/** Turn the water sound on/off / Включить или выключить звук воды */
	UFUNCTION(BlueprintCallable, Category = "ShowerFX", meta = (ToolTip = "Turn the water sound on or off / Включить или выключить звук воды."))
	void SetSoundOn(bool bOn);

	/** Turn everything on (water + splash + steam + sound) / Включить всё: воду, брызги, пар, звук */
	UFUNCTION(BlueprintCallable, Category = "ShowerFX", meta = (ToolTip = "Turn everything on: water, splash, steam, sound / Включить всё: воду, брызги, пар и звук."))
	void StartShower();

	/** Turn everything off / Выключить всё */
	UFUNCTION(BlueprintCallable, Category = "ShowerFX", meta = (ToolTip = "Turn everything off: water, splash, steam, sound / Выключить всё: воду, брызги, пар и звук."))
	void StopShower();

	// --- Events & dispatchers / Ивенты и диспатчеры ---

	/** Fired when water is toggled / Срабатывает при включении/выключении воды */
	UPROPERTY(BlueprintAssignable, Category = "ShowerFX|Events", meta = (ToolTip = "Fired when the water is toggled / Срабатывает при переключении воды."))
	FShowerFXBoolEvent OnWaterChanged;

	/** Fired when splash is toggled / Срабатывает при переключении брызг */
	UPROPERTY(BlueprintAssignable, Category = "ShowerFX|Events", meta = (ToolTip = "Fired when the splash is toggled / Срабатывает при переключении брызг."))
	FShowerFXBoolEvent OnSplashChanged;

	/** Fired when steam is toggled / Срабатывает при переключении пара */
	UPROPERTY(BlueprintAssignable, Category = "ShowerFX|Events", meta = (ToolTip = "Fired when the steam is toggled / Срабатывает при переключении пара."))
	FShowerFXBoolEvent OnSteamChanged;

	/** Fired when sound is toggled / Срабатывает при переключении звука */
	UPROPERTY(BlueprintAssignable, Category = "ShowerFX|Events", meta = (ToolTip = "Fired when the sound is toggled / Срабатывает при переключении звука."))
	FShowerFXBoolEvent OnSoundChanged;

	/** Implement in Blueprint child: shower fully started / Реализуй в блюпринте-наследнике: душ полностью включён */
	UFUNCTION(BlueprintImplementableEvent, Category = "ShowerFX|Events", meta = (ToolTip = "Blueprint event: shower started / Блюпринт-ивент: душ включён."))
	void OnShowerStarted();

	/** Implement in Blueprint child: shower fully stopped / Реализуй в блюпринте-наследнике: душ полностью выключен */
	UFUNCTION(BlueprintImplementableEvent, Category = "ShowerFX|Events", meta = (ToolTip = "Blueprint event: shower stopped / Блюпринт-ивент: душ выключен."))
	void OnShowerStopped();

	/** Change water density live / Менять плотность воды на лету */
	UFUNCTION(BlueprintCallable, Category = "ShowerFX", meta = (ToolTip = "Change the water spawn rate live / Менять плотность воды (капель/сек) на лету."))
	void SetFlowRate(float NewRate);

	/** Change steam density live / Менять плотность пара на лету */
	UFUNCTION(BlueprintCallable, Category = "ShowerFX", meta = (ToolTip = "Change the steam spawn rate live / Менять плотность пара (частиц/сек) на лету."))
	void SetSteamRate(float NewRate);

protected:
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> WaterComp;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> SplashComp;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> SteamComp;

	UPROPERTY()
	TObjectPtr<UAudioComponent> SoundComp;

	void BuildComponents();
	void ApplyParams();

	UNiagaraComponent* MakeFXComponent(FName Name, UNiagaraSystem* System, const FVector& Loc, bool bVisible);

	// Lazy asset default paths
	FString WaterSystemPath;
	FString SplashSystemPath;
	FString SteamSystemPath;
	FString PipeMeshPath;
	FString ElbowMeshPath;
	FString BrassMaterialPath;
	FString WaterMaterialPath;
	FString SteamMaterialPath;
	FString WaterSoundPath;
};
