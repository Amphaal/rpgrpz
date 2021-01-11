execute_process(
    COMMAND cat QtDeps_.txt 
    COMMAND xargs -L 1 cmake -E copy_if_different
)