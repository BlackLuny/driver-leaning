#ifndef HOOKNTOPENPROCESS
#define HOOKNTOPENPROCESS

int nNtOpenProcessAddr;
int nHookNtOpenProcessAddr;
int nHookNtOpenPrpcessJmp;
int nHookNtOpenPrpcessOldJmp;
int nObOpenObjectByPointerAddr;

__declspec(naked) void MyNtOpenProcess()
{
	__asm
	{
		push    dword ptr [ebp-38h]
		push    dword ptr [ebp-24h]
	}
	if(PanDuanProcessName("DNF.exe")||PanDuanProcessName("TenSafe.exe")||PanDuanProcessName("QQLogin.exe"))
	{
		__asm
		{
			//如果是DNF调用的
			jmp nHookNtOpenPrpcessOldJmp
		}
	}
	
	__asm
	{
		call nObOpenObjectByPointerAddr
		jmp nHookNtOpenPrpcessJmp
	}
	
}


void HookNtOpenProcess()
{
//	nNtOpenProcessAddr=GetSSDTFunctionAddr(122);
	nNtOpenProcessAddr=GetFunCtionAddr(L"NtOpenProcess");
	char code[7]={(char)0xff,(char)0x75,(char)0xc8,(char)0xff,(char)0x75,(char)0xdc,(char)0xe8};

	nHookNtOpenProcessAddr=SearchFeature(nNtOpenProcessAddr,code,7)-7;
	DbgPrint("nHookNtOpenProcessAddr=%x\n",nHookNtOpenProcessAddr);

	nHookNtOpenPrpcessJmp=nHookNtOpenProcessAddr+11;
	nHookNtOpenPrpcessOldJmp=nHookNtOpenProcessAddr+6;
	DbgPrint("nHookNtOpenPrpcessJmp=%x\n",nHookNtOpenPrpcessJmp);
	DbgPrint("nHookNtOpenPrpcessOldJmp=%x\n",nHookNtOpenPrpcessOldJmp);

	nObOpenObjectByPointerAddr=GetFunCtionAddr(L"ObOpenObjectByPointer");
	DbgPrint("nObOpenObjectByPointerAddr=%x\n",nObOpenObjectByPointerAddr);

	InLineHookEngine(nHookNtOpenProcessAddr,(int)MyNtOpenProcess);

}

void UnHookNtOpenProcess()
{
	char code[7]={(char)0xff,(char)0x75,(char)0xc8,(char)0xff,(char)0x75,(char)0xdc,(char)0xe8};
	
	UnInLineHookEngine(nHookNtOpenProcessAddr,code,5);
}

#endif