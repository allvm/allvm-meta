remove_definitions(-fprofile-arcs -ftest-coverage)

set(CMAKE_ASM_COMPILER nasm CACHE STRING Assembler)
set(CMAKE_ASM_FLAGS -felf64 CACHE STRING AssemblerFlags)
enable_language(ASM)

add_custom_command(OUTPUT SyscallList.h COMMAND perl "${CMAKE_SOURCE_DIR}/horizon/Baremetal/gen-syscall-list.pl" SyscallList.h DEPENDS "${CMAKE_SOURCE_DIR}/horizon/Baremetal/gen-syscall-list.pl")
add_custom_command(OUTPUT RuntimeAddressList.h COMMAND perl "${CMAKE_SOURCE_DIR}/horizon/Baremetal/gen-runtime-address-list.pl" "${CMAKE_BINARY_DIR}/horizon/Baremetal/RuntimeAddressList.h" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" DEPENDS "${CMAKE_SOURCE_DIR}/horizon/Baremetal/gen-runtime-address-list.pl")

add_definitions(-DBAREMETAL)
include_directories(${CMAKE_BINARY_DIR}/horizon/Baremetal)
add_library(Baremetal STATIC
                      SyscallList.h
                      RuntimeAddressList.h
                      assert.cc
                      cpu.cc
                      InterruptManager.cc
                      SpecialFile.cc
                      DebugStream.cc
                      syscallHandlers.cc
                      dynamicLookup.cc
                      PhysicalMemoryManager.cc
                      VirtualMemoryManager.cc
                      InterruptAsm.s
                      pthreadStubs.cc
                      callStart.s)

target_link_libraries(Baremetal Codegen Core Bytecode Builtins-Baremetal Runtime-Static)

add_executable(embedded init.cc)
set_target_properties(embedded PROPERTIES LINK_FLAGS -static)
target_link_libraries(embedded Baremetal)
if(COVERAGE)
  target_link_libraries(embedded -lgcov)
endif(COVERAGE)
add_custom_command(TARGET embedded
                   POST_BUILD
                   COMMAND insn-swap -s 0f05 -r cdff embedded
                   COMMENT "Swapping syscall instructions for int 0xff")
add_custom_command(TARGET embedded
  POST_BUILD
  COMMAND nm embedded | awk '/T kmain/ {print $$1}' | xargs -IREPL objcopy --set-start=0xREPL embedded
  COMMENT "Manually adjusting embedded executable entry point")


set(KERNEL_HBC CACHE PATH "Kernel bytecode file")
if(KERNEL_HBC)
  add_custom_command(TARGET embedded
    POST_BUILD
    COMMAND mkdir -p /tmp/hzn/boot
    COMMAND mkdir -p /tmp/hzn/kiwi/modules
    COMMAND cp ${CMAKE_SOURCE_DIR}/horizon/Baremetal/boot/cdboot.img /tmp/hzn/boot
    COMMAND cp embedded /tmp/hzn/kiwi/kernel
    COMMAND cp ${KERNEL_HBC} /tmp/hzn/kiwi/modules/hbc
    COMMAND mkisofs -quiet -J -R -l -b boot/cdboot.img -V "Horizon CD-ROM" -boot-load-size 4 -boot-info-table -no-emul-boot -o cd.img /tmp/hzn
    COMMAND rm -r /tmp/hzn
    COMMENT "Creating ISO9660 CD-ROM image")
  add_dependencies(embedded ${KERNEL_HBC})
else(KERNEL_HBC)
  message("Note: ISO9660 image not being created because CMake variable KERNEL_HBC (defining the bytecode file to use as the kernel) is not set.")
endif(KERNEL_HBC)

include_directories(.)
add_executable(insn-swap insn-swap.cc libudis86/decode.c libudis86/input.c libudis86/mnemonics.c libudis86/opcmap.c libudis86/syn-att.c libudis86/syn.c libudis86/syn-intel.c libudis86/udis86.c)
target_link_libraries(insn-swap ${LLVM_LIBS} -ldl -pthread)
if(COVERAGE)
  target_link_libraries(insn-swap -lgcov)
endif(COVERAGE)
