cmake_minimum_required(VERSION 3.20.0)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
	ChecksumHash
	GIT_REPOSITORY
		git@bitbucket.org:dinamiet/checksumhash.git
	GIT_TAG
		development
	SOURCE_DIR
		${CMAKE_SOURCE_DIR}/lib/ChecksumHash
)

FetchContent_MakeAvailable(
	ChecksumHash
)
