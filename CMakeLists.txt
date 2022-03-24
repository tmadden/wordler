cmake_minimum_required (VERSION 3.14)
project(wordler)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-DALIA_HTML_LOGGING)

# Add dependencies.
include(cmake/alia-html.cmake)
include(cmake/cppcodec.cmake)
include(cmake/cereal.cmake)

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
file(GLOB_RECURSE source_files "src/*.cpp")
add_executable(app ${source_files})
target_link_libraries(app alia_html)
target_include_directories(app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${cereal_SOURCE_DIR}/include)
set_target_properties(app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${deploy_dir})

# Also deploy asm-dom.js.
alia_post_build_deploy(app
    ${CMAKE_CURRENT_BINARY_DIR}/asm-dom.js
    ${deploy_dir})
