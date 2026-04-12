include("/home/okumnas/Cedt/game_engine/projects/crossing_road/cmake/cmake/CPM_0.38.7.cmake")
CPMAddPackage("NAME;assimp;GITHUB_REPOSITORY;assimp/assimp;VERSION;5.4.3;OPTIONS;ASSIMP_BUILD_TESTS OFF;ASSIMP_BUILD_SAMPLES OFF;ASSIMP_BUILD_ASSIMP_TOOLS OFF;ASSIMP_INSTALL_PDB OFF;BUILD_SHARED_LIBS ON")
set(assimp_FOUND TRUE)