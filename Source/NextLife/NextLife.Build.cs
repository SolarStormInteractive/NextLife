// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class NextLife : ModuleRules
{
	private string BinariesPath => Path.GetFullPath(Path.Combine(ModuleDirectory, "../../Binaries/"));
	private string PluginsPath => Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../"));

	public NextLife(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Disable for Non-Developer builds
		//OptimizeCode = CodeOptimization.Never;

        PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
                "CoreUObject",
                "Engine",
			}
		);

        PrivateDependencyModuleNames.AddRange(
	        new string[]
	        {
		        "Projects",
	        }
        );
	}
}
