Author: Matthew Jones.
mjon661@aucklanduni.ac.nz

A collection of search algorithms, search domains, and related utilities.

Overview:
	Written for C++11.
	Everything defined in root namespace mjon661.
	util/debug.hpp and source in app/ uses some linux/gcc specific functionality.
	Otherwise its vanilla C++11 with some static Boost libraries.

Dependencies/3rd party:
	Boost
	JSON for Modern C++. https://github.com/nlohmann/json. Header json.hpp placed in util/impl.

Directories:
	build/
		Build targets, .o and .d files. Git shouldn't be tracking anything in here.
		libs/
	src/
		app/
			Concrete code for build targets; anything that instantiates domains and algorithms.
		domain/
			Search domains and their supporting classes (e.g. problem generators).
		search/
			Search algorithms and classes specifically for their use (i.e. open/closed list implementations).
		structs/
			Generic containers, etc.
		util/
			Utilities used throughout.

Notes:
	Heavy use of templates.
	JSON used holding runtime configuration, results.
	mjon661::Json (in util/json.hpp) is an alias of nlohmann::json (util/impl/json.hpp).
	
	util/debug.hpp defines custom assert macros and a debug level define. DEBUGLEVEL can be 0-3 (see debug.hpp).
	Assert macros take a boolean condition, and optionally a printf format string and a variable number of 
	arguments that are used with vprintf. Assert macros throw an AssertException (debug.hpp) on failure, which 
	carry vprintf output and other useful information in their what().
	
	fast_assert() for assertion checks with sub linear cost (wrt to node expansions). Enabled by DEBUGLEVEL>=1.
	slow_assert(), linear cost. DEBUGLEVEL>=2.
	glacial_assert(), superlinear cost, or at least very expensive. DEBUGLEVEL==3.

	ConfigException (util/exception.hpp) should be thrown by alg/domain ctors if they fail to initialise
	with the given arguments.
	

Models:
	See doc/models.txt. A search algorithm requires a model - a set of particular functionality - to be implemented by domain classes.
	

Solution:
	The Solution template (search/solution.hpp) holds sequences of states and operators representing a path/solution. An
	empty instance is provided to an algorithm, which (upon successfully finding a solution) will populate it.
	
	
Domain:	
	Concrete classes.
	Initial state is (currently) created by calling the parameterless Methods::createState.
	Domains should be stateless from the perspective of algorithms. Must implemented ctor accepting a Json argument.


Algorithms:
	Single template parameter - a domain.
	Stateful, always existing in one of 'ready to perform search', 'search done - can retrieve information'.
	Must implemented ctor accepting a Json argument.
	See doc/algorithms.txt.
