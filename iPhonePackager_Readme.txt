//命令进行静默式的证书导入
# 导入证书
IPhonePackager.exe Install Engine -project D:\ActionRPG\ActionRPG.uproject -certificate G:\Client\Source\ThirdParty\iOS\com.xxxx.yyyy\Development\com.xxxx.yyyy_Development.p12 -cerpassword cerpassword -bundlename com.xxxx.yyyy
# 导入provision
IPhonePackager.exe Install Engine -project D:\ActionRPG\ActionRPG.uproject -provision G:\Client\Source\ThirdParty\iOS\com.xxxx.yyyy\Development\com.xxxx.yyyy_Development.mobileprovision -bundlename com.xxxx.yyyy

// D:\UnrealEngine\Engine\Source\Programs\IOS\iPhonePackager\iPhonePackager.sln
// D:\UnrealEngine\Engine\Binaries\DotNET\IOS\IPhonePackager.exe
// Engine/Source/Programs/IOS/iPhonePackager/ToolsHub.cs

public static void TryInstallingCertificate_PromptForKey(string CertificateFilename, bool ShowPrompt = true)
{
    try
    {
        if (!String.IsNullOrEmpty(CertificateFilename) || ShowOpenFileDialog(CertificatesFilter, "Choose a code signing certificate to import", "", "", ref ChoosingFilesToInstallDirectory, out CertificateFilename))
        {
            if (Environment.OSVersion.Platform == PlatformID.MacOSX || Environment.OSVersion.Platform == PlatformID.Unix)
            {
                // run certtool y to get the currently installed certificates
                CertToolData = "";
                Process CertTool = new Process();
                CertTool.StartInfo.FileName = "/usr/bin/security";
                CertTool.StartInfo.UseShellExecute = false;
                CertTool.StartInfo.Arguments = "import \"" + CertificateFilename +"\" -k login.keychain";
                CertTool.StartInfo.RedirectStandardOutput = true;
                CertTool.OutputDataReceived += new DataReceivedEventHandler(OutputReceivedCertToolProcessCall);
                CertTool.Start();
                CertTool.BeginOutputReadLine();
                CertTool.WaitForExit();
                if (CertTool.ExitCode != 0)
                {
                    // todo: provide some feedback that it failed
                }
                Console.Write(CertToolData);
            }
            else
            {
                // Load the certificate
                string CertificatePassword = "";
                string[] arguments = Environment.GetCommandLineArgs();
                for(int index = 0;index < arguments.Length; ++index)
                {
                    if (arguments[index] == "-cerpassword" && index != (arguments.Length - 1))
                    {
                        CertificatePassword = arguments[index + 1];
                        Console.WriteLine("Usage -cerpassword argumnet");
                    }
                }

                X509Certificate2 Cert = null;
                try
                {
                    Cert = new X509Certificate2(CertificateFilename, CertificatePassword, X509KeyStorageFlags.PersistKeySet | X509KeyStorageFlags.Exportable | X509KeyStorageFlags.MachineKeySet);
                }
                catch (System.Security.Cryptography.CryptographicException ex)
                {
                    // Try once with a password
                    if (PasswordDialog.RequestPassword(out CertificatePassword))
                    {
                        Cert = new X509Certificate2(CertificateFilename, CertificatePassword, X509KeyStorageFlags.PersistKeySet | X509KeyStorageFlags.Exportable | X509KeyStorageFlags.MachineKeySet);
                    }
                    else
                    {
                        // User cancelled dialog, rethrow
                        throw ex;
                    }
                }

                // If the certificate doesn't have a private key pair, ask the user to provide one
                if (!Cert.HasPrivateKey)
                {
                    string ErrorMsg = "Certificate does not include a private key and cannot be used to code sign";

                    // Prompt for a key pair
                    if (MessageBox(new IntPtr(0), "Next, please choose the key pair that you made when generating the certificate request.",
                        Config.AppDisplayName,
                        0x00000000 | 0x00000040 | 0x00001000 | 0x00010000) == 1)
                    {
                        string KeyFilename;
                        if (ShowOpenFileDialog(KeysFilter, "Choose the key pair that belongs with the signing certificate", "", "", ref ChoosingFilesToInstallDirectory, out KeyFilename))
                        {
                            Cert = CryptoAdapter.CombineKeyAndCert(CertificateFilename, KeyFilename);
                                
                            if (Cert.HasPrivateKey)
                            {
                                ErrorMsg = null;
                            }
                        }
                    }

                    if (ErrorMsg != null)
                    {
                        throw new Exception(ErrorMsg);
                    }
                }

                // Add the certificate to the store
                X509Store Store = new X509Store();
                Store.Open(OpenFlags.ReadWrite);
                Store.Add(Cert);
                Store.Close();
            }
        }
    }
    catch (Exception ex)
    {
        string ErrorMsg = String.Format("Failed to load or install certificate due to an error: '{0}'", ex.Message);
        Program.Error(ErrorMsg);
        System.Threading.Thread.Sleep(500);
        MessageBox(new IntPtr(0), ErrorMsg, Config.AppDisplayName, 0x00000000 | 0x00000010 | 0x00001000 | 0x00010000);
    }
}