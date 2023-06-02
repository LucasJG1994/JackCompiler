cmake_minimum_required(VERSION 2.8.11)
project(jack)

include_directories(
	src
	src/parser
)

add_executable(jack
	src/main.c
	
	src/parser/scanner.cpp
	
	src/parser/scanner.h
	
	src/parser/jack.y
)

