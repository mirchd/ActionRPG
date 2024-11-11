#include "CS_OpenSSL.h"

#define LOCTEXT_NAMESPACE "FCS_OpenSSLModule"

void FCS_OpenSSLModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FCS_OpenSSLModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCS_OpenSSLModule, CS_OpenSSL)