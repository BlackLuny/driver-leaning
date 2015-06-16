 
#include <string.h>
 
/*
0012F628   10000000  ....  //hMod
0012F62C   0012F640  @?. //UnsafeModuleName
0012F630   00000000  ....//ThreadId
0012F634   00000002  ....//hookid =Wh_KeyBoard
0012F638   100010E0  ?..   GameDll.Gameproc //HookProc
0012F63C   00000002  .... //Ansi
0012F640   00660064  d.f.
0012F644   0012F678  x?.   UNICODE "I:\\VC_Code\\PassNP_Code\\MFC_EXE\\Release\\GameDll.dll"
DWORD tid=0;
HMODULE hdll=	LoadLibraryA("GameDll.dll");
HOOKPROC Gameproc=(HOOKPROC)GetProcAddress(hdll,"Gameproc");
	Nt_SetWindowsHookEx(WH_KEYBOARD,Gameproc,::GetModuleHandle("GameDll.dll"),tid);
	
	  typedef struct _SetWindowsHookEx_Data
	  {
	  HINSTANCE hMod;
	  PUNICODE_STRING UnsafeModuleName;
	  DWORD ThreadId;
	  int HookId;
	  HOOKPROC HookProc;
	  BOOL Ansi;
	  
		}SetWindowsHookEx_Data;

*/

//#pragma pack(1)
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength; 
    PWSTR  Buffer; 	
} UNICODE_STRING,*PUNICODE_STRING;
//#pragma pack() 


__declspec(naked) void sysCall()
{

_asm
{
 
__emit   0x8B
__emit   0xD4 
__emit   0x0F 
__emit   0x34  
}
}


__declspec(naked) HHOOK __stdcall Nt_SetWindowsHookEx (HINSTANCE hMod, PUNICODE_STRING UnsafeModuleName, DWORD ThreadId, int HookId, HOOKPROC HookProc, BOOL Ansi)

{
	 _asm
	 {
		 MOV EAX,0x1225
		 call sysCall		
         retn 0x18
	 } 
	
}


void My_SetWindowsHookEx()
{   //取得RtlInitUnicodeString地址
	typedef   (__stdcall *PRtlInitUnicodeString)( PUNICODE_STRING , PCWSTR  );
	PRtlInitUnicodeString  RtlInitUnicodeString;
	RtlInitUnicodeString=(PRtlInitUnicodeString)GetProcAddress(GetModuleHandle("ntdll.dll"),"RtlInitUnicodeString");
    //
	HINSTANCE hMod=LoadLibraryA("gamedll.dll");
	UNICODE_STRING UnsafeModuleName;
	PUNICODE_STRING pname=&UnsafeModuleName;
	DWORD ThreadId=0;
	int HookId=WH_KEYBOARD;
	HOOKPROC Gameproc=(HOOKPROC)GetProcAddress(hMod,"Gameproc");
	BOOL Ansi=2;
	//初始化UnsafeModuleName 字串
	WCHAR wName[256];
	char Fullpath[256];
	GetCurrentDirectory(256,Fullpath);
	strcat(Fullpath, "\\gamedll.dll");//Fullpath=Fullpath+"gamedll.dll";
	//把Fullpath 转换成 宽字符串
	MultiByteToWideChar (CP_ACP, 0, Fullpath, -1, wName, sizeof(wName)*2+1); 

	//初始化PUNICODE_STRING字串结构 方法1
	//pname->Buffer= wName ;
	//pname->Length=wcslen(pname->Buffer)*2+1  ;
	//UnsafeModuleName.MaximumLength=0x0fff;

   	//初始化PUNICODE_STRING字串结构 方法2
	RtlInitUnicodeString(pname,wName);
	// hMod=GetModuleHandle("GameDll.dll");
	Nt_SetWindowsHookEx ( hMod,  &UnsafeModuleName,  ThreadId,  HookId,   Gameproc,  Ansi);
 
}
__declspec(naked) HWND __stdcall Nt_FindWindow (int p1,int p2,PUNICODE_STRING p3_ClassName,PUNICODE_STRING p4_Caption, int p5) 
{


	__asm
	{
	    mov eax,0x117a //NtUserFindWindowEx
		call sysCall 
		retn 0x14
	}
}