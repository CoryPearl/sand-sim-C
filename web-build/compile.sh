emcc -o index.html web-main.c \
    raylib/src/libraylib.web.a \
    -Os -Wall \
    -I/opt/homebrew/include \
    -s USE_GLFW=3 \
    -s ASYNCIFY \
    -s SINGLE_FILE=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -DPLATFORM_WEB \
    --shell-file shell.html