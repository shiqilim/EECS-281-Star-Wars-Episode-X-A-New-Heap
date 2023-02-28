/// PROJECT IDENTIFIER: AD48FB4835AF347EB0CA8009E24C3B13F8519882

#include <getopt.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <deque>
#include <stack>
#include <queue>
#include <utility>
#include <vector>
#include "battalion.h"
#include "P2random.h"
using namespace std;

int main(int argc, char* argv[]) {
	ios_base::sync_with_stdio(false);

	Battalion game;
	game.getMode(argc, argv);
	game.start();

}