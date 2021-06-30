cmake_minimum_required(VERSION 3.19)

set(CMAKE_TOOLCHAIN_FILE "$ENV{SCE_PSP2_SDK_DIR}/host_tools/build/cmake/psp2-snc-toolchain.cmake")

project(repatch_ex)

include(VitaDevelopmentSuite)
enable_language(EMD)
set(CMAKE_C_STANDARD 99)

add_compile_options(
  -Xdiag=0 -Xquit=2
)

add_link_options(
  --prx-no-runtime-support
  --strip-unused-data
  --strip-duplicates
  --strip-report=strip-report.txt
)

include_directories(
  ${VDSUITE_KERNEL_INCLUDE_DIRECTORIES}
  ${VDSUITE_STANDARD_INCLUDE_DIRECTORIES}
)

link_directories(
  	${VDSUITE_LIBRARY_DIRECTORIES}
)

add_library(${PROJECT_NAME} MODULE
	repatch.c
	repatch_ex.emd
)

target_link_libraries(${PROJECT_NAME}
	taihenForKernel_stub
	taihenModuleUtils_stub
	SceSysmemForDriver_stub
	SceSysclibForDriver_stub
	SceThreadmgrForDriver_stub
	SceIofilemgrForDriver_stub
	SceDebugForDriver_stub
	SceSysrootForKernel_stub
	SceSblACMgrForDriver_stub
)

VDSuiteSignElf(${PROJECT_NAME}.skprx ${PROJECT_NAME} COMPRESS ON STRIP ON)
