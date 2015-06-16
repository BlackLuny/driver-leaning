//#include <windows.h>
#pragma pack(1)
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;  
    PWSTR  Buffer; 
 
} UNICODE_STRING,*PUNICODE_STRING;
#pragma pack()
__declspec(naked)   void sysFastCall()
{
	__asm
	{
	//	7C92E510 >  8BD4          MOV EDX,ESP
    //7C92E512    0F34            SYSENTER
	  	mov edx,esp
        __emit 0x0f
		__emit 0x34
	}
}
/*
77D28285   .  FF75 18       PUSH DWORD PTR SS:[EBP+18]                    ;  0
77D28288   .  FF75 E8       PUSH DWORD PTR SS:[EBP-18]                    ;  PU_LCatipn
77D2828B   .  FF75 F8       PUSH DWORD PTR SS:[EBP-8]                     ;  NULL
77D2828E   .  FF75 0C       PUSH DWORD PTR SS:[EBP+C]                     ;  0
77D28291   .  FF75 08       PUSH DWORD PTR SS:[EBP+8]                     ;  0
77D28294   .  E8 13450000   CALL USER32.77D2C7AC                          ;  NtUserFindWindow


*/
__declspec(naked) HWND  __stdcall My_FindWindow(
											   int p1,
											   int p2,
											   PUNICODE_STRING pu_classname,
											   PUNICODE_STRING pu_catption,
											   int p5)
{
	__asm 
	{
		MOV EAX,0x117A
        call sysFastCall
		RETN 0x14

	}
}
