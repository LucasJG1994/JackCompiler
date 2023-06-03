cmake_minimum_required(VERSION 2.8.11)
project(jack)

include_directories(
	src
	src/parser
)

add_executable(jack
	src/main.c
	src/dirWin.c
	src/c2t.cpp

	src/dir.h
	src/c2t.h
	
	src/parser/scanner.cpp
	
	src/parser/scanner.h
	
	src/parser/jack.tab.c
	src/parser/jack.tab.h

	src/parser/jack.y
)

