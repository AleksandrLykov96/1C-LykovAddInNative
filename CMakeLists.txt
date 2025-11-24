# CMakeList.txt: проект CMake для lykovAddInNative; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.27)
cmake_policy(SET CMP0091 NEW)

project (lykovAddInNative LANGUAGES CXX DESCRIPTION "Lykov (AddInNative)")
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# Какие компоненты компилировать
set(ENABLE_BASE_FUNCTION false)
set(ENABLE_HTTP_CLIENT false)
set(ENABLE_POSTGRE_SQL false)
set(ENABLE_PDF_EDITOR true)

# Базовые параметры компиляции
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CRT_LINKAGE static)
set(CMAKE_BUILD_PARALLEL_LEVEL 1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXE_LINKER_FLAGS "-static")
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
set(BUILD_SHARED_LIBS OFF)
set(output_directory ${CMAKE_SOURCE_DIR}/bin)


if (WIN32)
    string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS         ${CMAKE_CXX_FLAGS})
    string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_DEBUG   ${CMAKE_CXX_FLAGS_DEBUG})
    string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})

    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif(UNIX)
    set(CMAKE_THREAD_LIBS_INIT "-lpthread")
    
    set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -g -MT -std=c++20 -Wall -pedantic -fPIC ")
    set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -g -MT -std=c++20 -Wall -pedantic -fPIC ")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -MT -std=c++20 -Wall -pedantic -fPIC ")
    set(VCPKG_CXX_FLAGS         "${VCPKG_CXX_FLAGS} -g -MT -std=c++20 -Wall -pedantic -fPIC -static-libgcc -static-libstdc++ -lpthread ")
    set(VCPKG_CXX_FLAGS_DEBUG   "${VCPKG_CXX_FLAGS_DEBUG} -g -MT -std=c++20 -Wall -pedantic -fPIC -static-libgcc -static-libstdc++ -lpthread ")
    set(VCPKG_CXX_FLAGS_RELEASE "${VCPKG_CXX_FLAGS_RELEASE} -g -MT -std=c++20 -Wall -pedantic -fPIC -static-libgcc -static-libstdc++ -lpthread ")
    set(VCPKG_C_FLAGS           "${VCPKG_C_FLAGS} -g -MT -std=c++20 -Wall -pedantic -fPIC -static-libgcc -static-libstdc++ -lpthread ")

    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -MT -O3 ")
        set(VCPKG_CXX_FLAGS         "${VCPKG_CXX_FLAGS} -MT -O3 ")
        set(VCPKG_CXX_FLAGS_DEBUG   "${VCPKG_CXX_FLAGS_DEBUG} -MT -O3 ")
        set(VCPKG_CXX_FLAGS_RELEASE "${VCPKG_CXX_FLAGS_RELEASE} -MT -O3 ")
        set(VCPKG_C_FLAGS           "${VCPKG_C_FLAGS} -MT -O3 ")
    endif()
    if (TARGET_ARCH STREQUAL "x86")
        set(CMAKE_CXX_FLAGS         "-m32 ${CMAKE_CXX_FLAGS}")
        set(VCPKG_CXX_FLAGS         "-m32 ${VCPKG_CXX_FLAGS}")
        set(VCPKG_CXX_FLAGS_DEBUG   "-m32 ${VCPKG_CXX_FLAGS_DEBUG}")
        set(VCPKG_CXX_FLAGS_RELEASE "-m32 ${VCPKG_CXX_FLAGS_RELEASE}")
        set(VCPKG_C_FLAGS           "-m32 ${VCPKG_C_FLAGS}")
    endif ()
endif()

if(WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
else()
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif()

if(WIN32)
    CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/include/forConfigure/LykovAddInNative.rc.in" "${CMAKE_CURRENT_SOURCE_DIR}/lykovAddInNative.rc")
    CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/include/forConfigure/LykovAddInNative.def.in" "${CMAKE_CURRENT_SOURCE_DIR}/lykovAddInNative.def")
    
    ENABLE_LANGUAGE(RC)
endif()

set(AddInNative_IncludeDir "${CMAKE_SOURCE_DIR}/include")

set(AddInNative_SRC # Список базовых файлов
        "include/baseClass/baseClassForExtension.hpp"        
        "include/baseClass/stdafx.hpp"
        "include/baseClass/encrypt.hpp"
        "include/baseClass/convertingString.hpp"
        "include/baseClass/from1C/AddInDefBase.h"
        "include/baseClass/from1C/com.h"
        "include/baseClass/from1C/ComponentBase.h"
        "include/baseClass/from1C/IMemoryManager.h"
        "include/baseClass/from1C/types.h"
        
        "src/baseClass/dllmain.cpp"
        "src/baseClass/dllExport.cpp"
        "src/baseClass/baseClassForExtension.cpp"
        "src/baseClass/encrypt.cpp"
        "src/baseClass/convertingString.cpp"
)

# Определения препроцессора
if(WIN32)
    list(APPEND AddInNative_SRC lykovAddInNative.rc lykovAddInNative.def)
    add_definitions(-DWIN32 -D_USRDLL -DADDINCPP_EXPORTS -D_WINDOWS)
    if(TARGET_PLATFORM_32)
        add_definitions(-D_USE_32BIT_TIME_T)
    endif()
endif()

add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DNDEBUG)
endif()

# BOOST
add_definitions(-DBOOST_FILESYSTEM_NO_DEPRECATED)
add_definitions(-DBOOST_BEAST_USE_STD_STRING_VIEW)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DBOOST_DISABLE_ASSERTS)
    add_definitions(-DBOOST_DISABLE_CURRENT_LOCATION)
endif()

# RapidJSON
add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)
add_definitions(-DRAPIDJSON_VALUE_DEFAULT_ARRAY_CAPACITY=32)
add_definitions(-DRAPIDJSON_VALUE_DEFAULT_OBJECT_CAPACITY=32)
add_definitions(-DRAPIDJSON_USE_MEMBERSMAP=1)

if (WIN32)
    add_definitions(-DRAPIDJSON_SIMD)
    add_definitions(-DRAPIDJSON_SSE42)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
endif()

# Остальные
add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)
add_definitions(-DUUID_SYSTEM_GENERATOR)
add_definitions(-DPLOG_ENABLE_WCHAR_INPUT)

if(UNIX)
    add_definitions(-DLIBXSLT_WITH_THREADS=ON)
    add_definitions(-DLIBXSLT_WITH_XSLT_DEBUG=OFF)
endif()

# Подключаем нужные пакеты
set(LYKOV_BOOST_LIBS
    exception
    date_time
    serialization
)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
set(Boost_NO_WARN_NEW_VERSIONS ON)
set(Boost_USE_MULTITHREADED ON)
set(OPENSSL_STATIC ON)

find_package(Threads REQUIRED)
find_package(uni-algo CONFIG REQUIRED)
find_package(Boost COMPONENTS ${LYKOV_BOOST_LIBS} REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(plog REQUIRED)
find_package(stduuid REQUIRED)
find_package(ZLIB REQUIRED)

find_path(SKCRYPTER_INCLUDE_DIRS "skCrypter.h")
link_directories(${Boost_LIBRARY_DIRS})

# Определение библиотек
set(LIST_EXTENSION_PROJECTS "")

if (ENABLE_BASE_FUNCTION)
    set(Extension_Base_Function_Name ${PROJECT_NAME}_BaseFunctions)

    add_library(${Extension_Base_Function_Name} SHARED
            ${AddInNative_SRC}
            "include/extensions/extensionBaseFunctions.hpp"
            "src/extensions/extensionBaseFunctions.cpp"
    )
    
    target_compile_definitions(${Extension_Base_Function_Name} PRIVATE ENABLE_BASE_FUNCTION=true)
    target_link_libraries(${Extension_Base_Function_Name} PRIVATE ZLIB::ZLIB)

    list(APPEND LIST_EXTENSION_PROJECTS ${Extension_Base_Function_Name})
endif()
if (ENABLE_HTTP_CLIENT)
    find_package(CURL CONFIG REQUIRED)
    find_package(magic_enum CONFIG REQUIRED)

    set(HTTP_Client_Name ${PROJECT_NAME}_HTTPClient)

    add_library(${HTTP_Client_Name} SHARED ${AddInNative_SRC} "include/extensions/HTTPClient.hpp" "src/extensions/HTTPClient.cpp")
    
    target_compile_definitions(${HTTP_Client_Name} PRIVATE ENABLE_HTTP_CLIENT=true)
    target_link_libraries(${HTTP_Client_Name} PRIVATE CURL::libcurl magic_enum::magic_enum)

    list(APPEND LIST_EXTENSION_PROJECTS ${HTTP_Client_Name})
endif()
if (ENABLE_POSTGRE_SQL)
    find_package(CURL CONFIG REQUIRED)
    find_package(libpqxx CONFIG REQUIRED)

    set(PostgreSQL_Name ${PROJECT_NAME}_postgreSQL)

    add_library(${PostgreSQL_Name} SHARED ${AddInNative_SRC} "include/extensions/postgreSQL.hpp" "src/extensions/postgreSQL.cpp")
    
    target_compile_definitions(${PostgreSQL_Name} PRIVATE ENABLE_POSTGRE_SQL=true)
    target_link_libraries(${PostgreSQL_Name} PRIVATE CURL::libcurl libpqxx::pqxx)
    
    list(APPEND LIST_EXTENSION_PROJECTS ${PostgreSQL_Name})
endif()
if (ENABLE_PDF_EDITOR)
    find_package(PoDoFo CONFIG REQUIRED)

    set(PdfEditor_Name ${PROJECT_NAME}_PdfEditor)

    add_library(${PdfEditor_Name} SHARED ${AddInNative_SRC} "include/extensions/pdfEditor.hpp" "src/extensions/pdfEditor.cpp")
    
    target_compile_definitions(${PdfEditor_Name} PRIVATE ENABLE_PDF_EDITOR=true)
    target_link_libraries(${PdfEditor_Name} PRIVATE podofo::podofo)
    
    list(APPEND LIST_EXTENSION_PROJECTS ${PdfEditor_Name})
endif()

# Общие параметры библиотек
set(BUILD_SHARED_LIBS ON)

foreach(current_project_name ${LIST_EXTENSION_PROJECTS})
    target_compile_features(${current_project_name} PRIVATE cxx_std_20)
    target_include_directories(${current_project_name} PRIVATE ${AddInNative_IncludeDir} ${Boost_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${SKCRYPTER_INCLUDE_DIRS})
    
    if (WIN32)
        target_compile_options(${current_project_name} PRIVATE /utf-8)
    endif()

    foreach(boost_lib ${LYKOV_BOOST_LIBS})
        target_link_libraries(${current_project_name} PRIVATE "Boost::${boost_lib}")
    endforeach()
    
    target_link_libraries(${current_project_name} PRIVATE Threads::Threads uni-algo::uni-algo OpenSSL::Crypto OpenSSL::SSL stduuid rapidjson plog::plog)
    
    if (UNIX)
        set(properties_for_nix
            "-Wunknown-pragmas -std=c++20 -pedantic -static-libgcc -static-libstdc++ -lm -lc -ldl -lgcc -Wl,--no-as-needed -lpthread -Xlinker --version-script ${CMAKE_CURRENT_SOURCE_DIR}/include/forConfigure/version.script ")

        if (CMAKE_BUILD_TYPE STREQUAL "Release")
            set(properties_for_nix "${properties_for_nix} -O3 -s -fPIC -Wall ")
        endif()

        set_target_properties(${current_project_name} PROPERTIES LINK_FLAGS ${properties_for_nix})
    endif()

    set_target_properties(${current_project_name} PROPERTIES
        OUTPUT_NAME ${current_project_name}-${TypeCompile}
        POSITION_INDEPENDENT_CODE ON)

    add_custom_command(TARGET ${current_project_name} POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${current_project_name}> ${output_directory}/${current_project_name}/$<TARGET_FILE_NAME:${current_project_name}>
        MAIN_DEPENDENCY ${current_project_name}
    )
endforeach()
