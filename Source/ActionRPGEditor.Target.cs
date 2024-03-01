// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildBase;
using UnrealBuildTool;
using System;
using System.IO;
using System.Collections.Generic;
using System.Diagnostics;
using EpicGames.Core;

public class ActionRPGEditorTarget : TargetRules
{
	public ActionRPGEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		ExtraModuleNames.AddRange( new string[] { "ActionRPG" } );

		// Build\BatchFiles\Build.bat -Target="GameEditor Win64 Development" -Project="G:\Client\Game.uproject" -WaitMutex -importcer
		bool bExportCer = IsContainInCmd("-importcer");
		if(Target.Platform == UnrealTargetPlatform.Win64 && bExportCer)
		{
			string IPhonePackagerExePath = Path.GetFullPath(Path.Combine(Unreal.EngineDirectory.FullName, "Binaries/DotNET/IOS/IPhonePackager.exe"));
			string UProjectFile = Target.ProjectFile.ToString();
			Console.WriteLine("uproject file:" + UProjectFile);
			Console.WriteLine("IPhonePackagerExePath:" + IPhonePackagerExePath);
			if(File.Exists(IPhonePackagerExePath))
			{
				Dictionary<string, string> BundleIdMap = new Dictionary<string, string>();
				BundleIdMap.Add("com.xxx.yyy", "password1");
				BundleIdMap.Add("com.xxx.yyy.zzz", "password2");
				string[] Configurations = { "Development" };
				string CerRelativePath = "Source/ThirdParty/IOS/";

				DirectoryReference ProjectDir = ProjectFile.Directory;
				foreach(KeyValuePair<string, string> BundleIdPair in BundleIdMap)
				{
					string BundleId = BundleIdPair.Key;
					for(int ConfigurationIndex = 0; ConfigurationIndex < Configurations.Length;  ConfigurationIndex++)
					{
						string PackageConfiguration = Configurations[ConfigurationIndex];
						string mobileprovision_name = BundleId + "_" + PackageConfiguration + ".mobileprovision";
						string cer_file_name = BundleId + "_" + PackageConfiguration + ".p12";
						string cerPath = Path.Combine(ProjectDir.FullName, CerRelativePath, BundleId, PackageConfiguration, cer_file_name);
						string provisionPath = Path.Combine(ProjectDir.FullName, CerRelativePath, BundleId, PackageConfiguration, mobileprovision_name);

						if(File.Exists(cerPath) && File.Exists(provisionPath))
						{
							string cerPassword = BundleIdPair.Value;
							string[] Cmds =
							{
								String.Format(
									"Install Engine -project {0} -certificate {1} -cerpassword {2} -bundlename {3}",
									UProjectFile, cerPath, cerPassword, BundleId),
								String.Format(
									"Install Engine -project {0} -provision {1} -bundlename {2}",
									UProjectFile, provisionPath, BundleId),
							};
							for(int CmdIndex = 0; CmdIndex < Cmds.Length; CmdIndex++)
							{
								ProcessStartInfo startInfo = new ProcessStartInfo();
								startInfo.CreateNoWindow = true;
								startInfo.UseShellExecute = false;
								startInfo.FileName = IPhonePackagerExePath;
								startInfo.Arguments = Cmds[CmdIndex];
								Console.WriteLine(string.Format("Import Cer&Provision cmd: {0} {1}", startInfo.FileName, startInfo.Arguments));
								Process.Start(startInfo);
							}
						}
					}
				}
			}
		}
	}

	private bool IsContainInCmd( string cmd )
	{
		string[] arguments = Environment.GetCommandLineArgs();
		for (int index = 0; index < arguments.Length; ++index)
		{
			if (arguments[index] == cmd)
			{
				return true;
			}
		}
		return false;
	}
}
