find_package(Qt6 REQUIRED COMPONENTS Test Sql)

function(add_kanoop_database_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE
        Qt6::Test
        Qt6::Sql
        KanoopDatabaseQt
        KanoopCommonQt
    )
    target_include_directories(${name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
    )
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_kanoop_database_test(tst_databasecredentials)
add_kanoop_database_test(tst_sqlparser)
add_kanoop_database_test(tst_datasource)
