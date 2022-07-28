#include "detection.h"

NTSTATUS enumCallbacks(POBJECT_TYPE objectType)
{
	POBJECT_TYPE pObject = objectType;

	if (pObject == *PsProcessType)
		DebugLog("ObProcessCallbacks:\n");
	else if (pObject == *PsThreadType)
		DebugLog("ObThreadCallbacks:\n");

	__try
	{
		POBJECT_CALLBACK_ENTRY pCallbackEntry = *(POBJECT_CALLBACK_ENTRY*)((UCHAR*)pObject + 0xC8);
		POBJECT_CALLBACK_ENTRY Head = pCallbackEntry;

		while (TRUE)
		{
			PCALLBACK_ENTRY preOp = (PCALLBACK_ENTRY)pCallbackEntry->PreOperation;
			PCALLBACK_ENTRY postOp = (PCALLBACK_ENTRY)pCallbackEntry->PostOperation;

			PCHAR name = getNameFromAddr((UINT_PTR)preOp);
			
			if (pCallbackEntry->CallbackEntry->Version && preOp)
				DebugLog("module: %s\npre callback addr: %p, altitude: %wZ, version: %hu\n", name, preOp, &pCallbackEntry->CallbackEntry->Altitude, pCallbackEntry->CallbackEntry->Version);
			if (postOp)
				DebugLog("post callback: %p\n", postOp);

			pCallbackEntry = (POBJECT_CALLBACK_ENTRY)pCallbackEntry->CallbackList.Flink;
			if (Head == pCallbackEntry)
				break;
		}

		return STATUS_SUCCESS;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return STATUS_UNSUCCESSFUL;
	}
}

PCHAR getNameFromAddr(UINT_PTR addr)
{
	PVOID moduleList = getModuleList();
	PRTL_PROCESS_MODULES modules = (PRTL_PROCESS_MODULES)moduleList;

	for (SIZE_T i = 0; i < modules->NumberOfModules; i++)
	{
		PRTL_PROCESS_MODULE_INFORMATION modInfo = &modules->Modules[i];
		if (addr <= (UINT_PTR)modInfo->ImageBase + (UINT_PTR)modInfo->ImageSize && addr >= (UINT_PTR)modInfo->ImageBase)
			return (PCHAR)(modInfo->FullPathName + modInfo->OffsetToFileName);
	}

	return "unknown module, callback was registered from unsigned driver";
}

PVOID getModuleList()
{
	ULONG len = 0;

	ZwQuerySystemInformation(SystemModuleInformation, NULL, 0, &len);
	len += (1024 * 0xA);

	PVOID moduleList = ExAllocatePoolWithTag(PagedPool, len, 0);

	if (!NT_SUCCESS(ZwQuerySystemInformation(SystemModuleInformation, moduleList, len, &len)))
	{
		if (moduleList)
			ExFreePool(moduleList);

		return NULL;
	}

	return moduleList;
}