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

class VecSynthProvider:
    def __init__(self, valobj, internal_dict):
        self.valobj = valobj
        self.data = valobj.GetChildMemberWithName("m_pData")
        self.size = valobj.GetChildMemberWithName("m_size").GetValueAsSigned()

    def num_children(self):
        return int(self.size)

    def get_child_at_index(self, idx):
        element_type = self.data.GetType().GetPointeeType()
        offset = idx * element_type.GetByteSize()
        return self.data.CreateChildAtOffset(f"[{idx}]", offset, element_type)

    def get_child_index(self, name):
        if name.startswith('[') and name.endswith(']'):
            return int(name[1:-1])
        return -1

def __lldb_init_module(debugger, internal_dict):
    debugger.HandleCommand(
            'type summary add --python-function adt_formatters.StringView_SummaryProvider adt::StringView'
    )
    print("Registered custom formatter for adt::StringView.")

    debugger.HandleCommand(
            'type summary add --python-function adt_formatters.StringView_SummaryProvider adt::String'
    )
    print("Registered custom formatter for adt::String.")

    debugger.HandleCommand(
        'type synthetic add -l adt_formatters.VecSynthProvider adt::Vec<.*> --regex'
    )
    print("Registered custom formatter for adt::Vec.")
