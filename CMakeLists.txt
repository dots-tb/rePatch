cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(repatch)
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -std=gnu99")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")

link_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(${PROJECT_NAME}
	repatch.c
)

target_link_libraries(${PROJECT_NAME}
	taihenForKernel_stub
	SceSysmemForDriver_stub
	SceSysclibForDriver_stub
	SceThreadmgrForDriver_stub
	SceIofilemgrForDriver_stub
	SceDebugForDriver_stub
	SceSysrootForKernel_stub
	SceSblACMgrForDriver_stub
)
vita_create_self(${PROJECT_NAME}.skprx ${PROJECT_NAME}
  UNSAFE
  CONFIG ${CMAKE_SOURCE_DIR}/exports.yml
)