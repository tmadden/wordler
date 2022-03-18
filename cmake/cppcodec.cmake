include(FetchContent)

message(STATUS "Fetching cppcodec")

FetchContent_Declare(cppcodec
  GIT_REPOSITORY
  https://github.com/tplgy/cppcodec
  GIT_TAG v0.2
  GIT_SHALLOW TRUE)

FetchContent_Populate(cppcodec)

include_directories(${cppcodec_SOURCE_DIR})
