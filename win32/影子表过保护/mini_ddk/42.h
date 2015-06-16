#define VERSION_2K          50

#define VERSION_XP          51

#define VERSION_2K3         52
#define VERSION_XP64        52
#define VERSION_2K3_R2      52

#define VERSION_VISTA       60
#define VERSION_SERVER2008  60

#define VERSION_WIN7            61
#define VERSION_SERVER2008_R2   61
#pragma PAGECODE
DWORD GetVersion()
{  ULONG rtn=0;
	ULONG MajorVersion,MinorVersion,BuildNumber;
	PsGetVersion(&MajorVersion,&MinorVersion,&BuildNumber,NULL);
	rtn=MajorVersion;
	rtn=rtn *10;
	rtn+=MinorVersion;
	return rtn;
}

#pragma PAGECODE
DWORD Get_KeServiceDescriptorTableShadow_Addr()
{   DWORD KeServiceDescriptorTableShadow=0;
	DWORD Version=GetVersion();
	switch (Version  )
	{
        case VERSION_2K:
			  KeServiceDescriptorTableShadow=(DWORD)KeServiceDescriptorTable+0xE0;
		    break;
		case VERSION_2K3:
		    break;
		case VERSION_XP:
			  KeServiceDescriptorTableShadow=(DWORD)KeServiceDescriptorTable-0x40;
		    break;
		default:
			break;
		 
	}
	return KeServiceDescriptorTableShadow;

}
#pragma PAGECODE
VOID Show_SSDTShadowList()
{ 
	KdPrint(("Entry  Show_SSDTShadowList \n"));
	 
	DWORD TableBase=Get_KeServiceDescriptorTableShadow_Addr();
	TableBase=TableBase+0x10;//表基址
	DWORD TableCount=TableBase+8;//表函数 数量
	 
   
	DWORD count=*((PDWORD)TableCount);//函数数量
	KdPrint(("SSDT_Shadow Base=%x Count=%x\n",TableBase,count));
	//__asm  int 3
    PDWORD CFun_Addr=PDWORD(TableBase);//+=355
	CFun_Addr=PDWORD(*CFun_Addr);
	for (DWORD i=0;i<count;i++)
	{   
		 KdPrint(("\n %d=%x\n",i,*CFun_Addr ));
		CFun_Addr++;
		//__asm int 3
		 
	}
	 
}
HWND myh;
typedef BOOL (__stdcall *PNtUserDestroyWindow)(HWND hwnd);
PNtUserDestroyWindow Old_NtUserDestroyWindow,Cur_NtUserDestroyWindow;

#pragma PAGECODE
BOOL __stdcall My_NtUserDestroyWindow(HWND h)
{ 
	KdPrint(("h=%x \n",h));
	if (h==myh) 
	{
		KdPrint(("被保护窗口h=%x \n",h));
		return FALSE;
	}else	return Old_NtUserDestroyWindow(h);
}


#pragma PAGECODE
VOID SSDT_HOOK_NtUserDestroyWindow() //355
{ KdPrint(("Entry  Show_SSDTShadowList \n"));

DWORD TableBase=Get_KeServiceDescriptorTableShadow_Addr();
TableBase=TableBase+0x10;
DWORD TableCount=TableBase+8;


DWORD count=*((PDWORD)TableCount);
KdPrint(("SSDT_Shadow Base=%x Count=%x\n",TableBase,count));
//__asm  int 3
PDWORD CFun_Addr=PDWORD(TableBase);
CFun_Addr=PDWORD(*CFun_Addr);
CFun_Addr+=355;
Old_NtUserDestroyWindow=(PNtUserDestroyWindow)(*CFun_Addr);
KdPrint(("\n NtUserDestroyWindow当前地址=%x,%x \n",CFun_Addr,*CFun_Addr));
__asm //去掉页面保护
{
	cli
		mov eax,cr0
		and eax,not 10000h //and eax,0FFFEFFFFh
		mov cr0,eax

}

*CFun_Addr=(DWORD)(&My_NtUserDestroyWindow);
KdPrint(("\n NtUserDestroyWindow HOOK后地址=%x,%x \n",CFun_Addr,*CFun_Addr));

__asm 
{ 
	   mov     eax, cr0 
		or     eax, 10000h 
		mov     cr0, eax 
		sti 
}   

}



#pragma PAGECODE
VOID SSDT_UNHOOK_NtUserDestroyWindow() //335
{ 
	KdPrint(("Entry  Show_SSDTShadowList \n"));

DWORD TableBase=Get_KeServiceDescriptorTableShadow_Addr();
TableBase=TableBase+0x10;
DWORD TableCount=TableBase+8;


DWORD count=*((PDWORD)TableCount);
KdPrint(("SSDT_Shadow Base=%x Count=%x\n",TableBase,count));
//__asm  int 3
PDWORD CFun_Addr=PDWORD(TableBase);
CFun_Addr=PDWORD(*CFun_Addr);
CFun_Addr+=355;
__asm //去掉页面保护
{
	cli
		mov eax,cr0
		and eax,not 10000h //and eax,0FFFEFFFFh
		mov cr0,eax

}
*CFun_Addr=(DWORD)(Old_NtUserDestroyWindow);


__asm 
{ 
	mov     eax, cr0 
		or     eax, 10000h 
		mov     cr0, eax 
		sti 
}   
}