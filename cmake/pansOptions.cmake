add_library(pans_options INTERFACE) # 配置接口包

target_compile_options(pans_options INTERFACE
    $<$<OR:$<CXX_COMPILER_ID:GNU>, $<CXX_COMPILER_ID:Clang>>:
        # GCC / Clang 通用选项
        -Wall
        -Wextra                 # 额外警告（比-Wall更严格）
        -Wpedantic              # 严格遵循标准
        -fno-strict-aliasing    # 关闭严格别名规则（不同类型的指针可以相互赋值）
    >
)

target_compile_options(pans_options INTERFACE
    $<$<AND:$<PLATFORM_ID:Linux>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:
        -fPIC                   # 编译位置无关代码
    >
)

target_link_options(pans_options INTERFACE 
    $<$<AND:$<PLATFORM_ID:Linux>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:
        -rdynamic               # 栈回溯需要
    >
)

target_compile_definitions(pans_options INTERFACE 
    $<$<CONFIG:Debug>:PANS_DEBUG>
)

target_compile_options(pans_options INTERFACE
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Debug>:-ggdb>
)

# ===== Release/RelWithDebInfo 共用选项 =======
target_compile_options(pans_options INTERFACE 
    $<$<CONFIG:Release>:-DNDEBUG>
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Release>:-fno-omit-frame-pointer>

    $<$<CONFIG:RelWithDebInfo>:-DNDEBUG>
    $<$<CONFIG:RelWithDebInfo>:-O2>
    $<$<CONFIG:RelWithDebInfo>:-g>
    $<$<CONFIG:RelWithDebInfo>:-fno-omit-frame-pointer>
)

option(ENABLE_COVERAGE "Enable code coverrage instrumentation" OFF)
if(ENABLE_COVERAGE)
    target_compile_options(pans_options INTERFACE
        $<$<CONFIG:Debug>:--coverage>
    )
    target_link_options(pans_options INTERFACE
        $<$<CONFIG:Debug>:--coverage>
    )
endif()