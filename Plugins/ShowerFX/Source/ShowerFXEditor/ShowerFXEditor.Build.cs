using UnrealBuildTool;

public class ShowerFXEditor : ModuleRules
{
	public ShowerFXEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"ShowerFX",
			"Niagara",
			"NiagaraEditor",
			"UnrealEd",
			"Slate",
			"SlateCore"
		});
	}
}
