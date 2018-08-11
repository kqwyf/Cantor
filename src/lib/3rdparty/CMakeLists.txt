add_definitions(-fPIC)

set(cppmarkdown_SRCS
    markdown.h
    markdown.cpp
    markdown-tokens.h
    markdown-tokens.cpp
)

add_library(cppmarkdown STATIC
	${cppmarkdown_SRCS}
)

target_link_libraries(cppmarkdown PUBLIC boost_regex)
