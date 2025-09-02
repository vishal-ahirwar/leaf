#pragma once
#include<vector>
#include<string>
int install();

int create();

int publish();

int upload();

int runTests();

int format();

int clean();

int debug();

int addPackage();

int removePackage();

int addApp();

int addLib();

int doctor();

void betterArgs(std::vector<std::string> &args, size_t argc, char **argv);

int help();

int build();

int compile();

int run();

int init();