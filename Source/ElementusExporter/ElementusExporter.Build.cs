// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEElementusExporter

using UnrealBuildTool;

public class ElementusExporter : ModuleRules
{
	public ElementusExporter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp17;

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core"
			});


		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CoreUObject",
				"Projects",
				"Engine"
			});

		PrivateIncludePathModuleNames.Add("DesktopPlatform");
	}
}