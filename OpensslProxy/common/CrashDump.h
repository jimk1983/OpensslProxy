
#ifdef __cplusplus
extern "C" {
#endif

void CrashDumpInitialize();

//void CrashDumpInitialize(const wchar_t *aInstallPath, const wchar_t *aDumpFolderPath, bool aFullDump, int aDumpFileMax);

#define  DumpDirName			L"\\OpenSSLProxy"
#define  DumpMiniName		L"\\minidump"

#ifdef __cplusplus
}
#endif