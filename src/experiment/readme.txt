

Scripts for executing searches and recording the results.

Most operations are performed by configuration.py:

	gencode: 						generates the C++ code for the searcher implementation.
	
	genprob <probsetname>: 			genereates the problem set specified.
	
	dump <probsetname> <outfile>:	generates the parameters of all searches that would be conducted, and dumps them to outfile.
	
	exec <probsetname> <outfile>:	performs all searches relevant to the problem set.
	


configuration.py defines classes:

	AlgorithmInfo( 	NiceName,
					C++ class name,
					header file for class,
					requires abstractable domain,
					is utility aware,
					optional runtime parameters to be passed in
				)
	
	
	DomainInfo(		NiceName,
					C++ class name,
					header file for class,
					provides abstraction,
					problem class for this domain
			)
	
	
	ProblemSetInfo(	problem class this set belongs to,
					file name,
					function that generates array of problems,
					args for function
				)


There are four parameters that specify each search conducted for dump/exec: (domain, algorithm, weights, problemNumber).




