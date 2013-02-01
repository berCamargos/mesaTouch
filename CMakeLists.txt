project( mesa )
find_package( OpenCV REQUIRED )
Find_Package ( SDL REQUIRED )
Find_Package ( SDL_image REQUIRED )
link_libraries (
   ${SDL_LIBRARY}
   ${SDLIMAGE_LIBRARY} # if using SDL_image, obviously
   SDLmain # Sadly not included in SDL_LIBRARY variable
)
add_executable( mesa mesa )
target_link_libraries( mesa ${OpenCV_LIBS} )
set(CMAKE_C_FLAGS_RELEASE "-O3  -lSDL -lSDL_image")
