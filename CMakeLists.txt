cmake_minimum_required(VERSION 3.10)

project("PlayerVoiceEvents" VERSION 0.8 LANGUAGES CXX)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(headers
        src/Main.h
        src/PapyrusFunctions.h
        src/EventListener.h
        src/CustomEventListener.h
        src/Utils.h
        src/SoundEvent.h
        src/Logger.h
        src/LoopManager.h
        src/ConditionManager.h
        src/ConfigParser.h
        src/Exports.h
)

set(sources
        src/Plugin.cpp
        src/Main.cpp
        src/PapyrusFunctions.cpp
        src/EventListener.cpp
        src/CustomEventListener.cpp
        src/Utils.cpp
        src/SoundEvent.cpp
        src/LoopManager.cpp
)

find_package(CommonLibSSE CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
add_commonlibsse_plugin(${PROJECT_NAME} SOURCES ${headers} ${sources})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_precompile_headers(${PROJECT_NAME} PRIVATE PCH.h)

target_link_libraries(${PROJECT_NAME} PRIVATE yaml-cpp)

set(DLL_FOLDER "${CMAKE_SOURCE_DIR}/SKSE/Plugins")

message(STATUS "SKSE plugin output folder: ${DLL_FOLDER}")

add_custom_command(TARGET "${PROJECT_NAME}" POST_BUILD
        COMMAND echo Copying Plugin-DLL...
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${DLL_FOLDER}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        COMMAND echo Copying Plugin-PDB...
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
        VERBATIM
)