# CMakeList.txt: проект CMake для lykovAddInNative; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.20)
cmake_policy(SET CMP0091 NEW)

# Общие параметры
project (lykovAddInNative LANGUAGES CXX DESCRIPTION "Lykov (AddInNative)")
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(ENABLE_BASE_FUNCTION true)
set(ENABLE_HTTP_CLIENT true)
set(ENABLE_POSTGRE_SQL true)
set(ENABLE_MY_SQL true)
set(output_directory ${CMAKE_SOURCE_DIR}/bin)

# Базовые параметры
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CRT_LINKAGE static)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS ON)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
set(CMAKE_EXE_LINKER_FLAGS "-static")

if (VCPKG_TARGET_TRIPLET STREQUAL x86-windows-static)
    set(ENABLE_MY_SQL false) # Всегда false (не работает на x86)
endif()

if (WIN32)  
    string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
    string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})

    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    set(CMAKE_THREAD_LIBS_INIT "-lpthread")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--no-as-needed -MT -std=c++17 -pedantic -static-libgcc -static-libstdc++ -lpq -fPIC -lpthread ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--no-as-needed -MT -std=c++17 -pedantic -static-libgcc -static-libstdc++ -lpq -fPIC -lpthread ")
    
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -O3 -DNDEBUG -s ")    
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O3 -DNDEBUG -s ")
    endif()
endif()

if(WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
else()
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif()

# Подготовка параметров для компиляции
if(WIN32)
    CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/include/forConfigure/LykovAddInNative.rc.in" "${CMAKE_CURRENT_SOURCE_DIR}/lykovAddInNative.rc")
    CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/include/forConfigure/LykovAddInNative.def.in" "${CMAKE_CURRENT_SOURCE_DIR}/lykovAddInNative.def")
    ENABLE_LANGUAGE(RC)
endif()

set(AddInNative_IncludeDir "${CMAKE_SOURCE_DIR}/include")

set(AddInNative_SRC # Список базовых файлов
        "include/baseClass/stdafx.hpp"
        "src/baseClass/dllmain.cpp"
        "include/from1C/AddInDefBase.h"
        "include/from1C/com.h"
        "include/from1C/ComponentBase.h"
        "include/from1C/IMemoryManager.h"
        "include/from1C/types.h"

        "include/baseClass/globalFunctions.hpp"
        "src/baseClass/globalFunctions.cpp"

        "include/baseClass/lykovExceptions.hpp"
        "src/baseClass/lykovExceptions.cpp"

        "include/baseClass/baseExtensionClass.hpp"
        "src/baseClass/baseExtensionClass.cpp"

        "include/baseClass/lykovAddInNative.hpp"
        "src/baseClass/lykovAddInNative.cpp"
)

if(WIN32)
    list(APPEND AddInNative_SRC lykovAddInNative.rc lykovAddInNative.def)
    add_definitions(-DUNICODE -DWIN32 -D_USRDLL -D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE -DADDINCPP_EXPORTS)
    if(TARGET_PLATFORM_32)
        add_definitions(-D_USE_32BIT_TIME_T)
    endif()
endif()

# Подключаем нужные пакеты
set(LYKOV_BOOST_LIBS
    filesystem
    locale
    system
    thread
)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
set(Boost_NO_WARN_NEW_VERSIONS ON)
set(Boost_USE_MULTITHREADED ON)
set(OPENSSL_STATIC ON)

find_package(Boost COMPONENTS ${LYKOV_BOOST_LIBS} REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
find_package(stduuid CONFIG REQUIRED)
find_package(utf8cpp CONFIG REQUIRED)

find_package(OpenSSL REQUIRED)

link_directories(${Boost_LIBRARY_DIRS})

# Определение библиотек
set(LIST_EXTENSION_PROJECTS "")

if (ENABLE_BASE_FUNCTION)
    if(WIN32)
        find_library(LIB_ZLIB_NG zlibstatic-ng PATHS ${Boost_LIBRARY_DIRS})
    else()
        find_library(LIB_ZLIB_NG libz-ng.a PATHS ${Boost_LIBRARY_DIRS})
    endif()

    set(Extension_Base_Function_Name ${PROJECT_NAME}_ExtFunctions)

    add_library(${Extension_Base_Function_Name} SHARED ${AddInNative_SRC} "include/extensions/extensionBaseFunction.hpp" "src/extensions/extensionBaseFunction.cpp")
    
    target_compile_definitions(${Extension_Base_Function_Name} PRIVATE ENABLE_BASE_FUNCTION=true)
    target_link_libraries(${Extension_Base_Function_Name} PRIVATE ${LIB_ZLIB_NG})

    list(APPEND LIST_EXTENSION_PROJECTS ${Extension_Base_Function_Name})
endif()
if (ENABLE_HTTP_CLIENT)
    find_package(CURL CONFIG REQUIRED)
    set(HTTP_Client_Name ${PROJECT_NAME}_HTTPClient)

    add_library(${HTTP_Client_Name} SHARED ${AddInNative_SRC} "include/extensions/HTTPClient.hpp" "src/extensions/HTTPClient.cpp")
    
    target_compile_definitions(${HTTP_Client_Name} PRIVATE ENABLE_HTTP_CLIENT=true)
    target_link_libraries(${HTTP_Client_Name} PRIVATE CURL::libcurl)

    list(APPEND LIST_EXTENSION_PROJECTS ${HTTP_Client_Name})
endif()
if (ENABLE_POSTGRE_SQL)
    find_package(CURL CONFIG REQUIRED)
    find_package(libpqxx CONFIG REQUIRED)

    set(PostgreSQL_Name ${PROJECT_NAME}_PostgreSQL)

    add_library(${PostgreSQL_Name} SHARED ${AddInNative_SRC} "include/extensions/postgreSQL.hpp" "src/extensions/postgreSQL.cpp")
    
    target_link_libraries(${PostgreSQL_Name} PRIVATE CURL::libcurl libpqxx::pqxx)
    target_compile_definitions(${PostgreSQL_Name} PRIVATE ENABLE_POSTGRE_SQL=true)
    
    list(APPEND LIST_EXTENSION_PROJECTS ${PostgreSQL_Name})
endif()
if (ENABLE_MY_SQL)
    find_package(libmysql REQUIRED)
    set(MySQL_Name ${PROJECT_NAME}_MySQL)

    add_library(${MySQL_Name} SHARED ${AddInNative_SRC} "include/extensions/mySQLConn.hpp" "src/extensions/mySQLConn.cpp")
    
    target_compile_definitions(${MySQL_Name} PRIVATE ENABLE_MY_SQL=true)
    target_link_libraries(${MySQL_Name} PRIVATE ${MYSQL_LIBRARIES})

    list(APPEND LIST_EXTENSION_PROJECTS ${MySQL_Name})
endif()

# Общие параметры библиотек
foreach(current_project_name ${LIST_EXTENSION_PROJECTS})
    target_compile_features(${current_project_name} PRIVATE cxx_std_17)
    target_include_directories(${current_project_name} PRIVATE ${AddInNative_IncludeDir} ${Boost_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})

    foreach(boost_lib ${LYKOV_BOOST_LIBS})
        target_link_libraries(${current_project_name} PRIVATE "Boost::${boost_lib}")
    endforeach()

    target_link_libraries(${current_project_name} PRIVATE OpenSSL::Crypto OpenSSL::SSL rapidjson stduuid)

    # Разные версии
    if (WIN32)
        target_link_libraries(${current_project_name} PRIVATE utf8cpp)
    else()
        target_link_libraries(${current_project_name} PRIVATE utf8::cpp)
    endif()

    if (NOT WIN32)
        set_target_properties(${current_project_name} PROPERTIES LINK_FLAGS "-fPIC -std=c++17 -Wall -Wunknown-pragmas -pedantic -O3 -ldl -lc -lm -Wl,--no-as-needed -lpthread -Xlinker --version-script ${CMAKE_CURRENT_SOURCE_DIR}/version.script ")
    endif()
    target_precompile_headers(${current_project_name} PUBLIC "include/baseClass/stdafx.hpp")

    set_target_properties(${current_project_name} PROPERTIES
        OUTPUT_NAME ${current_project_name}-${TypeCompile}
        POSITION_INDEPENDENT_CODE ON)

    add_custom_command(TARGET ${current_project_name} POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${current_project_name}> ${output_directory}/${current_project_name}/$<TARGET_FILE_NAME:${current_project_name}>
        MAIN_DEPENDENCY ${current_project_name}
    )
endforeach()

#------------------- Окончание (вывод параметров) -------------------
message(-------------------------------LYKOV_Параметры-------------------------------)
message("    PROJECT_NAME: " ${PROJECT_NAME})
message("    CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})
message("    CMAKE_TOOLCHAIN_FILE: " ${CMAKE_TOOLCHAIN_FILE})
message("    VCPKG_TARGET_TRIPLET: " ${VCPKG_TARGET_TRIPLET})
message("    TypeCompile: " ${TypeCompile})
message("    AddInNative_SRC: " ${AddInNative_SRC})
message("    Boost_INCLUDE_DIR: " ${Boost_INCLUDE_DIR})
message("    Boost_LIBRARY_DIRS: " ${Boost_LIBRARY_DIRS})
message("    AddInNative_IncludeDir: " ${AddInNative_IncludeDir})
message("    LIB_ZLIB_NG: " ${LIB_ZLIB_NG})
message("    CMAKE_TOOLCHAIN_FILE: " ${CMAKE_TOOLCHAIN_FILE})
message("    VCPKG_TARGET_TRIPLET: " ${VCPKG_TARGET_TRIPLET})
message("    CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
message("    ENABLE_BASE_FUNCTION: " ${ENABLE_BASE_FUNCTION})
message("    ENABLE_HTTP_CLIENT: " ${ENABLE_HTTP_CLIENT})
message("    ENABLE_POSTGRE_SQL: " ${ENABLE_POSTGRE_SQL})
message("    ENABLE_MY_SQL: " ${ENABLE_MY_SQL})
message("    output_directory: " ${output_directory})
message("    LIST_EXTENSION_PROJECTS: " ${LIST_EXTENSION_PROJECTS})
message(-----------------------------------------------------------------------------)
