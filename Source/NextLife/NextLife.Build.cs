// Copyright 2020-2021 Solar Storm Interactive. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class NextLife : ModuleRules
{
	public NextLife(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Disable for Non-Developer builds
		OptimizeCode = CodeOptimization.Never;

        PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
                "CoreUObject",
                "Engine",
                "AIModule",
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
