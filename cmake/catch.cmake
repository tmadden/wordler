include(FetchContent)

message(STATUS "Fetching Catch2")

FetchContent_Declare(Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.0.0-preview3
  GIT_SHALLOW TRUE)

FetchContent_MakeAvailable(Catch2)
