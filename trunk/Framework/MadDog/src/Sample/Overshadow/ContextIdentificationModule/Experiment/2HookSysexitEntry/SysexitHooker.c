#include "SysexitHooker.h"

BOOLEAN StartRecording;
ULONG64 SyscallTimes;
ULONG32 pSyscallTimes;

ULONG32 OriginSysenterEIP[2];
ULONG32 OriginSysenterESP[2];
static KMUTEX g_Mutex;
static ULONG lock;
static ULONG plock;

ULONG32 SavedEax;
ULONG32 SavedEbx;
ULONG32 SavedEcx;
ULONG32 SavedEdx;
ULONG32 SavedEax2;
ULONG32 SavedEbx2;
ULONG32 SavedEcx2;
ULONG32 SavedEdx2;
void NTAPI IncreaseCounter()
{
	//KeWaitForSingleObject (&g_Mutex, Executive, KernelMode, FALSE, NULL);
	//SyscallTimes++;
	__asm{
		//inc dword ptr [pSyscallTimes]
	}
	//KeReleaseMutex (&g_Mutex, FALSE);
}
void __declspec(naked) CcFakeSysenterTrap()
{
	//ULONG32 currentProcessor;
	//ULONG32 targetESP;
	//ULONG32 targetEIP;
	
	__asm{
	loop_down:
		lock	bts dword ptr [plock], 0
		jb	loop_down; Acquire A Spin Lock
	}

	__asm{
		push eax
		push ebx
		push ecx
		push edx
	}

	//currentProcessor = KeGetCurrentProcessorNumber();
	//targetESP=OriginSysenterESP[currentProcessor];
	//targetEIP=OriginSysenterEIP[currentProcessor];
	
	//SavedEax2[currentProcessor] = SavedEax;
	//SavedEcx2[currentProcessor] = SavedEcx;
	//SavedEdx2[currentProcessor] = SavedEdx;

	SyscallTimes++;
	//IncreaseCounter();
	__asm{
		pop edx
		pop ecx
		pop ebx
		pop eax	
	}
	__asm{
			//mov ebx, currentProcessor
			//imul ebx,4
			//mov eax,[SavedEax2+ebx]
			//mov ecx,[SavedEcx2+ebx]
			//mov edx,[SavedEdx2+ebx]
	}

	__asm{
		pop edx
		pop ecx
		pop ebx
		pop eax
		lock	btr dword ptr [plock], 0; Release the Spin Lock
		jmp OriginSysenterEIP[0]
	}
}


void NTAPI CcSetupSysenterTrap(int cProcessorNumber)
{
	PVOID newSysenterESP;
	OriginSysenterEIP[cProcessorNumber] = MsrRead(MSR_IA32_SYSENTER_EIP);
	OriginSysenterESP[cProcessorNumber] = MsrRead(MSR_IA32_SYSENTER_ESP);
	HvmPrint(("In CcSetupSysenterTrap(): Core:%d, OriginSysenterEIP:%x\n",cProcessorNumber,OriginSysenterEIP[cProcessorNumber]));
	HvmPrint(("In CcSetupSysenterTrap(): Core:%d, OriginSysenterESP:%x\n",cProcessorNumber,OriginSysenterESP[cProcessorNumber]));
	newSysenterESP = ExAllocatePoolWithTag (NonPagedPool, 4 * PAGE_SIZE, 'ITL');
	MsrWrite(MSR_IA32_SYSENTER_EIP,&CcFakeSysenterTrap);
	MsrWrite(MSR_IA32_SYSENTER_ESP,newSysenterESP);
	HvmPrint(("In CcSetupSysenterTrap(): Core:%d, NewSysenterEntry:%x\n",cProcessorNumber,MsrRead(MSR_IA32_SYSENTER_EIP)));
	
	pSyscallTimes =&SyscallTimes;
	plock=&lock;
	
}
void NTAPI CcDestroySysenterTrap(int cProcessorNumber)
{
	MsrWrite(MSR_IA32_SYSENTER_EIP,OriginSysenterEIP[cProcessorNumber]);
	MsrWrite(MSR_IA32_SYSENTER_ESP,OriginSysenterESP[cProcessorNumber]);
}

NTSTATUS DriverUnload (
    PDRIVER_OBJECT DriverObject
)
{
	//CCHAR cProcessorNumber;
	//KIRQL OldIrql;
	////cProcessorNumber = 0;
	//for (cProcessorNumber = 0; cProcessorNumber < KeNumberProcessors; cProcessorNumber++) 
	//{
	//	KeSetSystemAffinityThread ((KAFFINITY) (1 << cProcessorNumber));
	//	OldIrql = KeRaiseIrqlToDpcLevel ();

	//	CcDestroySysenterTrap(cProcessorNumber);

	//	KeLowerIrql (OldIrql);
	//	KeRevertToUserAffinityThread ();

	//}

	//DbgPrint("Sysenter %d times\n",SyscallTimes);
    	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry (
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
)
{
   	NTSTATUS Status;
	LARGE_INTEGER MsrValue,MsrValue1,MsrValue2;
	MsrValue1.QuadPart = MsrRead(MSR_IA32_VMX_PINBASED_CTLS);
	DbgPrint("0x%llX\n0x%llX\n",MsrValue1.LowPart,MsrValue1.HighPart);
 //  	CCHAR cProcessorNumber;
	//KIRQL OldIrql;
	////cProcessorNumber = 0;
 //   	//__asm { int 3 }
	//__asm{
	//	and	dword ptr [plock], 0
	//}
	//for (cProcessorNumber = 0; cProcessorNumber < KeNumberProcessors; cProcessorNumber++) 
	//{
	//	KeSetSystemAffinityThread ((KAFFINITY) (1 << cProcessorNumber));		
	//	OldIrql = KeRaiseIrqlToDpcLevel ();

	//	CcSetupSysenterTrap(cProcessorNumber);

	//	KeLowerIrql (OldIrql);
	//	KeRevertToUserAffinityThread ();

	//}

	//KeRevertToUserAffinityThread ();

     	DriverObject->DriverUnload = DriverUnload;
    	return STATUS_SUCCESS;
}
