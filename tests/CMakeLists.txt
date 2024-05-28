cmake_minimum_required(VERSION 3.12)
project(test)

file(GLOB TEST_ITEMS RELATIVE ${PROJECT_SOURCE_DIR} *)

foreach(ITEM ${TEST_ITEMS})
    if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${ITEM})
        aux_source_directory(${PROJECT_SOURCE_DIR}/${ITEM} ITEM_SRC)
        add_executable(${ITEM} ${ITEM_SRC})
        target_link_libraries(${ITEM} PRIVATE ${NEO_PROJECT_NAME}-engine ${NEO_PROJECT_NAME}-vm ${NEO_PROJECT_NAME}-common)
        set(ITEM_SRC)
    endif(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${ITEM})
endforeach(ITEM ${TEST_ITEMS})

foreach(ITEM ${TEST_ITEMS})
    if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${ITEM})
        add_test(NAME ${ITEM} COMMAND ${ITEM})
    endif(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${ITEM})
endforeach(ITEM ${TEST_ITEMS})
