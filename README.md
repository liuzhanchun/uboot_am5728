# uboot_am5728
push 之前先执行 clean.sh

简述启动流程及代码分析（详细参考https://blog.csdn.net/liuzhanchun/article/details/102899673）

第一阶段
主要执行流程：reset -> cpu_init_crit -> lowlevel_init -> _main

第二阶段
主要执行流程：board_init_f -> relocate_code -> board_init_r
board_init_r函数，进而调用initcall_run_list(init_sequence_r)函数执行一系列初始化函数以实现后半部分板级初始化，并在initcall_run_list函数里进入run_main_loop不再返回。
