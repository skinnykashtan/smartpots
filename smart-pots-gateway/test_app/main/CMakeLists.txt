idf_component_register(
        SRCS "test_main.cpp" "test_queue.cpp"
        INCLUDE_DIRS "."
        REQUIRES unity test
        WHOLE_ARCHIVE
)