# Downloads the benchmark corpus into DEST: cmake -DDEST=<dir> -P corpus.cmake
#
# The files live in a gist, pinned to one revision, so no HTML is kept in the repository.
# SOURCES.md in the gist lists where each file comes from.

set(GIST https://gist.githubusercontent.com/BayernMuller/3d691a4713c9082bf426f8fb3338c67c/raw)
set(REVISION 284d14127c8e6d6f04c4326f2ee660e7a3f48e74)

set(CORPUS
  wikipedia_web_browser.html 807c3fe0b3fbe8cd8ec0bc6a7d3fe397e3ccd2a53480f51f216cf446119cd5a9
  wikipedia_html.html 01dddc8810567a6fafcc9e56b1d05b5ae1d8987320e724d31b5cf85cdcc1c6da
  wikipedia_united_states.html b178c54d2a7133084d43aae76cecbd9d30762ec64253c77fba23b7ca226e8121
  generated_deep.html 2f9ab8b51c8d5d3b2605678c52add10fdfc64ac5b8f8c197251d6045e33d765b
)

while (CORPUS)
  list(POP_FRONT CORPUS name hash)
  set(path ${DEST}/${name})
  if (EXISTS ${path})
    file(SHA256 ${path} actual)
    if (actual STREQUAL hash)
      continue()
    endif()
  endif()
  message(STATUS "Downloading ${name}")
  file(DOWNLOAD ${GIST}/${REVISION}/${name} ${path} EXPECTED_HASH SHA256=${hash} STATUS status)
  list(GET status 0 code)
  if (NOT code EQUAL 0)
    file(REMOVE ${path})
    message(FATAL_ERROR "Could not download ${name}: ${status}")
  endif()
endwhile()
