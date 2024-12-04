#include "boilerplate.h"
#include <math.h>
#include <random>
#include <iostream>
#include <ctime>
#include <algorithm>

// Global variables to be place in proper place later.
int targetVertices = -1;
int targetPentachora = -1;

double randd() {
	return ((double)rand() / (RAND_MAX + 1.0));
}

/* penalty for moving away from balance */
double beta(int n, int balance, double scaling) {
	double ans;
	(double)balance;
	(double)n;
		
	ans = exp(scaling*(balance-n));
	ans = ans/(1+ans);
	return ans;
}

/*
# pick move: pick move to eventually change number of vertices
#
# tri:		state triangulation
# xx:		threshold to try 3-3-move (in G(x,y) we have xx = 1 - 2(x+y))
# b:		threshold to do 2-4- over 2-0-move (edge or triangle)
# verts:	target number of vertices
*/
bool perform(regina::Triangulation<4>& tri, double xx, double b, int verts, int lowerBound, int upperBound) {
	int v = tri.countVertices();
	int e = tri.countEdges();
	int f = tri.countTriangles();
	int tet = tri.countTetrahedra();
	
	/*std::vector<int> lst_e(e);
	std::iota(std::begin(lst_e),std::end(lst_e),0);
	std::vector<int> lst_f(f);
	std::iota(std::begin(lst_f),std::end(lst_f),0);
	std::vector<int> lst_tet(tet);
	std::iota(std::begin(lst_tet),std::end(lst_tet),0);*/
	
	bool check;
	double x, y;
	
	if (v < verts) {
		while (v < verts) {
			tri.pachner(tri.pentachoron(0), false, true);
			return true;
		}
	} else if (v > verts) {
		// try edge collapse
		//std::random_shuffle(lst_e.begin(), lst_e.end());
		for (int i = 0; i < e; i++) {
			//if (tri.collapseEdge(tri.edge(lst_e[i]), true, true)) {
			if (tri.collapseEdge(tri.edge(i), true, true)) {
				return true;
			}
		}
	}
	// edge collapse failed
	x = randd();
	if (x > xx && tri.size() >= lowerBound) {
		// try 3-3 move
		//std::random_shuffle(lst_f.begin(), lst_f.end());
		for (int i = 0; i < f; i++) {
			//if (tri.pachner(tri.triangle(lst_f[i]), true, true)) {
			if (tri.pachner(tri.triangle(i), true, true)) {
				return true;
			}
		}
	}
	// 3-3-move not executed, check if 2-0-move should be performed
	y = randd();
	if (y > b && tri.size() >= lowerBound) {
		// case very likely if state is larger than target
		// try going down using 2-0 edge or triangle moves
		for (int i = 0; i < e; i++) {
			//if (tri.twoZeroMove(tri.edge(lst_e[i]), true, true)) {
			if (tri.twoZeroMove(tri.edge(i), true, true)) {
				return true;
			}
		}
		for (int i = 0; i < f; i++) {
			//if (tri.twoZeroMove(tri.triangle(lst_f[i]), true, true)) {
			if (tri.twoZeroMove(tri.triangle(i), true, true)) {
				return true;
			}
		}
	}
	// 2-0-move not executed, try 2-4-move instead
	//std::random_shuffle(lst_tet.begin(), lst_tet.end());
	if (tri.size() <= upperBound-2) {
	  for (int i = 0; i < tet; i++) {
		//if (tri.pachner(tri.tetrahedron(lst_tet[i]), true, true)) {
		if (tri.pachner(tri.tetrahedron(i), true, true)) {
			return true;
		}
	  }
	} else {
	  // no 2-4 move because we reached upper bound:
	  // try other move (3-3 or 2-0)
	  for (int i = 0; i < f; i++) {
		//if (tri.pachner(tri.triangle(lst_f[i]), true, true)) {
		if (tri.pachner(tri.triangle(i), true, true)) {
			return true;
		}
	  }
	  for (int i = 0; i < e; i++) {
			//if (tri.twoZeroMove(tri.edge(lst_e[i]), true, true)) {
			if (tri.twoZeroMove(tri.edge(i), true, true)) {
				return true;
			}
	  }
	  for (int i = 0; i < f; i++) {
			//if (tri.twoZeroMove(tri.triangle(lst_f[i]), true, true)) {
			if (tri.twoZeroMove(tri.triangle(i), true, true)) {
				return true;
			}
	  }	
	}
	
	// nothing worked
	for (int i = 0; i < tet; i++) {
		//if (tri.pachner(tri.tetrahedron(lst_tet[i]), true, true)) {
		if (tri.pachner(tri.tetrahedron(i), true, true)) {
			return true;
		}
	}	
	return false;
}




/*
# target vertex: connect triangulation to a verts-vertex triangulation
#
# tri:		state triangulation
# xx:		threshold to try 3-3-move (in G(x,y) we have xx = 1 - 2(x+y))
# balance:	preferred size of triangulation
# scaling:	parameter for severity of penalty to be off balance
*/
bool step(regina::Triangulation<4>& tri, double xx, int balance, double scaling, int lowerBound, int upperBound) {
	int vertices = tri.countVertices();
	int pentachora = tri.size();
	bool res = false;
	// steps
	int st = 0;
	// penalty
	double b;
		
	while (true) {
		st++;
		b = beta(pentachora,balance,scaling);
		res = perform(tri,xx,b,targetVertices,lowerBound,upperBound);
		if (not res) {
			std::cout <<  "found bad triangulations" << std::endl;
			return false;
		}
		//std::cout << tri.isoSig() << "\t" << tri.countVertices() << std::endl;
		vertices = tri.countVertices();
		pentachora = tri.size();
		if (lowerBound < targetPentachora) {
			if (vertices == targetVertices && pentachora==targetPentachora) {
				//std::cout <<  st << std::endl;
				return true; }
		} else {
			if (vertices == targetVertices && pentachora==lowerBound) {
				//std::cout <<  st << std::endl;
				return true; }
		}
		if (st%5000000 == 0) {
			std::cout <<  st << "\t v: " << vertices << "\t p: " << pentachora << std::endl;
		}
	}
	return false;			
}

void usage(const char* progName, const std::string& error = std::string()) {
    if (!error.empty()) {
        std::cerr << error << "\n\n";
    }
    std::cerr << "Usage:" << std::endl;
    std::cerr << "      " << progName << " { census file } [ -v=targetVertices ] [ -p=targetPentachora ] \n";
    exit(1);
}

bool argCharComp(char arr[], char c) {
    return arr[0] == '-' && arr[1] == c;
}

int main(int argc, char* argv[]) {
    std::string rawCensusFile;
    if (argc < 2) {
        usage(argv[0], "Error: No census file provided.");
    }
    if (2 < argc) {
        for (int i=2; i<argc; ++i) {
            if (argCharComp(argv[i],'v')) {
                targetVertices = std::stoi(argv[i]+=2);
            }
            else if (argCharComp(argv[i],'p')) {
                targetPentachora = std::stoi(argv[i]+=2);
            }
            else {
                usage(argv[0],std::string("Invalid Option: ")+argv[i]);
            }
        }
    }
//    else {
        rawCensusFile = argv[1];
//    }
    const char* censusFile = rawCensusFile.c_str();
    
// use different random seed for every run
srand((unsigned)time(0));

// parameters

// probability of choosing 3-3 over up down
double xx = 0.1;
// preferred size
int balance = 2 * targetPentachora - 3;
// aggressiveness
double scaling = 1.0;


// steps
int steps = 100;

// result of merging
bool res;
// number of components in union find data structure
int numComponents;
// progress counters
int ctr = 0;
int cctr = 0;

// for final approach
int lowerBound = 10;
regina::Triangulation<4> copy;

// for timing
int tm = time(NULL);


// representative triangulation of current connected component
regina::Triangulation<4> rep;

// output from search through Pachner graph
regina::Triangulation<4> curTriangulation, seed;


// Load census into triangulation set data union find data structure
std::cout << "Load census ";
TriangulationSet census(censusFile);
std::cout << " ...done: " << census.countComponents() << " triangulations loaded." << std::endl;

    /*
     If no targetVertices or targetPentachora were given at runtime,
     look at the first triangulation in the file and set:
            - targetVertices = \chi(FIRST_TRIANGULATION)
            - targetPentachora = FIRST_TRIANGULATION.size()
     */

    regina::Triangulation<4> firstTri = census.components().rep();
    if (targetVertices == -1) {
        // todo: take max
        targetVertices = std::max(1,(int)firstTri.eulerCharTri());
    }
    if (targetPentachora == -1) {
        targetPentachora = firstTri.size();
    }
    
    std::cerr << "Target vertices: " << targetVertices << ", Target pentachora: " << targetPentachora << std::endl;

/* first iteration through data structure:

If triangulation has two vertices: connect it to some other 2-vertex, 
6-pentachoron triangulation and merge components.

If triangulation as more than two vertices: connect it to
some 2-vertex 6-pentachoron triangulation.

Important: first argument of merge becomes component representative
and must be the 2-vertex triangulation

*/
for (Component curcomp = census.components(); curcomp; ++curcomp) {
	curTriangulation = curcomp.rep();
	//std::cout << curTriangulation.countVertices() << " vertices" << std::endl;	
	step(curTriangulation,xx,balance,scaling,0,9999);
	//std::cout << curTriangulation.isoSig() << "\t" << rep.isoSig() << std::endl;
	res = census.merge(curcomp,curTriangulation,false);
	//if (res) std::cout << "MERGED" << std::endl;
	ctr++;
	std::cout << ctr << " triangulations processed in step one. " << census.countComponents() << " components, " << std::endl;
	//std::cout << time(NULL) - tm << " seconds elapsed. " << std::endl;
	
}

std::cout << "STEP ONE:" << time(NULL) - tm << " seconds." << std::endl;

tm = time(NULL);

/* second step keeps on running through connected components until 
number of components is less than 10 */

std::cout << "DONE WITH FIRST PART." << std::endl;

ctr = 0;
// get number of components
numComponents = census.countComponents();
while (numComponents > 10) {
	ctr++;
	// get first component
	for (Component curcomp = census.components(); curcomp; ++curcomp) {
		curTriangulation = curcomp.rep();
		res = false;
		for (int st = 0; st <= steps; st++) {
			step(curTriangulation,xx,balance,scaling,0,9999);
			res = census.merge(curcomp,curTriangulation,false);
			if (res) {
				numComponents = census.countComponents();
				if (numComponents > 20) {
					numComponents = census.countComponents();
					std::cout << "number of connected components " << numComponents << std::endl;
					break;
				} else {
					std::cout << "number of connected components " << numComponents << std::endl;
					for (Component ccomp = census.components(); ccomp; ++ccomp) {
						rep = ccomp.rep();
						std::cout << rep.isoSig() << "\t has size \t" << ccomp.size() << std::endl;
					}
					break;
				}
			} 
		}
	}
	numComponents = census.countComponents();
	std::cout << "run " << ctr << " complete: " << numComponents << " connected components" << std::endl;
}

std::cout << "STEP ONE:" << time(NULL) - tm << " additional seconds." << std::endl;
tm = time(NULL);

std::cout << "DONE WITH SECOND PART." << std::endl;


balance = targetPentachora * 2;
scaling = 0.9;
std::cout << "Changing balance and scaling to " << balance << " and " << scaling << "." << std::endl;
numComponents = census.countComponents();
while (numComponents > 1) {
	// get first component
	for (Component curcomp = census.components(); curcomp; ++curcomp) {
		curTriangulation = curcomp.rep();
		res = false;
		for (int st = 0; st <= steps; st++) {
			step(curTriangulation,xx,balance,scaling,0,9999);
			res = census.merge(curcomp,curTriangulation,false);
			if (res) {
				numComponents = census.countComponents();
				std::cout << "number of connected components " << numComponents << std::endl;
				for (Component ccomp = census.components(); ccomp; ++ccomp) {
					rep = ccomp.rep();
					std::cout << rep.isoSig() << "\t has size \t" << ccomp.size() << std::endl;
					std::cout << time(NULL)-tm << "\t seconds for last step \t" << ccomp.size() << std::endl;
					tm = time(NULL);
				}
				break;
			} 
		}
	}
	numComponents = census.countComponents();
}

std::cout << "THEOREM PROVED, HAVE A NICE DAY." << std::endl;		
		


}
