cmake_minimum_required (VERSION 3.14)
project(wordler)

set(CMAKE_CXX_STANDARD 20)

# Add dependencies.
include(cmake/alia-html.cmake)
include(cmake/cppcodec.cmake)
include(cmake/cereal.cmake)
include(cmake/catch.cmake)

# Create a subdirectory in the build dir that we'll use to accumulate all the
# files that need to be deployed to the web.
set(deploy_dir ${CMAKE_CURRENT_BINARY_DIR}/deploy)
file(MAKE_DIRECTORY ${deploy_dir})

# Deploy the index.html and style.css files.
alia_deploy(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/index.html"
    "${deploy_dir}")
alia_deploy(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/style.css"
    "${deploy_dir}")

# Deploy all the word files.
file(GLOB word_files "words/*")
file(MAKE_DIRECTORY "${deploy_dir}/words")
foreach(word_file ${word_files})
    alia_deploy(
        "${word_file}"
        "${deploy_dir}/words")
endforeach()

# Add the main app.
file(GLOB_RECURSE source_files "src/wordler/*.cpp")
add_library(wordler ${source_files})
target_link_libraries(wordler alia_html)
target_include_directories(wordler PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${cereal_SOURCE_DIR}/include)
add_executable(app "src/app.cpp")
target_link_libraries(app wordler)
set_target_properties(app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${deploy_dir})

# Also deploy asm-dom.js.
alia_post_build_deploy(app
    ${CMAKE_CURRENT_BINARY_DIR}/asm-dom.js
    ${deploy_dir})

# Add tests.
file(GLOB_RECURSE test_files CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)
add_executable(tests ${test_files})
target_include_directories(tests PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Catch2_SOURCE_DIR}/include)
target_link_libraries(tests PUBLIC wordler Catch2::Catch2WithMain)
enable_testing()
add_test(NAME unit_tests COMMAND tests)
