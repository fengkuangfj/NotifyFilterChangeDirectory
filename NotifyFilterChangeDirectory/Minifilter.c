/*++

Copyright (c) 1999 - 2002  Microsoft Corporation

Module Name:

passThrough.c

Abstract:

This is the main module of the passThrough miniFilter driver.
This filter hooks all IO operations for both pre and post operation
callbacks.  The filter passes through the operations.

Environment:

Kernel mode

--*/

#include "Minifilter.h"

PFLT_FILTER			gFilterHandle = NULL;
ULONG_PTR			OperationStatusCtx = 1;
ULONG				gTraceFlags = 0;
DRIVER_INITIALIZE	DriverEntry;

//
//  operation registration
//

CONST FLT_OPERATION_REGISTRATION Callbacks[] =
{
	{
		IRP_MJ_CREATE,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_CREATE_NAMED_PIPE,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_CLOSE,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_READ,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_WRITE,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_QUERY_INFORMATION,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_SET_INFORMATION,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_QUERY_EA,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_SET_EA,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_FLUSH_BUFFERS,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_QUERY_VOLUME_INFORMATION,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_SET_VOLUME_INFORMATION,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_DIRECTORY_CONTROL,
		0,
		PreDirectoryControl/*PtPreOperationPassThrough*/,
		PostDirectoryControl/*PtPostOperationPassThrough*/
	},

	{
		IRP_MJ_FILE_SYSTEM_CONTROL,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_DEVICE_CONTROL,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_INTERNAL_DEVICE_CONTROL,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_SHUTDOWN,
		0,
		PtPreOperationNoPostOperationPassThrough,
		NULL
	},                               //post operations not supported

	{
		IRP_MJ_LOCK_CONTROL,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_CLEANUP,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_CREATE_MAILSLOT,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_QUERY_SECURITY,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_SET_SECURITY,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_QUERY_QUOTA,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_SET_QUOTA,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_PNP,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_RELEASE_FOR_SECTION_SYNCHRONIZATION,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_ACQUIRE_FOR_MOD_WRITE,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_RELEASE_FOR_MOD_WRITE,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_ACQUIRE_FOR_CC_FLUSH,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_RELEASE_FOR_CC_FLUSH,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_FAST_IO_CHECK_IF_POSSIBLE,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_NETWORK_QUERY_OPEN,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_MDL_READ,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_MDL_READ_COMPLETE,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_PREPARE_MDL_WRITE,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_MDL_WRITE_COMPLETE,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_VOLUME_MOUNT,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_VOLUME_DISMOUNT,
		0,
		PtPreOperationPassThrough,
		PtPostOperationPassThrough
	},

	{
		IRP_MJ_OPERATION_END
	}
};

//
//  This defines what we want to filter with FltMgr
//

CONST FLT_REGISTRATION FilterRegistration =
{

	sizeof(FLT_REGISTRATION),         //  Size
	FLT_REGISTRATION_VERSION,           //  Version
	0,                                  //  Flags

	NULL,                               //  Context
	Callbacks,                          //  Operation callbacks

	PtUnload,                           //  MiniFilterUnload

	PtInstanceSetup,                    //  InstanceSetup
	PtInstanceQueryTeardown,            //  InstanceQueryTeardown
	PtInstanceTeardownStart,            //  InstanceTeardownStart
	PtInstanceTeardownComplete,         //  InstanceTeardownComplete

	NULL,                               //  GenerateFileName
	NULL,                               //  GenerateDestinationFileName
	NULL                                //  NormalizeNameComponent
};

NTSTATUS
PtInstanceSetup(
__in PCFLT_RELATED_OBJECTS FltObjects,
__in FLT_INSTANCE_SETUP_FLAGS Flags,
__in DEVICE_TYPE VolumeDeviceType,
__in FLT_FILESYSTEM_TYPE VolumeFilesystemType
)
{
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);
	UNREFERENCED_PARAMETER(VolumeDeviceType);
	UNREFERENCED_PARAMETER(VolumeFilesystemType);

	PAGED_CODE();

	PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,
		("PassThrough!PtInstanceSetup: Entered\n"));

	return STATUS_SUCCESS;
}

NTSTATUS
PtInstanceQueryTeardown(
__in PCFLT_RELATED_OBJECTS FltObjects,
__in FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
)
{
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);

	PAGED_CODE();

	PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,
		("PassThrough!PtInstanceQueryTeardown: Entered\n"));

	return STATUS_SUCCESS;
}

VOID
PtInstanceTeardownStart(
__in PCFLT_RELATED_OBJECTS FltObjects,
__in FLT_INSTANCE_TEARDOWN_FLAGS Flags
)
{
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);

	PAGED_CODE();

	PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,
		("PassThrough!PtInstanceTeardownStart: Entered\n"));
}

VOID
PtInstanceTeardownComplete(
__in PCFLT_RELATED_OBJECTS FltObjects,
__in FLT_INSTANCE_TEARDOWN_FLAGS Flags
)
{
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);

	PAGED_CODE();

	PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,
		("PassThrough!PtInstanceTeardownComplete: Entered\n"));
}

NTSTATUS
DriverEntry(
__in PDRIVER_OBJECT DriverObject,
__in PUNICODE_STRING RegistryPath
)
{
	NTSTATUS status;

	UNREFERENCED_PARAMETER(RegistryPath);

	PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,
		("PassThrough!DriverEntry: Entered\n"));

	//
	//  Register with FltMgr to tell it our callback routines
	//

	status = FltRegisterFilter(DriverObject,
		&FilterRegistration,
		&gFilterHandle);

	ASSERT(NT_SUCCESS(status));

	if (NT_SUCCESS(status)) {

		//
		//  Start filtering i/o
		//

		status = FltStartFiltering(gFilterHandle);

		if (!NT_SUCCESS(status)) {

			FltUnregisterFilter(gFilterHandle);
		}
	}

	return status;
}

NTSTATUS
PtUnload(
__in FLT_FILTER_UNLOAD_FLAGS Flags
)
{
	UNREFERENCED_PARAMETER(Flags);

	PAGED_CODE();

	PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,
		("PassThrough!PtUnload: Entered\n"));

	FltUnregisterFilter(gFilterHandle);

	return STATUS_SUCCESS;
}


FLT_PREOP_CALLBACK_STATUS
PtPreOperationPassThrough(
__inout PFLT_CALLBACK_DATA Data,
__in PCFLT_RELATED_OBJECTS FltObjects,
__deref_out_opt PVOID *CompletionContext
)
{
	NTSTATUS status;

	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(CompletionContext);

	PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,
		("PassThrough!PtPreOperationPassThrough: Entered\n"));

	//
	//  See if this is an operation we would like the operation status
	//  for.  If so request it.
	//
	//  NOTE: most filters do NOT need to do this.  You only need to make
	//        this call if, for example, you need to know if the oplock was
	//        actually granted.
	//

	if (PtDoRequestOperationStatus(Data)) {

		status = FltRequestOperationStatusCallback(Data,
			PtOperationStatusCallback,
			(PVOID)(++OperationStatusCtx));
		if (!NT_SUCCESS(status)) {

			PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
				("PassThrough!PtPreOperationPassThrough: FltRequestOperationStatusCallback Failed, status=%08x\n",
				status));
		}
	}

	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

VOID
PtOperationStatusCallback(
__in PCFLT_RELATED_OBJECTS FltObjects,
__in PFLT_IO_PARAMETER_BLOCK ParameterSnapshot,
__in NTSTATUS OperationStatus,
__in PVOID RequesterContext
)
{
	UNREFERENCED_PARAMETER(FltObjects);

	PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,
		("PassThrough!PtOperationStatusCallback: Entered\n"));

	PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
		("PassThrough!PtOperationStatusCallback: Status=%08x ctx=%p IrpMj=%02x.%02x \"%s\"\n",
		OperationStatus,
		RequesterContext,
		ParameterSnapshot->MajorFunction,
		ParameterSnapshot->MinorFunction,
		FltGetIrpName(ParameterSnapshot->MajorFunction)));
}

FLT_POSTOP_CALLBACK_STATUS
PtPostOperationPassThrough(
__inout PFLT_CALLBACK_DATA Data,
__in PCFLT_RELATED_OBJECTS FltObjects,
__in_opt PVOID CompletionContext,
__in FLT_POST_OPERATION_FLAGS Flags
)
{
	UNREFERENCED_PARAMETER(Data);
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(CompletionContext);
	UNREFERENCED_PARAMETER(Flags);

	PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,
		("PassThrough!PtPostOperationPassThrough: Entered\n"));

	return FLT_POSTOP_FINISHED_PROCESSING;
}

FLT_PREOP_CALLBACK_STATUS
PtPreOperationNoPostOperationPassThrough(
__inout PFLT_CALLBACK_DATA Data,
__in PCFLT_RELATED_OBJECTS FltObjects,
__deref_out_opt PVOID *CompletionContext
)
{
	UNREFERENCED_PARAMETER(Data);
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(CompletionContext);

	PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,
		("PassThrough!PtPreOperationNoPostOperationPassThrough: Entered\n"));

	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

BOOLEAN
PtDoRequestOperationStatus(
__in PFLT_CALLBACK_DATA Data
)
{
	PFLT_IO_PARAMETER_BLOCK iopb = Data->Iopb;

	//
	//  return boolean state based on which operations we are interested in
	//

	return (BOOLEAN)

		//
		//  Check for oplock operations
		//

		(((iopb->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) &&
		((iopb->Parameters.FileSystemControl.Common.FsControlCode == FSCTL_REQUEST_FILTER_OPLOCK) ||
		(iopb->Parameters.FileSystemControl.Common.FsControlCode == FSCTL_REQUEST_BATCH_OPLOCK) ||
		(iopb->Parameters.FileSystemControl.Common.FsControlCode == FSCTL_REQUEST_OPLOCK_LEVEL_1) ||
		(iopb->Parameters.FileSystemControl.Common.FsControlCode == FSCTL_REQUEST_OPLOCK_LEVEL_2)))

		||

		//
		//    Check for directy change notification
		//

		((iopb->MajorFunction == IRP_MJ_DIRECTORY_CONTROL) &&
		(iopb->MinorFunction == IRP_MN_NOTIFY_CHANGE_DIRECTORY))
		);
}

FLT_PREOP_CALLBACK_STATUS
PreDirectoryControl(
__inout PFLT_CALLBACK_DATA Data,
__in PCFLT_RELATED_OBJECTS FltObjects,
__deref_out_opt PVOID *CompletionContext
)
{
	NTSTATUS status;

	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(CompletionContext);

	PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,
		("PassThrough!PtPreOperationPassThrough: Entered\n"));

	//
	//  See if this is an operation we would like the operation status
	//  for.  If so request it.
	//
	//  NOTE: most filters do NOT need to do this.  You only need to make
	//        this call if, for example, you need to know if the oplock was
	//        actually granted.
	//

	if (PtDoRequestOperationStatus(Data)) {

		status = FltRequestOperationStatusCallback(Data,
			PtOperationStatusCallback,
			(PVOID)(++OperationStatusCtx));
		if (!NT_SUCCESS(status)) {

			PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
				("PassThrough!PtPreOperationPassThrough: FltRequestOperationStatusCallback Failed, status=%08x\n",
				status));
		}
	}

	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS
PostDirectoryControl(
__inout PFLT_CALLBACK_DATA Data,
__in PCFLT_RELATED_OBJECTS FltObjects,
__in_opt PVOID CompletionContext,
__in FLT_POST_OPERATION_FLAGS Flags
)
{
	UNREFERENCED_PARAMETER(Data);
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(CompletionContext);
	UNREFERENCED_PARAMETER(Flags);

	PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,
		("PassThrough!PtPostOperationPassThrough: Entered\n"));

	return FLT_POSTOP_FINISHED_PROCESSING;
}
