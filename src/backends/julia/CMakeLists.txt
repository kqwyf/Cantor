add_subdirectory(juliaserver)

set(JuliaBackend_SRCS
    juliabackend.cpp
    juliasession.cpp
    juliaexpression.cpp
)

kconfig_add_kcfg_files(JuliaBackend_SRCS settings.kcfgc)
ki18n_wrap_ui(JuliaBackend_SRCS settings.ui)

add_backend(juliabackend ${JuliaBackend_SRCS})

target_link_libraries(cantor_juliabackend Qt5::DBus)

install(FILES juliabackend.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})