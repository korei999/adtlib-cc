import lldb

def StringView_SummaryProvider(valobj, internal_dict):
    data_ptr_obj = valobj.GetChildMemberWithName("m_pData")
    size_obj = valobj.GetChildMemberWithName("m_size")

    if not data_ptr_obj.IsValid() or not size_obj.IsValid():
        return "Invalid StringView object"

    length = size_obj.GetValueAsUnsigned(0)
    if length == 0:
        return '""'

    address = data_ptr_obj.GetValueAsUnsigned(0)
    if address == 0:
        return "nullptr"

    error = lldb.SBError()
    process = valobj.GetProcess()
    memory_bytes = process.ReadMemory(address, length, error)

    if error.Success():
        return f'({length}): "{memory_bytes.decode("utf-8")}"'
    else:
        return f"<error: {error.GetCString()}>"

def __lldb_init_module(debugger, internal_dict):
    debugger.HandleCommand(
            'type summary add --python-function adt_formatters.StringView_SummaryProvider adt::StringView'
    )
    print("Registered custom formatter for adt::StringView.")

    debugger.HandleCommand(
            'type summary add --python-function adt_formatters.StringView_SummaryProvider adt::String'
    )
    print("Registered custom formatter for adt::String.")
