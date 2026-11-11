cmake_minimum_required(VERSION 3.10.0)
project(ish_dungeon VERSION 0.1.0 LANGUAGES C CXX)

# указываем стандарт C++
set(CMAKE_CXX_STANDARD 20)
# генерация compile_commands.json для clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

include(FetchContent)


# библиотека httplib для работы с сетью
FetchContent_Declare(httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.64.0
)
FetchContent_MakeAvailable(httplib)

# библиотека для работы с json
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(nlohmann_json)


# список файлов с исходным кодом, который необходимо скомпилировать
add_library(
    ish_dungeon
    src/ish_dungeon.cpp
    src/article_repository.cpp
    src/utils.cpp
)

target_link_libraries(ish_dungeon PUBLIC httplib::httplib)
target_link_libraries(ish_dungeon PUBLIC nlohmann_json::nlohmann_json)

# директория с заголовочными файлами
target_include_directories(ish_dungeon PUBLIC include/)


# сама программа
add_executable(ish_dungeon_exec src/main.cpp)
target_link_libraries(ish_dungeon_exec PRIVATE ish_dungeon)
target_include_directories(ish_dungeon_exec PUBLIC include/)
target_compile_definitions(ish_dungeon_exec PRIVATE
    PROJECT_WWW_DIR="${CMAKE_CURRENT_SOURCE_DIR}/www"
)


# тестирование
include(CTest)
enable_testing()

# библиотека для тестирования catch2 скачивается автоматически без необходимости ручной установки
FetchContent_Declare(catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.15.2
)
FetchContent_MakeAvailable(catch2)

# список тестов
add_executable(tests tests/test1.cpp)

# зависимости тестов: ваша библиотека и catch2
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
target_link_libraries(tests PRIVATE ish_dungeon)

target_include_directories(tests PRIVATE include/)
