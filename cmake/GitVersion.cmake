# Get date and time
string(TIMESTAMP BUILD_DATE "%Y%m%d")
string(TIMESTAMP BUILD_TIME "%H%M%S")

# Get short git commit hash
execute_process(
  COMMAND git rev-parse --short=7 HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Combine to make the build number
set(BUILD_NUMBER "${BUILD_DATE}.${BUILD_TIME}.${GIT_COMMIT_HASH}")

# Optional: write to a header file
#configure_file(
#  ${CMAKE_SOURCE_DIR}/build_info.h.in
#  ${CMAKE_BINARY_DIR}/generated/build_info.h
#  @ONLY
#)

# Include directory so you can use the header
include_directories(${CMAKE_BINARY_DIR}/generated)

# Print it to console
message(STATUS "Generated build number: ${BUILD_NUMBER}")
