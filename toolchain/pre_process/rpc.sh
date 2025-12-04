#!/usr/bin/env bash

#TODO: rename vars
#TODO: change methods to functions
#TODO: make shit less compilcated

SCRIPTDIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
source "$SCRIPTDIR/../MaxOS.sh"

ROOT="$(cd "$SCRIPTDIR/../../" && pwd)"

rpcgen "Scanning for .rpc files..."

# Remove leading/trailing whitespace
trim() { local v="$*"; v="${v#"${v%%[![:space:]]*}"}"; v="${v%"${v##*[![:space:]]}"}"; printf '%s' "$v"; }

# Map IDL to C++ types, push/get functions
idl_to_cpp() {
    case "$1" in
        uint32) printf 'uint32_t' ;;
        uint64) printf 'uint64_t' ;;
        int32)  printf 'int32_t' ;;
        int64)  printf 'int64_t' ;;
        string) printf 'const char*' ;;
        blob)   printf 'const void*' ;;
        void)   printf 'void' ;;
        *)      printf '%s' "$1" ;; # passthrough
    esac
}
push_for() {
    case "$1" in
        uint32) printf 'push_uint32' ;;
        uint64) printf 'push_uint64' ;;
        int32)  printf 'push_int32' ;;
        int64)  printf 'push_int64' ;;
        string) printf 'push_string' ;;
        blob)   printf 'push_blob' ;;
        *)      printf '' ;;
    esac
}
get_for() {
    case "$1" in
        uint32) printf 'get_uint32' ;;
        uint64) printf 'get_uint64' ;;
        int32)  printf 'get_int32' ;;
        int64)  printf 'get_int64' ;;
        string) printf 'get_string' ;;
        blob)   printf 'get_blob' ;;
        *)      printf '' ;;
    esac
}

# Get the argument names from an argument spec
get_call_args() {
    local argspec="$1"
    if [ -z "$argspec" ]; then return; fi

    IFS=',' read -ra parts <<< "$argspec"
    local names=()
    for ap in "${parts[@]}"; do
        names+=("$(trim "$ap" | awk '{print $2}')")
    done
    (IFS=','; printf '%s' "${names[*]}")
}

# Call a method with an argument spec
make_call() {
    local method="$1"
    local argspec="$2"
    echo -n "${method}("
    get_call_args "$argspec"
    echo -n ")"
}

find "$ROOT" -name "*.rpc" | while read -r file; do
    rpcgen "Processing $file"

    # Read metadata: package, version, path, service
    package=$(awk '/^[[:space:]]*package[[:space:]]+/{gsub(/;/,""); print $2; exit}' "$file" || true)
    version=$(awk '/^[[:space:]]*version[[:space:]]+/{gsub(/;/,""); print $2; exit}' "$file" || true)
    path_raw=$(awk '/^[[:space:]]*path[[:space:]]+/{gsub(/;/,""); print $2; exit}' "$file" || true)
    service=$(awk '/^[[:space:]]*service[[:space:]]+/{gsub(/\{/,""); print $2; exit}' "$file" || true)

    folder=$(dirname "$file")
    clean_path=$(printf '%s' "$path_raw" | sed 's|^/||; s|/$||' | tr -d '\r\n')

    # Create output directories
    incdir="$folder/include/$clean_path"
    srcdir="$folder/src/$clean_path"
    inc_include_prefix="${clean_path}/"
    mkdir -p "$incdir" "$srcdir"

    # Create output files
    srv_h="$incdir/${service}_server.h"
    srv_cpp="$srcdir/${service}_server.cpp"
    cli_h="$incdir/${service}_client.h"
    cli_cpp="$srcdir/${service}_client.cpp"
    types_h="$incdir/${service}_types.h"
    touch "$srv_h" "$srv_cpp" "$cli_h" "$cli_cpp" "$types_h"

    rpcgen "  service: $service"
    rpcgen "  path:    $path_raw"

    # Get all lines defining a rpc method
    mapfile -t raw_rpcs < <(sed -n 's/^[[:space:]]*rpc[[:space:]]\+/\0/p' "$file" | sed -n 's/^[[:space:]]*rpc[[:space:]]\+//p' || true)
    if [ "${#raw_rpcs[@]}" -eq 0 ]; then
        mapfile -t raw_rpcs < <(awk '/^[[:space:]]*rpc[[:space:]]+/{print substr($0, index($0,$2))}' "$file" 2>/dev/null || true)
    fi

    # Arrays to hold parsed data
    declare -a METHODS=()
    declare -a RETTYPES=()
    declare -a ARGLISTS=()
    declare -a FLAGS=()

    for rline in "${raw_rpcs[@]}"; do

        # Clean line (remove semicolon, trim)
        line=$(printf '%s' "$rline" | sed 's/;[[:space:]]*$//' )
        line=$(trim "$line")

        # Extract flags if present (stuff between [])
        flags=""
        if [[ "$line" =~ \[(.*)\] ]]; then
            flags="${BASH_REMATCH[1]}"
            line="${line/\[${BASH_REMATCH[1]}\]/}"
            line=$(trim "$line")
        fi

        # Extract return type, method name, argument list
        if [[ "$line" =~ ^\([[:space:]]*([^\)]*)[[:space:]]*\)[[:space:]]+([A-Za-z_][A-Za-z0-9_]*)[[:space:]]*\((.*)\)[[:space:]]*$ ]]; then
            # Multireturn
            rettype_raw="(${BASH_REMATCH[1]})"
        elif [[ "$line" =~ ^([A-Za-z0-9_]+)[[:space:]]+([A-Za-z_][A-Za-z0-9_]*)[[:space:]]*\((.*)\)[[:space:]]*$ ]]; then
            rettype_raw="${BASH_REMATCH[1]}"
        else
            rpcgen "  [WARN] Could not parse RPC line: '$rline' (skipping)"
            continue
        fi
        method_raw="${BASH_REMATCH[2]}"
        args_raw="${BASH_REMATCH[3]}"

        # Trim the args
        args_norm=""
        if [ -z "$(trim "$args_raw")" ]; then
            args_norm=""
        else
            # Split by comma, trim each part
            IFS=',' read -ra aparts <<< "$args_raw"
            out_parts=()
            for ap in "${aparts[@]}"; do
                apn=$(trim "$ap")

                # Should only be one space between type and name
                apn=$(echo "$apn" | sed 's/[[:space:]]\+/ /g')
                out_parts+=("$apn")
            done
            args_norm=$(IFS=','; printf '%s' "${out_parts[*]}")
        fi

        # Store parsed data
        METHODS+=("$method_raw")
        RETTYPES+=("$rettype_raw")
        ARGLISTS+=("$args_norm")
        FLAGS+=("$flags")
    done

    # Generate types for multireturns
    cat > "$types_h" <<EOF
#pragma once
#include <cstdint>
#include <ipc/rpc.h>

EOF

    # For each function with a multireturn, generate a struct
    for idx in "${!METHODS[@]}"; do

        # Get the return type and function name
        rett="${RETTYPES[$idx]}"
        method="${METHODS[$idx]}"

        # Check if it's a multireturn (contains '()')
        if [[ "$rett" =~ ^\((.*)\)$ ]]; then

            # Create the struct for the multireturn
            struct_name="${method}_result_t"
            echo "struct ${struct_name} {" >> "$types_h"

            # Split the types by comma
            inner="${BASH_REMATCH[1]}"
            IFS=',' read -ra rets <<< "$inner"

            # Add each return type as a variable
            i=0
            for rt in "${rets[@]}"; do
                rtrim=$(trim "$rt")
                cpp_type=$(idl_to_cpp "$rtrim")
                echo "    ${cpp_type} v${i};" >> "$types_h"
                i=$((i+1))
            done

            # Close the struct
            echo "};" >> "$types_h"
            echo "" >> "$types_h"
        fi
    done

    # Generate server header
    cat > "$srv_h" <<EOF
#pragma once
#include <ipc/rpc.h>
#include <${inc_include_prefix}${service}_types.h>

void register_${service}_functions();
void run_${service}();

EOF

    # Generate client header
    cat > "$cli_h" <<EOF
#pragma once
#include <ipc/rpc.h>
#include <${inc_include_prefix}${service}_types.h>

using namespace MaxOS::KPI::ipc;

void wait_for_${service}_server();

EOF

    # Forward declare the functions for the server, generate definitions for the client
    for idx in "${!METHODS[@]}"; do

        # Get the function name, return type, and argument list
        rett="${RETTYPES[$idx]}"
        method="${METHODS[$idx]}"
        args="${ARGLISTS[$idx]}"

        # If multireturn, use generated struct
        if [[ "$rett" =~ ^\((.*)\)$ ]]; then
            cpp_ret="${method}_result_t"
        else
            cpp_ret=$(idl_to_cpp "$rett")
        fi

        # Generate the list of arguments
        arg_decl=""
        if [ -n "$args" ]; then

            # Split by comma
            IFS=',' read -ra argparts <<< "$args"
            tmp=()

            for ap in "${argparts[@]}"; do
                apn=$(trim "$ap")

                # Split into type and name
                atype=$(echo "$apn" | awk '{print $1}')
                aname=$(echo "$apn" | awk '{print $2}')

                # Map IDL type to C++ type and append
                cpp_type=$(idl_to_cpp "$atype")
                tmp+=("${cpp_type} ${aname}")
            done
            arg_decl=$(IFS=', '; printf '%s' "${tmp[*]}")
        fi

        # Output to headers
        echo "${cpp_ret} ${method}(${arg_decl});" >> "$srv_h"
        echo "${cpp_ret} ${method}(${arg_decl});" >> "$cli_h"
    done

    # Generate server source
    cat > "$srv_cpp" <<EOF
#include <${inc_include_prefix}${service}_server.h>

using namespace MaxOS::KPI::ipc;

EOF

    # Generate wrappers for each methods
    for idx in "${!METHODS[@]}"; do

        # Get the function name, return type, and argument list
        method="${METHODS[$idx]}"
        rett="${RETTYPES[$idx]}"
        args="${ARGLISTS[$idx]}"

        # Declare the wrapper function
        wrapper="${method}_wrapper"
        cat >> "$srv_cpp" <<EOF
static void ${wrapper}(ArgList* _args, ArgList* _returns) {
EOF

        # Parse and retrieve arguments
        if [ -n "$args" ]; then

            # Split by comma
            IFS=',' read -ra argparts <<< "$args"
            ai=0
            for ap in "${argparts[@]}"; do
                apn=$(trim "$ap")

                # Split into type and name
                atype=$(echo "$apn" | awk '{print $1}')
                aname=$(echo "$apn" | awk '{print $2}')

                # Generate retrieval code
                getfn=$(get_for "$atype")
                cpp_type=$(idl_to_cpp "$atype")

                # Get argument from _args
                if [ -z "$getfn" ]; then
                    echo "    // Unknown arg type '$atype' for ${aname}; retrieving raw blob" >> "$srv_cpp"
                    echo "    auto ${aname} = _args->get_blob(${ai});" >> "$srv_cpp"
                else
                    echo "    ${cpp_type} ${aname} = _args->${getfn}(${ai});" >> "$srv_cpp"
                fi
                ai=$((ai+1))
            done
        fi

        # Call the internal function and handle return values
        {
        if [[ "$rett" =~ ^\((.*)\)$ ]]; then
            # - Multireturn
            inner="${BASH_REMATCH[1]}"

            # Call the function and pass args
            echo -n "    ${method}_result_t _ret = "
            make_call "$method" "$args"
            echo ";" >> "$srv_cpp"

            # Split the return by comma
            IFS=',' read -ra rets <<< "$inner"
            i=0
            for rr in "${rets[@]}"; do

                # Trim and get push function
                rtrim=$(trim "$rr")
                pushfn=$(push_for "$rtrim")

                # Add to the returns
                if [ -z "$pushfn" ]; then
                    echo "    // TODO: unsupported return type '$rtrim' for ${method}" >> "$srv_cpp"
                else
                    echo "    _returns->${pushfn}(_ret.v${i});" >> "$srv_cpp"
                fi
                i=$((i+1))
            done

        else
            if [ "$rett" = "void" ]; then
                # - No return value

                # Call the function and pass args
                echo -n " "
                make_call "$method" "$args"
                echo ";" >> "$srv_cpp"

            else
                # - Single return value
                cpp_ret=$(idl_to_cpp "$rett")

                # Call the function and pass args
                echo -n "    ${cpp_ret} _ret = "
                make_call "$method" "$args"
                echo ";" >> "$srv_cpp"

                # Add to the returns
                pushfn=$(push_for "$rett")
                if [ -n "$pushfn" ]; then
                    echo "    _returns->${pushfn}(_ret);" >> "$srv_cpp"
                else
                    echo "    // TODO: unsupported return type '$rett' for ${method}" >> "$srv_cpp"
                fi
            fi
        fi

        # Close the wrapper function
        } >> "$srv_cpp"
        cat >> "$srv_cpp" <<EOF
}

EOF
    done

    # Create the registerer for each function
    echo "void register_${service}_functions() {" >> "$srv_cpp"
    for idx in "${!METHODS[@]}"; do
        method="${METHODS[$idx]}"
        wrapper="${method}_wrapper"
        echo "    register_function(\"${method}\", ${wrapper});" >> "$srv_cpp"
    done
    cat >> "$srv_cpp" <<EOF
}

void run_${service}() {
    register_${service}_functions();
    rpc_server_loop("${service}");
}
EOF

    # Gererate client source
    cat > "$cli_cpp" <<EOF
#include <${inc_include_prefix}${service}_client.h>

using namespace MaxOS::KPI::ipc;

void wait_for_${service}_server() {
    rpc_wait_for_server("${service}");
}

EOF

    for idx in "${!METHODS[@]}"; do

        # Get the function name, return type, argument list, and flags
        method="${METHODS[$idx]}"
        rett="${RETTYPES[$idx]}"
        args="${ARGLISTS[$idx]}"
        flags="${FLAGS[$idx]}"

        # If it is a multireturn, use generated struct
        if [[ "$rett" =~ ^\((.*)\)$ ]]; then
            cpp_ret="${method}_result_t"
        else
            cpp_ret=$(idl_to_cpp "$rett")
        fi

        # Generate the list of argument types and names
        arg_decl=""
        arg_names=""
        if [ -n "$args" ]; then

            # Split by comma
            IFS=',' read -ra argparts <<< "$args"
            decls=()
            names=()
            for ap in "${argparts[@]}"; do
                apn=$(trim "$ap")

                # Get type and name
                atype=$(echo "$apn" | awk '{print $1}')
                aname=$(echo "$apn" | awk '{print $2}')

                # Conver to c++ and append
                cpp_type=$(idl_to_cpp "$atype")
                decls+=("${cpp_type} ${aname}")
                names+=("${aname}")
            done
            arg_decl=$(IFS=', '; printf '%s' "${decls[*]}")
            arg_names=$(IFS=', '; printf '%s' "${names[*]}")
        fi

        # Create the function definition
        echo "${cpp_ret} ${method}(${arg_decl}) {" >> "$cli_cpp"
        echo "    ArgList _args;" >> "$cli_cpp"
        echo "    ArgList _returns;" >> "$cli_cpp"

        # Add each argument to the ArgList
        if [ -n "$args" ]; then

            # Split by comma
            IFS=',' read -ra argparts <<< "$args"
            ai=0
            for ap in "${argparts[@]}"; do
                apn=$(trim "$ap")

                # Get type and name
                atype=$(echo "$apn" | awk '{print $1}')
                aname=$(echo "$apn" | awk '{print $2}')

                # Add to ArgList
                pushfn=$(push_for "$atype")
                if [ -n "$pushfn" ]; then
                    echo "    _args.${pushfn}(${aname});" >> "$cli_cpp"
                else
                    echo "    // TODO: push unsupported arg type '$atype' for ${aname}" >> "$cli_cpp"
                fi
                ai=$((ai+1))
            done
        fi

        # Generate flag mask
        flagmask="0"
        if [ -n "$flags" ]; then

            # Split by comma
            IFS=',' read -ra fparts <<< "$flags"
            fm_lines=()
            for fp in "${fparts[@]}"; do

                # Map flag to enum
                ftrim=$(trim "$fp")
                case "$ftrim" in
                    urgent) fm_lines+=("(size_t)RPCMEssageFlags::URGENT") ;;
                    oneway) fm_lines+=("(size_t)RPCMEssageFlags::ONE_WAY") ;;
                    descriptor_as_data) fm_lines+=("(size_t)RPCMEssageFlags::DESCRIPTOR_AS_DATA") ;;
                    *) fm_lines+=("0") ;;
                esac
            done
            flagmask=$(IFS=' | '; printf '%s' "${fm_lines[*]}")
            if [ -z "$flagmask" ]; then flagmask="0"; fi
        fi

        # Call the function on the server
        echo "    rpc_call(\"${service}\", \"${method}\", &_args, &_returns, ${flagmask});" >> "$cli_cpp"

        # Extract and return the return values
        if [[ "$rett" =~ ^\((.*)\)$ ]]; then
            # - Multireturn
            inner="${BASH_REMATCH[1]}"

            # Split by comma
            IFS=',' read -ra rets <<< "$inner"
            init_items=()
            i=0
            for rr in "${rets[@]}"; do

                # Get trimmed type and get function
                rtrim=$(trim "$rr")
                getfn=$(get_for "$rtrim")
                if [ -n "$getfn" ]; then
                    init_items+=("_returns.${getfn}(${i})")
                else
                    init_items+=("/* unsupported */")
                fi
                i=$((i+1))
            done

            # Generate return struct
            echo "    return ${cpp_ret}{ $(IFS=', '; printf '%s' "${init_items[*]}") };" >> "$cli_cpp"
        else
            if [ "$rett" = "void" ]; then
                # - No return value
                echo "    return;" >> "$cli_cpp"
            else
                # - Single return value

                # Get the get function
                getfn=$(get_for "$rett")
                if [ -n "$getfn" ]; then
                    echo "    return _returns.${getfn}(0);" >> "$cli_cpp"
                else
                    echo "    // TODO: unsupported return type '$rett' for ${method}" >> "$cli_cpp"
                    echo "    return ( ${cpp_ret} )0;" >> "$cli_cpp"
                fi
            fi
        fi

        echo "}" >> "$cli_cpp"
        echo "" >> "$cli_cpp"
    done

    rpcgen "Done."
done
