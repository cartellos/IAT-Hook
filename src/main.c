#include <Windows.h>
#include <stdio.h>

typedef int (WINAPI* MessageBoxA_t)(HWND, LPCSTR, LPCSTR, UINT);
MessageBoxA_t originalMessageBoxA = NULL;

int WINAPI MyMessageBoxA(HWND hWnd, LPCSTR text, LPCSTR caption, UINT type)
{
	return originalMessageBoxA(hWnd, "Hooked!", caption, type);
}

int main(void) {
	printf("Hooking...\n");
	MessageBoxA(0, "Original", "None", MB_OK);
	DWORD oldprotect = NULL;
	DWORD temp;

	HMODULE hModule = GetModuleHandleA(NULL);

	IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)hModule;
	IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)((BYTE*)hModule + dos->e_lfanew);

	IMAGE_IMPORT_DESCRIPTOR* imp = (IMAGE_IMPORT_DESCRIPTOR*)(
		(BYTE*)hModule +
		nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress
	);

    for (; imp->Name; imp++) {
        char* dllName = (char*)((BYTE*)hModule + imp->Name);

        IMAGE_THUNK_DATA* thunk = (IMAGE_THUNK_DATA*)((BYTE*)hModule + imp->FirstThunk);
		IMAGE_THUNK_DATA* origThunk = (IMAGE_THUNK_DATA*)((BYTE*)hModule + imp->OriginalFirstThunk);
		
		
		
        for (; thunk->u1.Function; thunk++, origThunk++) {

			if (origThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) {
				continue;
			}

			IMAGE_IMPORT_BY_NAME* import = (IMAGE_IMPORT_BY_NAME*)((BYTE*)hModule + origThunk->u1.AddressOfData);

			
			if (strcmp(import->Name, "MessageBoxA") == 0) {
				
				VirtualProtect(
					&thunk->u1.Function,
					sizeof(void*),
					PAGE_READWRITE,
					&oldprotect
				);

				originalMessageBoxA = (MessageBoxA_t)thunk->u1.Function;
				thunk->u1.Function = (ULONG_PTR)MyMessageBoxA;

				VirtualProtect(
					&thunk->u1.Function,
					sizeof(void*),
					oldprotect,
					&temp
				);
				printf("Hooked!\n");
			}
        }
    }

	MessageBoxA(0, "Test", "None", MB_OK);
	return 0;
}