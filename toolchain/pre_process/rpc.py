#!/usr/bin/env python3
import os
import re
import sys
from pathlib import Path
from dataclasses import dataclass
from typing import List, Optional, Tuple, Dict

SCRIPT_DIR = Path(__file__).resolve().parent
ROOT_DIR = SCRIPT_DIR.parent.parent

# TYPE CONVERSIONS
TYPE_MAP = {
    "bool" : "bool",
    "uint32": "uint32_t",
    "uint64": "uint64_t",
    "int32": "int32_t",
    "int64": "int64_t",
    "string": "mstring",#"const char*",
    "void": "void"
}

PUSH_FUNC_MAP = {
    "bool": "push_bool",
    "uint32": "push_uint32",
    "uint64": "push_uint64",
    "int32": "push_int32",
    "int64": "push_int64",
    "mstring": "push_string",
}

GET_FUNC_MAP = {
    "bool": "get_bool",
    "uint32": "get_uint32",
    "uint64": "get_uint64",
    "int32": "get_int32",
    "int64": "get_int64",
    "mstring": "get_string",
}

RPC_FLAGS_MAP = {
    "urgent": "(size_t)RPCMEssageFlags::URGENT",
    "oneway": "(size_t)RPCMEssageFlags::ONE_WAY",
    "descriptor_as_data": "(size_t)RPCMEssageFlags::DESCRIPTOR_AS_DATA",
}

CPP_TO_IDL = {
    "bool": "bool",
    "uint32_t": "uint32",
    "uint64_t": "uint64",
    "int32_t": "int32",
    "int64_t": "int64",
    "string": "mstring",
}

# CLASSES
@dataclass
class Argument:
    type_idl: str
    name: str

    @property
    def type_cpp(self) -> str:
        return TYPE_MAP.get(self.type_idl, self.type_idl)

    @property
    def push_func(self) -> str:
        return PUSH_FUNC_MAP.get(self.type_idl, "")

    @property
    def get_func(self) -> str:
        return GET_FUNC_MAP.get(self.type_idl, "")

@dataclass
class RpcMethod:
    name: str
    args: List[Argument]
    return_types: List[str]
    flags: List[str]

    @property
    def is_multireturn(self) -> bool:
        return len(self.return_types) > 1

    @property
    def cpp_return_type(self) -> str:
        if self.is_multireturn:
            return f"{self.name}_result_t"

        # No return or explicit void
        if not self.return_types:
            return "void"
        rt = self.return_types[0].strip()

        if rt.lower() == "void":
            return "void"

        if rt in TYPE_MAP:
            return TYPE_MAP[rt]
        if rt.lower() in TYPE_MAP:
            return TYPE_MAP[rt.lower()]

        return rt

@dataclass
class RpcService:
    name: str
    package: str
    version: str
    path_raw: str
    clean_path: str
    functions: List[RpcMethod]
    source_file: Path
    class_header: Optional[str] = None
    class_name: Optional[str] = None
    service_pattern: Optional[str] = None
    namespace: Optional[str] = None

# HELPERS
def log(msg):
    print(f"[rpcgen] {msg}")

def idl_to_cpp(idl_type):
    return TYPE_MAP.get(idl_type, idl_type)

def normalize_pointer_spacing(t: str) -> str:
    """
    Normalize spacing for pointer tokens so that 'char *', 'char * const' etc.
    become 'char*' (keeping the '*' attached). Preserve '*' so pointer information
    can be detected by cpp_type_to_idl.
    """
    if not t:
        return t
    s = t.strip()
    # Remove extra spaces around '*' and collapse multiple spaces
    s = re.sub(r'\s*\*\s*', '*', s)
    s = re.sub(r'\s+', ' ', s)
    return s.strip()

def cpp_type_to_idl(t: str) -> str:

    # Get the base type
    t = t.strip()
    # Keep pointer '*' information — only remove the 'const' keyword but preserve '*'
    t = re.sub(r'\bconst\b', 'const', t)  # placeholder to ensure word boundary usage
    t = re.sub(r'\bconst\b\s*', '', t)    # remove 'const' but don't touch '*'
    t = t.replace('&', '').strip()
    t = normalize_pointer_spacing(t)

    # Quick direct mapping first (handles 'char*', 'const void*', etc.)
    if t in CPP_TO_IDL:
        return CPP_TO_IDL[t]

    # string heuristics: char*/std::string
    if re.search(r'char\*$', t) or re.search(r'\bstd::string\b', t) or t.endswith("string"):
        return "mstring"
    if "string" in t:
        return "mstring"

    # integer patterns
    if re.match(r'^(uint32(_t)?|unsigned\s+int)$', t) or 'uint32_t' in t:
        return "uint32"
    if re.match(r'^(uint64(_t)?|unsigned\s+long)$', t) or 'uint64_t' in t:
        return "uint64"
    if re.match(r'^(int32(_t)?|int)$', t) or 'int32_t' in t:
        return "int32"
    if re.match(r'^(int64(_t)?|long)$', t) or 'int64_t' in t:
        return "int64"

    # void or empty -> void
    if t == '' or t.lower() == 'void':
        return "void"

    return t

def sanitize_cpp_type(t: str) -> str:
    if not t:
        return "void"
    return t.strip()

def arg_decl_list(args: List[Argument]) -> str:
    return ", ".join([f"{idl_to_cpp(a.type_idl)} {a.name}" for a in args])

def start_header(lines: List[str], gaurd: str):
    lines.append("#ifndef " + gaurd)
    lines.append("#define " + gaurd)
    lines.append("")

def end_header(lines: List[str], gaurd: str):
    lines.append("")
    lines.append("#endif // " + gaurd)

def add_lines(line: str, files: List[List[str]]):
    for f in files:
        f.append(line)

def extract_arguments(function: RpcMethod, lines: List[str], get: bool = True) -> List[str]:

    access = "->" if get else "."

    call_args = []
    for i, arg in enumerate(function.args):

        # Get the operation
        fn = GET_FUNC_MAP.get(arg.type_idl) if get else PUSH_FUNC_MAP.get(arg.type_idl)
        fn_param = i if get else arg.name
        fn_result = f" {idl_to_cpp(arg.type_idl)} {arg.name} = " if get else ""

        # Get the arg
        if fn:
            lines.append(f"    {fn_result}_args{access}{fn}({fn_param});")
            call_args.append(arg.name)
            continue

        # Cant get the arg easily, fallback to blob
        if get:
            lines.append(f"     {idl_to_cpp(arg.type_idl)} {arg.name} = ({idl_to_cpp(arg.type_idl)})_args->get_blob({i}); // unsupported arg; fallback to blob")
        else:
            lines.append(f"    _args.push_blob(&{fn_param}, sizeof({idl_to_cpp(arg.type_idl)})); // unsupported arg; fallback to blob")

        call_args.append(arg.name)
    return call_args


# PARSERS
def parse_args(args_str: str) -> List[Argument]:
    """Parses 'type1 name1, type2 name2' into a list of Argument objects."""

    # No arguments
    args = []
    if not args_str or not args_str.strip():
        return args

    # Split by commas and parse each
    parts = [p.strip() for p in args_str.split(',') if p.strip()]
    for part in parts:

        # Split by spaces
        tokens = part.split()
        if len(tokens) >= 2:
            type_tok = " ".join(tokens[:-1])
            name_tok = tokens[-1]
            idl = cpp_type_to_idl(type_tok)
            args.append(Argument(type_idl=idl, name=name_tok))
        elif len(tokens) == 1:
            # only a type, create auto name
            type_tok = tokens[0]
            idl = cpp_type_to_idl(type_tok)
            args.append(Argument(type_idl=idl, name="arg0"))
    return args

def parse_class_header(header_path: Path, class_name: str) -> Dict[str, Tuple[str, List[Tuple[str,str]]]]:
    """
    Read header and extract virtual function declarations for class_name.
    Returns a dict: function_name -> (return_type_cpp, [(param_type, param_name), ...])
    """

    # Try open the header
    out = {}
    try:
        text = header_path.read_text()
    except Exception as e:
        raise ValueError(f"Could not read header file {header_path}: {e}")

    # Try to get the lines inside the class
    class_re = re.compile(r'\bclass\s+' + re.escape(class_name) + r'\b([^;{]*)\{', re.MULTILINE)
    class_lines = class_re.search(text)
    if not class_lines:
        raise ValueError(f"Could not find class {class_name} in {header_path}")

    # Find the end of the class block
    start = class_lines.start()
    brace_level = 0
    end = None
    for i in range(start, len(text)):
        if text[i] == '{':
            brace_level += 1
            if brace_level == 1:
                block_start = i+1
        elif text[i] == '}':
            brace_level -= 1
            if brace_level == 0:
                end = i
                break
    if end is None:
        raise ValueError(f"Could not find end of class {class_name} in {header_path}")
    class_block = text[block_start:end]

    # Find all the virtual functions (including =0, const, override)
    function_re = re.compile(r'virtual\s+([^\(\);]+?)\s+(\w+)\s*\(([^\)]*)\)\s*(?:const\s*)?(?:=\s*0\s*)?(?:override\s*)?;', re.MULTILINE)
    for function_line in function_re.finditer(class_block):

        # Split the structure
        return_type     = function_line.group(1).strip()
        name            = function_line.group(2).strip()
        params          = function_line.group(3).strip()

        # No params
        if not params:
            out[name] = (return_type, [])
            continue

        # Parse each param
        param_list = []
        for part in [p.strip() for p in params.split(',') if p.strip()]:

            # Extract type and name
            tokens = part.split()
            if len(tokens) >= 2:
                p_type = " ".join(tokens[:-1])
                p_name = tokens[-1]
            else:
                p_type = tokens[0]
                p_name = "arg0"

            # Normalise type: remove 'const' but keep pointer '*' attached
            p_type = re.sub(r'\bconst\b', '', p_type).strip()
            p_type = normalize_pointer_spacing(p_type)
            param_list.append((p_type, p_name))

        out[name] = (return_type, param_list)
    return out

def parse_rpc_file(file_path: Path) -> RpcService:
    content = file_path.read_text()
    functions = []

    # Get the metadata
    package = re.search(r'^\s*package\s+([\w\.]+);?', content, re.MULTILINE)
    version = re.search(r'^\s*version\s+([\d\.]+);?', content, re.MULTILINE)
    output_path = re.search(r'^\s*path\s+([\S]+);?', content, re.MULTILINE)

    # Clean the metadata
    path_raw = output_path.group(1) if output_path else "/"
    clean_path = path_raw.strip('/').replace('\r', '').replace('\n', '')

    # Class servers can have a header and namespace
    class_header_match = re.search(r'^\s*class\s*<([^>]+)>', content, re.MULTILINE)
    class_header = class_header_match.group(1).strip() if class_header_match else None
    namespace = re.search(r'^\s*namespace\s+([\w:]+);?', content, re.MULTILINE)

    # Check for a class service: "service class ClassName service_pattern { ... }"
    service_re = re.compile(r'^\s*service\s+class\s+(\w+)\s+([\w\{\}_]+)\s*\{(.*?)\}', re.MULTILINE | re.DOTALL)
    service_class = service_re.search(content)
    if  service_class:
        class_name      = service_class.group(1)
        service_pattern = service_class.group(2)
        body            = service_class.group(3)
    else:

        # Check for a simple service: "service service_pattern { ... }"
        srv_simple = re.search(r'^\s*service\s+(\w+)\s*\{(.*?)\}', content, re.MULTILINE | re.DOTALL)
        if srv_simple:
            class_name      = None
            service_pattern = srv_simple.group(1)
            body            = srv_simple.group(2)
        else:
            raise ValueError("No valid service definition found in RPC file.")


    # RPC functions can either be provided or fetched from the class
    rpc_pattern_full = re.compile(r'^\s*rpc\s+(?:\[(.*?)\])?\s*(?:\((.*?)\)|(\w+))\s+(\w+)\s*\((.*?)\)\s*;?', re.MULTILINE | re.DOTALL)
    rpc_pattern_auto = re.compile(r'^\s*rpc\s+(\w+)\s*;\s*$', re.MULTILINE)

    # Fine all full definitions
    for match in rpc_pattern_full.finditer(body):

        # Extract the flags, return types, name, and args
        flags_str, multiret_str, singleret_str, function_name, args_str = match.groups()

        # Parse the extracted data
        flags = [f.strip() for f in flags_str.split(',')] if flags_str else []
        arguments = parse_args(args_str)

        # Get the return types of the function
        if multiret_str:
            return_types = [t.strip() for t in multiret_str.split(',') if t.strip()]
        else:
            return_types = [singleret_str.strip()] if singleret_str else ["void"]

        functions.append(RpcMethod(name=function_name, args=arguments, return_types=return_types, flags=flags))

    # Find all auto definitions that weren't already defined (will be filled from class header later)
    for match in rpc_pattern_auto.finditer(body):
        function_name = match.group(1)
        if function_name in functions:
            continue
        functions.append(RpcMethod(name=function_name, args=[], return_types=["void"], flags=[]))

    return RpcService(
        name=class_name if class_name else service_pattern,
        package=package.group(1) if package else "",
        version=version.group(1) if version else "",
        path_raw=path_raw,
        clean_path=clean_path,
        functions=functions,
        source_file=file_path,
        class_header=class_header,
        class_name=class_name,
        service_pattern=service_pattern,
        namespace=namespace.group(1) if namespace else ""
    )

# GENERATORS

def generate_service_class_files(service: RpcService, output_inc: Path, output_src: Path):
    """
    Generates:
      - <ClassName>_server.h,cpp
      - <ClassName>_client.h,cpp

    Reading signatures from service.class_header if present.
    """
    class_name = service.class_name
    header_file = service.class_header
    if not class_name:
        raise ValueError("Service class generation requested but no class name specified.")

    # Make sure header file exists
    header_path = service.source_file.parent / "include" / header_file
    if header_file:
        if not header_path.exists():
            raise ValueError(f"Header file '{header_file}' not found for service class '{class_name}'.")

    # Parse the header to get the function signatures
    parsed_functions = {}
    if header_path:
        parsed_functions = parse_class_header(header_path, class_name)

    # Update all the functions in the rpc with the correct signatures
    for function in service.functions:

        # Function not parsed from header, skip
        if function.name not in parsed_functions:
            continue

        ret_type_cpp, params = parsed_functions[function.name]

        # Convert the c++ types into idl types
        args = []
        for (ptype, pname) in params:
            args.append(Argument(type_idl=cpp_type_to_idl(ptype), name=pname))

        # Save the idl typed info back into the function
        function.args = args
        function.return_types = [sanitize_cpp_type(ret_type_cpp)]

    # Files to be generated
    class_lower = class_name.lower()
    srv_h = output_inc / f"{class_lower}_server.h"
    cli_h = output_inc / f"{class_lower}_client.h"
    srv_cpp = output_src / f"{class_lower}_server.cpp"
    cli_cpp = output_src / f"{class_lower}_client.cpp"

    # File data
    srv_h_lines = []
    cli_h_lines = []
    srv_cpp_lines = []
    cli_cpp_lines = []

    # Make the output directories
    output_inc.mkdir(parents=True, exist_ok=True)
    output_src.mkdir(parents=True, exist_ok=True)

    # Function so that collapsable in editor
    def generate_service_class_headers():

        # Shared lines
        both = [srv_h_lines, cli_h_lines]

        # Header gaurds
        srv_gaurd = "{0}_{1}_SERVER_H".format(service.namespace.upper().replace("::", "_"), class_name.upper())
        client_gaurd = "{0}_{1}_CLIENT_H".format(service.namespace.upper().replace("::", "_"), class_name.upper())
        start_header(srv_h_lines, srv_gaurd)
        start_header(cli_h_lines, client_gaurd)

        # Setup includes
        add_lines("#include <ipc/rpc.h>", both)
        add_lines("#include <string.h>", both)
        if header_file: add_lines(f'#include <{header_file}>', both)
        add_lines("", both)

        # Add namespace if needed
        add_lines("using mstring = MaxOS::string;", both)
        add_lines("", both)
        if service.namespace != "": add_lines(f"namespace {service.namespace} {{", both)
        add_lines("", both)

        # Define the server
        srv_h_lines.extend([
            f"   class {class_name}Server" + " {",
            "      private:",
            f"           static {class_name}Server* s_instance;",
            f"           {class_name}* m_driver;",
            "      public:",
            f"           {class_name}Server({class_name}* driver);",
            f"           ~{class_name}Server();",
            "",
            f"           static {class_name}Server* server();",
            f"           static {class_name}* driver();",
            "",
            "            void start(mstring id);",
            "    };",
            ""

        ])

        # Define the client
        cli_h_lines.extend([
            f"   class {class_name}Client : public {class_name} " + "{",
            "      private:",
            "            mstring m_id;",
            "      public:",
            f"          explicit {class_name}Client(mstring id);",
            f"          ~{class_name}Client();",
            ""
        ])

        # Add the function declarations (override)
        for function in service.functions:

            # Get the return type and args
            ret = function.cpp_return_type
            args = arg_decl_list(function.args)
            cli_h_lines.append(f"           virtual {ret} {function.name}({args.strip()});")

        cli_h_lines.append("    };")
        add_lines("", both)

        # Close the files
        if service.namespace != "": add_lines("} // namespace " + service.namespace, both)
        end_header(srv_h_lines, srv_gaurd)
        end_header(cli_h_lines, client_gaurd)
    generate_service_class_headers()

    # Function so that collapsable in editor TODO: move to separate functions like service class headers
    def generate_service_class_files():

        # Shared lines
        both = [srv_cpp_lines, cli_cpp_lines]

        # Path to the headers
        path = f"{service.clean_path}/" if service.clean_path else ""

        # Setup header stuff
        srv_cpp_lines.append(f'#include <{path}{srv_h.name}>')
        cli_cpp_lines.append(f'#include <{path}{cli_h.name}>')
        add_lines("", both)
        add_lines("using namespace MaxOS::KPI::ipc;", both)

        # Namespace
        if service.namespace != "":
            add_lines(f"namespace {service.namespace} {{", both)
            add_lines("", both)

        # Server: static instance
        srv_cpp_lines.append(f"{class_name}Server* {class_name}Server::s_instance = nullptr;")
        srv_cpp_lines.append("")

        # Server: forward declare wrappers
        for function in service.functions:
            srv_cpp_lines.append(f"static void {function.name}_wrapper(ArgList* args, ArgList* returns);")
        srv_cpp_lines.append("")

        # Server: Class
        srv_cpp_lines.extend([
            f"{class_name}Server::{class_name}Server({class_name}* driver) {{",
            "",
            "      // Setup references",
            "      s_instance = this;",
            "      m_driver = driver;",
            "",
            "      // Register RPC calls",
        ])
        for function in service.functions:
            srv_cpp_lines.append(f'      register_function("{function.name}", {function.name}_wrapper);')
        srv_cpp_lines.extend([
            "}",
            "",
            f"{class_name}Server::~{class_name}Server() = default;",
            "",
            f"{class_name}Server* {class_name}Server::server() {{",
            "    if (s_instance == nullptr)",
            "        return nullptr;",
            "",
            "    return s_instance;",
            "}",
            "",
            f"{class_name}* {class_name}Server::driver() {{",
            "    if (s_instance == nullptr)",
            "        return nullptr;",
            "",
            "    return s_instance->m_driver;",
            "}",
            "",
            f"void {class_name}Server::start(mstring id) {{",
            "    mstring server_name = id;",
            "    rpc_server_loop(server_name.c_str());",
            "}",
            ""


        ])

        # Server: wrappers
        for function in service.functions:

            # Define the function
            srv_cpp_lines.append(f"static void {function.name}_wrapper(ArgList* _args, ArgList* _returns) {{")
            srv_cpp_lines.append(f"")
            srv_cpp_lines.append(f"     // Get the driver")
            srv_cpp_lines.append(f"     {class_name}* driver = {class_name}Server::driver();")
            srv_cpp_lines.append(f"     if (driver == nullptr) return;")
            srv_cpp_lines.append(f"")

            # Extract the args from the args list
            call_args = extract_arguments(function, srv_cpp_lines, get=True)
            srv_cpp_lines.append("")

            # Void function
            if function.cpp_return_type == "void" or (len(function.return_types) == 1 and function.return_types[0].lower() == "void"):
                srv_cpp_lines.append(f"     driver->{function.name}({', '.join(call_args)});")
                srv_cpp_lines.append("}")
                srv_cpp_lines.append("")
                continue

            # Get the return value
            return_value = function.cpp_return_type
            push_fn = PUSH_FUNC_MAP.get(cpp_type_to_idl(return_value))
            if not push_fn:
                raise ValueError(f"Unsupported return type '{return_value}' for function '{function.name}' in service class generation.")

            # Call the function and push the return value
            srv_cpp_lines.append(f"    {return_value} _ret = driver->{function.name}({', '.join(call_args)});")
            srv_cpp_lines.append(f"    _returns->{push_fn}(_ret);")
            srv_cpp_lines.append("}")
            srv_cpp_lines.append("")

        # Client: class (TODO: build constructor with pattern
        cli_cpp_lines.append(f"{class_name}Client::{class_name}Client(mstring id) {{")
        cli_cpp_lines.append(f'   m_id = (mstring)"driver_" + id;')
        cli_cpp_lines.append("}")
        cli_cpp_lines.append("")
        cli_cpp_lines.append(f"{class_name}Client::~{class_name}Client() = default;")
        cli_cpp_lines.append("")

        # Client: functions
        for function in service.functions:

            # Get return type and args
            ret = function.cpp_return_type
            args_decl = arg_decl_list(function.args)

            # Define the function
            cli_cpp_lines.append(f"{ret} {class_name}Client::{function.name}({args_decl.strip()}) " + "{")
            cli_cpp_lines.append("    ArgList _args;")
            cli_cpp_lines.append("    ArgList _returns;")
            cli_cpp_lines.append("")

            #  Call the RPC
            call_args = extract_arguments(function, cli_cpp_lines, get=False)
            if len(call_args): cli_cpp_lines.append("")
            cli_cpp_lines.append(f'    rpc_call(m_id.c_str(), "{function.name}", &_args, &_returns, 0);')

            # No return
            if function.cpp_return_type == "void" or (len(function.return_types) == 1 and function.return_types[0].lower() == "void"):
                cli_cpp_lines.append("    return;")
                cli_cpp_lines.append("}")
                cli_cpp_lines.append("")
                continue

            # Get the return value
            return_value = function.cpp_return_type
            get_fn = GET_FUNC_MAP.get(cpp_type_to_idl(return_value))
            if not get_fn:
                raise ValueError(f"Unsupported return type '{return_value}' for function '{function.name}' in service class generation.")

            # Return the value
            cli_cpp_lines.append(f"    return _returns.{get_fn}(0);")
            cli_cpp_lines.append("}")
            cli_cpp_lines.append("")

        # Close namespace
        if service.namespace != "":
            add_lines("} // namespace " + service.namespace, both)

    generate_service_class_files()

    # Write the files
    srv_h.write_text("\n".join(srv_h_lines))
    cli_h.write_text("\n".join(cli_h_lines))
    srv_cpp.write_text("\n".join(srv_cpp_lines))
    cli_cpp.write_text("\n".join(cli_cpp_lines))

    log(f"Generated service-class files for {class_name}")

def generate_types_header(service: RpcService, output_path: Path):

    lines = []

    # Header start
    header = f"{service.name.upper()}_TYPES_H"
    start_header(lines, header)

    # Includes
    lines.extend([
        "#include <cstdint>",
        "#include <ipc/rpc.h>",
        "using mstring = MaxOS::string;",
        ""
    ])

    # Multi-return structs
    for function in service.functions:

        # Skip normal returns
        if not function.is_multireturn:
            continue

        # Create the struct
        lines.append(f"struct {function.name}_result_t {{")
        for i, r_type in enumerate(function.return_types):
            lines.append(f"    {idl_to_cpp(r_type)} v{i};")
        lines.append("};")

        lines.append("")

    # Save the file
    end_header(lines, header)
    output_path.write_text("\n".join(lines))

def generate_server_header(service: RpcService, output_path: Path, include_prefix: str):

    lines = []

    # Header guard
    header_guard = f"{service.name.upper()}_SERVER_H"
    start_header(lines, header_guard)

    # Includes and consistent function
    lines.extend([
        "#include <ipc/rpc.h>",
        f"#include <{include_prefix}{service.name}_types.h>",
        "using mstring = MaxOS::string;",
        "",
        f"void register_{service.name}_functions();",
        f"void run_{service.name}();",
        ""
    ])

    # Declare functions
    for function in service.functions:
        arg_str = ", ".join([f"{arg.type_cpp} {arg.name}" for arg in function.args])
        lines.append(f"{function.cpp_return_type} {function.name}({arg_str});")

    # Save the file
    end_header(lines, header_guard)
    output_path.write_text("\n".join(lines))

def generate_client_header(service: RpcService, output_path: Path, include_prefix: str):

    lines = []

    # Header guard
    header_guard = f"{service.name.upper()}_CLIENT_H"
    start_header(lines, header_guard)

    # Includes and consistent function
    lines.extend([
        "#include <ipc/rpc.h>",
        f"#include <{include_prefix}{service.name}_types.h>",
        "",
        "using namespace MaxOS::KPI::ipc;",
        "using mstring = MaxOS::string;",
        "",
        f"void wait_for_{service.name}_server();",
        ""
    ])

    # Declare functions
    for function in service.functions:
        arg_str = ", ".join([f"{arg.type_cpp} {arg.name}" for arg in function.args])
        lines.append(f"{function.cpp_return_type} {function.name}({arg_str});")

    # Save the file
    end_header(lines, header_guard)
    output_path.write_text("\n".join(lines))

def generate_server_source(service: RpcService, output_path: Path, include_prefix: str):

    # Includes
    lines = [
        f"#include <{include_prefix}{service.name}_server.h>",
        "",
        "using namespace MaxOS::KPI::ipc;",
        ""
    ]

    # Wrappers TODO: make into reusable function for both
    for function in service.functions:

        # Define the function
        lines.append(f"static void {function.name}_wrapper(ArgList* _args, ArgList* _returns) {{")

        # Unpack arguments
        call_args = extract_arguments(function, lines, get=True)

        # Call implementation
        call_str = f"{function.name}({', '.join(call_args)})"
        if function.is_multireturn:
            lines.append(f"    {function.cpp_return_type} _ret = {call_str};")
            for i, r_type in enumerate(function.return_types):
                push_fn = PUSH_FUNC_MAP.get(r_type)
                if push_fn:
                    lines.append(f"    _returns->{push_fn}(_ret.v{i});")
                else:
                    lines.append(f"    // TODO: Unsupported return type '{r_type}'")
        elif function.cpp_return_type == "void":
            lines.append(f"    {call_str};")
        else:
            lines.append(f"    {function.cpp_return_type} _ret = {call_str};")
            push_fn = PUSH_FUNC_MAP.get(function.return_types[0])
            if push_fn:
                lines.append(f"    _returns->{push_fn}(_ret);")
            else:
                lines.append(f"    // TODO: Unsupported return type '{function.return_types[0]}'")
        lines.append("}")
        lines.append("")

    # Server registerer
    lines.append(f"void register_{service.name}_functions() {{")
    for function in service.functions:
        lines.append(f'    register_function("{function.name}", {function.name}_wrapper);')
    lines.append("}")
    lines.append("")

    # Server loop
    lines.append(f"void run_{service.name}() {{")
    lines.append(f"    register_{service.name}_functions();")
    lines.append(f'    rpc_server_loop("{service.name}");')
    lines.append("}")


    # Save the file
    output_path.write_text("\n".join(lines))

def generate_client_source(service: RpcService, output_path: Path, include_prefix: str):

    # Includes and consistent functions
    lines = [
        f"#include <{include_prefix}{service.name}_client.h>",
        "",
        "using namespace MaxOS::KPI::ipc;",
        "",
        f"void wait_for_{service.name}_server() {{",
        f'    rpc_wait_for_server("{service.name}");',
        "}",
        ""
    ]

    # Declare functions
    for function in service.functions:

        # Define the function
        arg_decl = ", ".join([f"{arg.type_cpp} {arg.name}" for arg in function.args])
        lines.append(f"{function.cpp_return_type} {function.name}({arg_decl}) {{")
        lines.append("    ArgList _args;")
        lines.append("    ArgList _returns;")

        # Flags
        flag_mask_parts = [RPC_FLAGS_MAP.get(f, "0") for f in function.flags if f in RPC_FLAGS_MAP]
        flag_mask = " | ".join(flag_mask_parts) if flag_mask_parts else "0"

        # RPC Call
        call_args = extract_arguments(function, lines, get=False)
        lines.append(f'    rpc_call("{service.name}", "{function.name}", &_args, &_returns, {flag_mask});')

        # Handle returns
        if function.is_multireturn:
            init_list = []
            for i, r_type in enumerate(function.return_types):
                get_fn = GET_FUNC_MAP.get(r_type)
                if get_fn:
                    init_list.append(f"_returns.{get_fn}({i})")
                else:
                    init_list.append("/* unsupported */")
            lines.append(f"    return {function.cpp_return_type}{{ {', '.join(init_list)} }};")
        elif function.cpp_return_type != "void":
            get_fn = GET_FUNC_MAP.get(function.return_types[0])
            if get_fn:
                lines.append(f"    return _returns.{get_fn}(0);")
            else:
                lines.append(f"    return ({function.cpp_return_type})0; // Unsupported return")
        else:
            lines.append("    return;")
        lines.append("}")
        lines.append("")
    output_path.write_text("\n".join(lines))

# MAIN

def process_file(file_path: Path):

    # parse the file
    log(f"Processing {file_path}")
    try:
        service = parse_rpc_file(file_path)
    except Exception as e:
        log(f"[ERROR] Failed to parse {file_path}: {e}")
        return

    log(f"  Service class: {service.name}")
    log(f"  Path:    {service.path_raw}")

    # Directory setup
    folder = file_path.parent
    inc_dir = folder / "include" / service.clean_path
    src_dir = folder / "src" / service.clean_path
    inc_dir.mkdir(parents=True, exist_ok=True)
    src_dir.mkdir(parents=True, exist_ok=True)

    include_prefix = f"{service.clean_path}/" if service.clean_path else ""

    # If this is a service class directive (class header + service class ...), produce server/client classes
    if service.class_header and service.class_name:
        generate_service_class_files(service, inc_dir, src_dir)
        return

    # Otherwise fallback to the older RPC generation behavior
    log("  Generating standard RPC service files...")
    files = {
        "types_h": inc_dir / f"{service.name}_types.h",
        "srv_h":   inc_dir / f"{service.name}_server.h",
        "cli_h":   inc_dir / f"{service.name}_client.h",
        "srv_cpp": src_dir / f"{service.name}_server.cpp",
        "cli_cpp": src_dir / f"{service.name}_client.cpp",
    }

    generate_types_header(service, files["types_h"])
    generate_server_header(service, files["srv_h"], include_prefix)
    generate_client_header(service, files["cli_h"], include_prefix)
    generate_server_source(service, files["srv_cpp"], include_prefix)
    generate_client_source(service, files["cli_cpp"], include_prefix)

def main():
    log("Scanning for .rpc files...")
    rpc_files = list(ROOT_DIR.rglob("*.rpc"))

    # No files found
    if not rpc_files:
        log("No .rpc files found.")
        return

    # Process each file
    for f in rpc_files:
        process_file(f)

    log("Done.")

if __name__ == "__main__":
    main()
